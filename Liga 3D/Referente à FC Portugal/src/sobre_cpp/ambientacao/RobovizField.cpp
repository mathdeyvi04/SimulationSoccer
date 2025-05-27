#include "RobovizField.h"
#include "RobovizLogger.h"
#include "World.h"
#include <cstdio>

/*////////////////////////////////////////////////////////////////////////

Único motivo de precisarmos deste .cpp são as seguintes definições

O static garante que essa variável fique viva sempre, até a completa
finalização do programa.

Precisamos usar essas linhas a seguir para poder foçar a alocação, pois
se fosse apenas no header, não haveria alocação.

*/////////////////////////////////////////////////////////////////////////

static World& mundo_existente = Singular<World>::obter_instancia();

decltype(RobovizField::cRingLineLength  ) constexpr RobovizField::cRingLineLength;
decltype(RobovizField::cPenaltyBoxDistX ) constexpr RobovizField::cPenaltyBoxDistX;
decltype(RobovizField::cHalfPenaltyWidth) constexpr RobovizField::cHalfPenaltyWidth;
decltype(RobovizField::cHalfGoalWidth   ) constexpr RobovizField::cHalfGoalWidth;
decltype(RobovizField::cHalfFieldLength ) constexpr RobovizField::cHalfFieldLength;
decltype(RobovizField::cGoalWidth       ) constexpr RobovizField::cGoalWidth;
decltype(RobovizField::cGoalDepth       ) constexpr RobovizField::cGoalDepth;
decltype(RobovizField::cGoalHeight      ) constexpr RobovizField::cGoalHeight;
decltype(RobovizField::cFieldLength     ) constexpr RobovizField::cFieldLength;
decltype(RobovizField::cFieldWidth      ) constexpr RobovizField::cFieldWidth;
decltype(RobovizField::cPenaltyLength   ) constexpr RobovizField::cPenaltyLength;
decltype(RobovizField::cPenaltyWidth    ) constexpr RobovizField::cPenaltyWidth;
decltype(RobovizField::cSegmentos::list ) constexpr RobovizField::cSegmentos::list;
decltype(RobovizField::cPontos::list    ) constexpr RobovizField::cPontos::list;

decltype(RobovizField::gMkrs::list      )           RobovizField::gMkrs::list;

/////////////////////////////////////////////////////////////////////////////////////

// Há limitações aqui.
void RobovizField::obter_marcadores_de_campo(){
	/*
	Descrição:
		Coletará todos os pontos de contato no chão que o robô
		consegue perceber e visualizar.

		Depois disso, ajusta o peso de cada ponto dependendo da
		incerteza (erro) na medição da distância, duplicando mais
		vezes os pontos mais confiáveis.
	*/
	
	// Adicionar pontos de contato do pé
	{
		/*
		Descrição:
			Verifica contato do pé e, caso esteja no chão, 
			adiciona nas respectivas listas necessárias, vulgo:

			- lista_de_pontos_de_contato_do_pe
			- lista_de_marcadores_de_chao

		Limitações:
			- Caso o pé entre em contato com outros jogadores ou com a bola, este contato
			será contabilizado, apesar de ser desnecessário.

			- Robô NAO do tipo 4 pode tocar o chão ou outras coisas com seus dedos do pé.
		*/
		for(
			int index_do_pe = 0;
			index_do_pe < 2;
			index_do_pe++
		){

			if(
				mundo_existente.se_pe_esta_tocando[ index_do_pe ]
			){
				
				lista_de_pontos_de_contato_do_pe.emplace_back(
															   sVetor3D({0, 0, 0}), 
															   mundo_existente.pos_rel_do_ponto_de_contato[index_do_pe].para_esferica(),
															   mundo_existente.pos_rel_do_ponto_de_contato[index_do_pe]
															 );

					 lista_de_marcadores_de_chao.emplace_back(
					 										   sVetor3D({0, 0, 0}), 
					 									       mundo_existente.pos_rel_do_ponto_de_contato[index_do_pe].para_esferica(),
					 									       mundo_existente.pos_rel_do_ponto_de_contato[index_do_pe]
					 									     );
			}
		}
	}

	// Adicionar linhas e corners
	{
		/*
		Descrição:
			Adicionaremos as respectivas entidades à lista de
			marcadores de chão.
		*/

		for(
			const Linha& linha_qualquer : lista_de_todos_os_segmentos
		){

			lista_de_marcadores_de_chao.emplace_back(
													 sVetor3D({0, 0, 0}),
													 linha_qualquer.inicio_e, 
													 linha_qualquer.inicio_c
													);

			lista_de_marcadores_de_chao.emplace_back(
													 sVetor3D({0, 0, 0}),
													 linha_qualquer.final_e, 
													 linha_qualquer.final_c
													);
		}

		quantidade_de_marcadores_de_chao_nao_colineares = lista_de_marcadores_de_chao.size();

		for(
			const sMarcador& canto_qualquer : lista_de_corners
		){

			lista_de_marcadores_de_chao.emplace_back(
													 sVetor3D({0, 0, 0}),
													 canto_qualquer.pos_rel_esf,
													 canto_qualquer.pos_rel_cart
													);
		}
	}

	// Considerar confiabilidade de Medidas
	{
		/*
		Descrição:
			Todas as os erros das coordenadas polares são
			dependentes da distância.
			Em especial, quanto maior a distância, maior
			a confiabilidade do sensor e do dado.

			A seguir, a forma como calcularemos o erro:	

			- ed -> erro na distância
			- er -> erro no arredondamento

		    - Var[distance error] = (d/100)^2 * Var[ed] + Var[er] 
		    
		    Aplicaremos da seguinte forma:
			
		    repetition = max(int(k*(1/var)),1), where k=1/1500
		    repetitions for    1 meter:  71
		    repetitions for    2 meters:
		    repetitions for >=19 meters:  1
			
			Onde K é apenas uma constante númerica.
		*/

		float K_W = 1500.f;
		for(
			const sMarcador& smkr : lista_de_marcadores_de_chao
		){
				
			// Observe que a componten x do vetor esférico, é o módulo da distância.
			float var  = ((smkr.pos_rel_esf.x * smkr.pos_rel_esf.x) / 10000.f) * var_distance + var_round_hundredth;
			float peso = 1.0f / (K_W * var);

			lista_de_marcadores_de_chao_pesados.insert(
				// No final da lista
				lista_de_marcadores_de_chao_pesados.end(),
				// Quantidade de interações dependendo da distância
				max( int(peso), 1),
				// o marcador
				smkr
			);
		}
	}
}

/*//////////////////////////////////////////////////////////////////////////////

Métodos de Apresentação

*//////////////////////////////////////////////////////////////////////////////


void RobovizField::atualizar_marcadores(){

	// Esvaziando listas
	{
		lista_de_todos_os_segmentos.clear();
	   lista_de_corners_e_goalposts.clear();
				   lista_de_corners.clear();
				 lista_de_goalposts.clear();
   lista_de_pontos_de_contato_do_pe.clear();
  lista_de_marcadores_identificados.clear();
  lista_de_marcadores_desconhecidos.clear();
   lista_de_segmentos_identificados.clear();
		lista_de_marcadores_de_chao.clear();
lista_de_marcadores_de_chao_pesados.clear();
	}


	// Preparando lista de marcadores de chão
	{
		for(
			int i = 0;
			i < 8;
			i++
		){

			// Pegamos um marcador de chão do mundo
			World::gMkr *landmkr = &(mundo_existente.marcadores_de_chao[i]);

			// Vamos criar formas diferentes de representação
			sFixedMkr *elemento_em_fixed_mkr;
			const sPonto *elemento_em_sponto;

			/*
			Verificações para obtermos conhecimento de qual canto é.
			*/
			if       ( (*landmkr).pos_abs.x == -15 && (*landmkr).pos_abs.y == -10 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_corner_mm);
				elemento_em_sponto    = &(cPontos:: corner_mm);
			} else if( (*landmkr).pos_abs.x == -15 && (*landmkr).pos_abs.y == +10 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_corner_mp);
				elemento_em_sponto    = &(cPontos:: corner_mp);
			} else if( (*landmkr).pos_abs.x == +15 && (*landmkr).pos_abs.y == -10 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_corner_pm);
				elemento_em_sponto    = &(cPontos:: corner_pm);
			} else if( (*landmkr).pos_abs.x == +15 && (*landmkr).pos_abs.y == +10 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_corner_pp);
				elemento_em_sponto    = &(cPontos:: corner_pp);
			} else if( (*landmkr).pos_abs.x == -15 && (*landmkr).pos_abs.y  <   0 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_goal_mm);
				elemento_em_sponto    = &(cPontos:: goal_mm);
			} else if( (*landmkr).pos_abs.x == -15 && (*landmkr).pos_abs.y  >   0 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_goal_mp);
				elemento_em_sponto    = &(cPontos:: goal_mp);
			} else if( (*landmkr).pos_abs.x == +15 && (*landmkr).pos_abs.y  <   0 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_goal_pm);
				elemento_em_sponto    = &(cPontos:: goal_pp);
			} else if( (*landmkr).pos_abs.x == +15 && (*landmkr).pos_abs.y  >   0 ) {

				elemento_em_fixed_mkr = &(gMkrs  ::_goal_pp);
				elemento_em_sponto    = &(cPontos:: goal_pp);
			} else{
				// Todos já foram varridos
				return;
			}

			// Se está vísivel, devemos configurar determinadas coisas
			if(
				(*elemento_em_fixed_mkr).se_esta_visivel == (*landmkr).se_esta_visivel
			){

				(*elemento_em_fixed_mkr).setar_pos_rel_a_partir_de_esf( (*landmkr).pos_rel_esf );

				sMarcador marcador_visto(
					elemento_em_sponto,
					(*landmkr).pos_rel_esf
				);

				lista_de_corners_e_goalposts     .push_back( marcador_visto );
				lista_de_marcadores_identificados.push_back( marcador_visto );

				((*landmkr).se_eh_canto ? lista_de_corners : lista_de_goalposts).push_back( marcador_visto );
			}	
		}
	}


	// Processando linhas
	for(
		const World::gLinha& glinha : mundo_existente.linhas_esfericas
	){

		lista_de_todos_os_segmentos.emplace_back(
												  glinha.inicio,
												  glinha.final 
												);
	}


	obter_marcadores_de_campo();
}


void RobovizField::atualizar_marcadores_a_partir_da_transformacao(
	const Matriz& matriz_de_transformacao
){
	/*
	Descrição:
		Identificação de linhas de segmentos a partir da matriz de 
		transformação de coordenadas.
	
		- Priorizando as maiores linhas, pois são mais únicas.
		- Uma linha de segmento será identificada se houver apenas uma
		outra linha real próxima.
		- Caso haja mais de uma linha real próxima, mas todas as outras
		candidatas já foram identificadas, então a associação ainda pode
		ser feita.
	*/

	// Construimos a lista ordenada
	vector<Linha*> linhas_com_comprimento_descendente;
	{
		for(
			Linha& linha : lista_de_todos_os_segmentos
		){
			
			linhas_com_comprimento_descendente.push_back( &linha );
		}


		// Aplicamos a função anônima
		sort(
			linhas_com_comprimento_descendente.begin(),
			linhas_com_comprimento_descendente.end(),
			[]( 
				const Linha* linha_a,
				const Linha* linha_b
			){
				// Ordenamos pelo maior comprimento.
				return ((*linha_a).comprimento > (*linha_b).comprimento);
			}
		);
	}


	// Identificamos linhas
	{
		/*
		Máxima Distância Permitida para fazermos a associação imediata
		*/
		const float max_dist_para_associacao = 0.3; 

		/*
		A diferença de comprimento é um fator decisivo para a associação
		*/
		const float max_diff_de_comprimento = 0.7;

		/*
		Máxima diferença permitida entre ângulos de linhas
		*/
		const float max_diff_de_angulacao_em_graus = 15;
		const float max_diff_de_angulacao = max_diff_de_angulacao_em_graus * 3.14159265f / 180.f;
		

		for(
			const Linha* linha_qualquer : linhas_com_comprimento_descendente
		){

			// Criação de formas mais simples de manipulação.
			Vetor3D pontos_da_linha_abs[2] = {
				matriz_de_transformacao * (*linha_qualquer).inicio_c,
				matriz_de_transformacao * (*linha_qualquer) .final_c
			};
			float angulo_da_linha = atan2f(
											pontos_da_linha_abs[1].y - pontos_da_linha_abs[0].y,
											pontos_da_linha_abs[1].x - pontos_da_linha_abs[0].x
										  );  // Estará em radiano, lembre-se.

			const sSegmento* melhor_linha_candidata = nullptr;

			// Buscar pela melhor candidata
			for(
				const sSegmento& ssegm : cSegmentos::list
			){

				// Possibilidades de negação desta candidata
				{
					if(
						(
							// Pular caso haja uma diferença de tamanho significativa
							((*linha_qualquer).comprimento - ssegm.comprimento) > max_diff_de_comprimento
						) || (
							// Pular caso haja uma diferença de ângulação significativa
							normalize_line_angle_rad(angulo_da_linha - ssegm.ang) > max_diff_de_angulacao
						)
					){

						continue;
					}


					// Pular caso ela já tenha sido identificada
					bool ja_verificada = false;
					for(
						const sSegmMkr& elemento_de_linha : lista_de_segmentos_identificados
					){

						if(
							elemento_de_linha.segm == &ssegm
						){
							ja_verificada = true;
							break;
						}
					}
					if(
						ja_verificada
					){

						continue;
					}
				}

				// Verificamos condições de associação e finalmente 
				{
					/*
					Erro será a soma da distância do segmento de linha para ambas extremidades
					da linha que estamos verificando, a qual tbm está no plano.

					Ambas funções estão disponíveis dentro do .h
					*/
					float error = calcular_dist_segm_para_pt2d_c(
																 ssegm,
																 pontos_da_linha_abs[0].para_2D()
															   );

					// Fazemos separado para economizar recursos
					if(
						error < max_dist_para_associacao
					){

						error += calcular_dist_segm_para_pt2d_c(
																 ssegm,
																 pontos_da_linha_abs[1].para_2D()
															   );
					}
					else{
						continue;
					}

					// Caso ainda seja menor neste caso
					if(
						error > max_dist_para_associacao
					){

						continue;
					}
				}

				// Fazemos a Associação
				{
					if(
						melhor_linha_candidata == nullptr
					){

						melhor_linha_candidata = &ssegm;
					}
					else{

						/*
						Não ironicamente não podemos usar operador terciário,
						pois este break precisa ser ativado caso venha para cá.

						Afinal, se houver mais de duas candidatas muito boas,
						devemos abordar.
						*/
						melhor_linha_candidata = nullptr;
						break;
					}
				}
			}

			// Caso não haja candidatas
			if(
				melhor_linha_candidata == nullptr
			){

				continue;
			}

			// Então há uma melhor linha candidata
			{

				// Correção de Ordem dos Pontos da Linha
				int ordem_dos_pontos[2] = {0, 1};
				{

					if(
						normalize_vector_angle_rad(
							angulo_da_linha - (*melhor_linha_candidata).ang
						) > 1.57079633f
					){

						ordem_dos_pontos[0] = 1;
						ordem_dos_pontos[1] = 0;
					}
				}

				// Array de Ponteiros
				const Vetor3D *linha_abs[2] = {
					&pontos_da_linha_abs[ ordem_dos_pontos[0] ],
					&pontos_da_linha_abs[ ordem_dos_pontos[1] ]
				};

				const Vetor3D *linha_esf[2] = {
					&((*linha_qualquer).obter_ponto_esf(ordem_dos_pontos[0])),
					&((*linha_qualquer).obter_ponto_esf(ordem_dos_pontos[1]))
				};

				const Vetor3D *linha_cart[2] = {
					&((*linha_qualquer).obter_ponto_cart(ordem_dos_pontos[0])),
					&((*linha_qualquer).obter_ponto_cart(ordem_dos_pontos[1]))
				};


				// Consertar coordenadas absolutas se está no campo de visão
				bool se_esta_no_campo_de_visao[2] = {false, false};
				const float margem_de_seguranca_para_angulo_do_cone = 5;
				const float min_dist_para_evitar_dist_focal = 0.2;
				/*
				---> Reconhecer extremidades da linha como pontos de campo.
				
				Vamos considerar que haja ruído nos dados recebidos, com isso vamos
				realizar os cálculos em cima dos dados hipoteticamente reais.

				- Noise applied horizontally sigma=0.1225,  Pr[-0.5< x <0.5]=0.99996
				- Noise applied vertically   sigma=0.1480,  Pr[-0.5< x <0.5]=0.99928


				Há dois cuidados a serem atentados:

				@ O servidor limita a distância focal para 10cm, especialmente na coordenada
				x. Logo, um ponto mais próximo que isso não será visto pela câmera.

				Sol: Impor que o ponto só será aceito se estiver a mais de 20 cm à frente.
				
				@ Às vezes, o erro de phi e de theta é maior que o esperado, produzindo pontos
				carteasianos e polares incoerentes.

				Sol: O simulador usa um cone de visão, como campo. Calculamos o ângulo
				entre o vetor do ponto e o eixo frontal, (1, 0, 0), sendo o ang_cone. Finalmente,
				limitamos a aceitação apenas a pontos com uma margem de segurança de 5° graus para
				evitar pontos nas bordas do cone.
				*/
				for(
					int i = 0;
						i < 2;
						i++
				){

					float ang_cone = acosf( (*linha_cart[i]).x / (*linha_esf[i]).x );
					const float max_cone_ang = (cHalfHorizontalFoV - margem_de_seguranca_para_angulo_do_cone) * (3.14159265f / 180.f);

					if(
						ang_cone < max_cone_ang && (*linha_cart[i]).x > min_dist_para_evitar_dist_focal
					){

						lista_de_marcadores_identificados.emplace_back(
																		(*melhor_linha_candidata).spts[i],
																		*linha_esf[i],
																		*linha_cart[i]
																	  );
						se_esta_no_campo_de_visao[i] = true;
					}
				}

				// Usaremos as coordenadas se o ponto foi confirmado. Caso não, faremos uma estimativa.
				const Linha linha_de_campo(
					(*(*melhor_linha_candidata).spts[0]).obter_vetor(),
					(*(*melhor_linha_candidata).spts[1]).obter_vetor(),
					(*melhor_linha_candidata).comprimento
				);

				sVetor3D linha_em_svet[2];

				for(
					int i = 0;
						i < 2;
						i++
				){

					// Se no campo de visão.
					if(
						se_esta_no_campo_de_visao[i]
					){

						linha_em_svet[i] = (*(*melhor_linha_candidata).spts[i]).svet;
					}
					else{
						
						// Caso não, vamos pegar o ponto mais próximo.
						Vetor3D ponto = linha_de_campo.segment_ponto_na_reta_mais_perto_cart(
																							  *linha_abs[i]
																							);

						linha_em_svet[i].setar(ponto);

						lista_de_marcadores_desconhecidos.emplace_back(
																		melhor_linha_candidata,
																		linha_em_svet[i],
																		*linha_esf[i],
																		*linha_cart[i]
																	  );
					}
				}


				// Salvar finalmente
				lista_de_segmentos_identificados.emplace_back(
					sMarcador(
						linha_em_svet[0],
						*linha_esf[0],
						*linha_cart[0]
					),
					sMarcador(
						linha_em_svet[1],
						*linha_esf[1],
						*linha_cart[1]
					),
					(*linha_qualquer).comprimento,
					melhor_linha_candidata
				);
			}
		}
	}
}


void RobovizField::atualizar_marcadores_desconhecidos_a_partir_da_transformacao(
	const Matriz& matriz_de_transformacao
){
	/*
	Atualizar a posição absoluta dos marcadores desconhecidos baseado
	na matriz de transformação e em linhas existentes.
	*/

	for(
		sMarcador& desconhecido : lista_de_marcadores_desconhecidos
	){

		Vetor3D pos_abs = matriz_de_transformacao * (desconhecido.pos_rel_cart);

		const Linha linha_de_campo(
			(*(*(desconhecido.segm)).spts[0]).obter_vetor(),
			(*(*(desconhecido.segm)).spts[1]).obter_vetor(),
			(*(desconhecido.segm)).comprimento
		);

		Vetor3D pos_abs_fixada = linha_de_campo.segment_ponto_na_reta_mais_perto_cart(pos_abs);

		desconhecido.spos_abs = sVetor3D( {pos_abs_fixada.x, pos_abs_fixada.y, pos_abs_fixada.z} );
	}	
}


/*//////////////////////////////////////////////////////////////////////////////

Métodos de Desenho

*//////////////////////////////////////////////////////////////////////////////


void RobovizField::ilustrador(
	const Matriz& Head_to_Field,
	int decisor
){
	/*
	Descrição:
		Desenhará todos os elementos vísiveis na tela.
		No original, havia 2 funções quase que exatamente iguais. Reduzi para que fossem
		as mesmas.

		Utiliza o decisor para ter informações de lado.
	*/

	string nome_para_buffer = "localization";

	// Observe que isso não é um Singular<RobovizLogger>, apesar de ser semelhante.
	// Sugiro que leia a definição em RobovizLogger.h
	RobovizLogger *roboviz = RobovizLogger::obter_instancia(); 

	/*
	Será inicializada apenas quando a função desenhar_visiveis for chamada,
	oq deverá ocorrer apenas uma.
	*/
	int resultado_de_inicializacao = (*roboviz).init();

	printf("\n-> Resultado da Inicialização do RobovizLogger dentro de ilustrador(): %d\n", resultado_de_inicializacao);


	// Desenhar todas as linhas, não importando se estão identificadas ou não.
	{

		for(
			const Linha& linha_qualquer : lista_de_todos_os_segmentos
		){

			Vetor3D inicio_c_transf = Head_to_Field * linha_qualquer.inicio_c;
			Vetor3D final_c_transf  = Head_to_Field * linha_qualquer .final_c;

			(*roboviz).desenhar_linha(
										decisor * inicio_c_transf.x,
										decisor * inicio_c_transf.y,
										inicio_c_transf.z,

										decisor * final_c_transf.x,
										decisor * final_c_transf.y,
										final_c_transf.z,

										1,  
								        0.8, // R 
									  	0,	 // G
									   	0,	 // B
									   	&nome_para_buffer
									);
		}
	}

	// Desenhar segmentos de linha identificados
	{

		for(
			const sSegmMkr& ssegm_mkr : lista_de_segmentos_identificados
		){

			Vetor3D mid = Vetor3D::ponto_medio(
												ssegm_mkr.pontos_marcadores[0].spos_abs.obter_vetor(),
												ssegm_mkr.pontos_marcadores[1].spos_abs.obter_vetor()
											  );

			string nome_da_linha_identificada = (*ssegm_mkr.segm).tag;

			(*roboviz).desenhar_anotacao(
										  &nome_da_linha_identificada,
										  decisor * mid.x,
										  decisor * mid.y,
										  mid.z,

										  0, // R
										  1, // G
										  0, // B

										  &nome_para_buffer
										);

			(*roboviz).desenhar_linha(
									   decisor * ssegm_mkr.pontos_marcadores[0].spos_abs.x,
									   decisor * ssegm_mkr.pontos_marcadores[0].spos_abs.y,
									   ssegm_mkr.pontos_marcadores[0].spos_abs.z,

									   decisor * ssegm_mkr.pontos_marcadores[1].spos_abs.x,
									   decisor * ssegm_mkr.pontos_marcadores[1].spos_abs.y,
									   ssegm_mkr.pontos_marcadores[1].spos_abs.z,

									   3,

									   0,   // R
									   0.8, // G
									   0,   // B

									   &nome_para_buffer
									 );
		}


		for(
			const sMarcador& mkr : lista_de_marcadores_identificados
		){

			string nome_do_marcador_identificado = (*(mkr.spt)).tag;

			(*roboviz).desenhar_anotacao(
										  &nome_do_marcador_identificado,
										  decisor * mkr.spos_abs.x,
										  decisor * mkr.spos_abs.y,
										  mkr.spos_abs.z + 1, // para_termos_um_salto_de_altura

										  1, // R
										  0, // G
										  0, // B

										  &nome_para_buffer
										);

			(*roboviz).desenhar_linha(
									   decisor * mkr.spos_abs.x,
									   decisor * mkr.spos_abs.y,
									   mkr.spos_abs.z,

									   decisor * mkr.spos_abs.x,
									   decisor * mkr.spos_abs.y,
									   mkr.spos_abs.z + 0.5,

									   1,

									   0.8, // R
									   0.8, // G
									   0.8, // B

									   &nome_para_buffer
									 );
		}


		for(
			const sMarcador& mkr : lista_de_marcadores_desconhecidos
		){

			string nome_do_marcador_desconhecido = "?";  // Acho mais apropriado

			(*roboviz).desenhar_anotacao(
										  &nome_do_marcador_desconhecido,
										  decisor * mkr.spos_abs.x,
										  decisor * mkr.spos_abs.y,
										  mkr.spos_abs.z + 1, // para_termos_um_salto_de_altura

										  1, // R
										  0, // G
										  0, // B

										  &nome_para_buffer
										);

			(*roboviz).desenhar_linha(
									   decisor * mkr.spos_abs.x,
									   decisor * mkr.spos_abs.y,
									   mkr.spos_abs.z,

									   decisor * mkr.spos_abs.x,
									   decisor * mkr.spos_abs.y,
									   mkr.spos_abs.z + 0.5,

									   1,

									   0.8, // R
									   0.8, // G
									   0.8, // B

									   &nome_para_buffer
									 );
		}
	}

	// Desenhar o jogador e a bola
	{
		
		Vetor3D agente = Head_to_Field.obter_vetor_de_translacao();

		// Desenhando seta para baixo logo acima do agente
		{
			(*roboviz).desenhar_linha(
										decisor * agente.x,
										decisor * agente.y,
										agente.z,

										decisor * agente.x,
										decisor * agente.y,
										agente.z + 0.5,

										2,

										1,
										0,
										0,

										&nome_para_buffer
									 );

			(*roboviz).desenhar_linha(
										decisor * agente.x,
										decisor * agente.y,
										agente.z,

										decisor * (agente.x - 0.2),
										decisor * agente.y,
										agente.z + 0.2,

										2,

										1,
										0,
										0,

										&nome_para_buffer
									 );

			(*roboviz).desenhar_linha(
										decisor * agente.x,
										decisor * agente.y,
										agente.z,

										decisor * (agente.x + 0.2),
										decisor * agente.y,
										agente.z + 0.2,

										2,

										1,
										0,
										0,

										&nome_para_buffer
									 );
		}

		// Podemos desenhar a bola, mas talvez seja desnecessário.

		(*roboviz).enviar_buffer_limpo(&nome_para_buffer);
	}
}
