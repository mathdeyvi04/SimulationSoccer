#include "RobovizLogger.h"
#include "RobovizField.h"

static World& mundo_existente = Singular<World>::obter_instancia();

/*
Único motivo de precisarmos deste .cpp são as seguintes definições

O static garante que essa variável fique viva sempre, até a completa
finalização do programa.

Precisamos usar essas linhas a seguir para poder foçar a alocação, pois
se fosse apenas no header, não haveria alocação.
*/
decltype(RobovizField::cRingLineLength)             constexpr RobovizField::cRingLineLength;
decltype(RobovizField::cPenaltyBoxDistX)            constexpr RobovizField::cPenaltyBoxDistX;
decltype(RobovizField::cHalfPenaltyWidth)           constexpr RobovizField::cHalfPenaltyWidth;
decltype(RobovizField::cHalfGoalWidth)              constexpr RobovizField::cHalfGoalWidth;
decltype(RobovizField::cHalfFieldLength)            constexpr RobovizField::cHalfFieldLength;
decltype(RobovizField::cGoalWidth)                  constexpr RobovizField::cGoalWidth;
decltype(RobovizField::cGoalDepth)                  constexpr RobovizField::cGoalDepth;
decltype(RobovizField::cGoalHeight)                 constexpr RobovizField::cGoalHeight;
decltype(RobovizField::cFieldLength)                constexpr RobovizField::cFieldLength;
decltype(RobovizField::cFieldWidth)                 constexpr RobovizField::cFieldWidth;
decltype(RobovizField::cPenaltyLength)              constexpr RobovizField::cPenaltyLength;
decltype(RobovizField::cPenaltyWidth)               constexpr RobovizField::cPenaltyWidth;
decltype(RobovizField::cSegmentos::list)            constexpr RobovizField::cSegmentos::list;
decltype(RobovizField::cPontos::list)               constexpr RobovizField::cPontos::list;

decltype(RobovizField::gMkrs::list)                           RobovizField::gMkrs::list;

//////////////////////////////////////////////////////////////////////////////////////////////////

void 
RobovizField::obter_marcadores_de_campo() {
	/*
	  Descrição:
	    Reúne os marcadores com coordenada Z absoluta conhecida no campo, como pontos de contato dos pés e extremos de segmentos de linha.
	    Esses marcadores são utilizados na estimativa de posição e orientação do agente com base em referências fixas do campo.

	  Parâmetros:
	    Nenhum.

	  Retorno:
	    Nenhum retorno direto. 
	    Popula as listas:
	      - list_feet_contact_points
	      - list_ground_markers
	      - list_weighted_ground_markers
	    Também atualiza o valor de `non_collinear_ground_markers`.
	*/

    /*
	Adiciona os pontos de contato dos pés do robô NAO ao conjunto de marcadores do chão.
	Dependência: os pés do agente devem estar em contato com o chão.
	Limitações:
	- se os pés estiverem tocando outros jogadores ou a bola, pode haver erro (solucionável se necessário).
	- robôs podem tocar o chão com os dedos dos pés, o que atualmente não é considerado, (aloprado).
    */
    for(
    	int i = 0;
     		i < 2;
    		i++
    ){
        if(
        	mundo_existente.pe_em_contato[i]
        ){ 

            list_feet_contact_points.emplace_back(
            									  sVetor3D({0, 0, 0}), 
            									  mundo_existente.pos_rel_contato_pe[i].to_esfe(), 
            									  mundo_existente.pos_rel_contato_pe[i]
            									  );
            list_ground_markers.emplace_back     (
            									  sVetor3D({0, 0, 0}), 
            									  mundo_existente.pos_rel_contato_pe[i].to_esfe(), 
            									  mundo_existente.pos_rel_contato_pe[i]
            									  );
        }
    }

    // Adiciona todos os extremos de segmentos de linha aos marcadores do chão
    for(
    	const Linha6D& linha : list_segments
    ){

        list_ground_markers.emplace_back(
        								sVetor3D({0, 0, 0}), 
        								linha.ponto_inicial_esferica, 
        								linha.ponto_inicial_cartesiano
        								);
        list_ground_markers.emplace_back(
        								sVetor3D({0, 0, 0}), 
        								linha.ponto_final_esferica, 
        								linha.ponto_final_cartesiano
        								);
    }

    // Define o número de marcadores não colineares (não inclui bandeiras de canto)
    marcadores_nao_colineares = list_ground_markers.size();

    // Adiciona as bandeiras de canto visíveis
    for(
    	const auto& mkr : list_landmarks_corners
    ){

        list_ground_markers.emplace_back(
        								sVetor3D({0, 0, 0}),
        								mkr.pos_rel_esf, 
        								mkr.pos_rel_cart
        								);
    }

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
	    repetitions for    2 meters: 55
	    repetitions for >=19 meters:  1
		
		Onde K é apenas uma constante númerica.
	*/
    for (
    	const auto& mkr : list_ground_markers
    ) {
    	// Observe que a componente x é a distância
        float var = pow(mkr.pos_rel_esf.x / 100.f, 2) * var_distance + var_round_hundredth;
        int repeticoes = max(int(1.f / (1500.f * var)), 1);

        list_weighted_ground_markers.insert(
        	list_weighted_ground_markers.end(), 
        	repeticoes,
        	mkr
        );
    }
}

void 
RobovizField::atualizar_marcadores(){
	/*
	Descrição:
	    Atualiza os marcadores visuais após uma etapa de percepção.
	    Essa função limpa os dados antigos e repopula as listas de 
	    marcadores visíveis, extremidades de linhas e pontos de contato com o solo.
	    Ela processa landmarks visíveis e linhas observadas, marcando quais objetos
	    estão disponíveis para uso em etapas posteriores como estimativa de pose e localização.

	Parâmetros:
	    Nenhum.

	Retorno:
	    Nenhum valor de retorno. 
	    Atualiza os seguintes vetores da classe `RobovizField`:
	        - list_landmarks
	        - list_landmarks_corners
	        - list_landmarks_goalposts
	        - list_known_markers
	        - list_segments
	        - list_ground_markers
	        - list_weighted_ground_markers
	        - list_feet_contact_points
	        - list_known_segments
	        - list_unknown_markers
	*/

	// Não é necessário reservar espaço, já que os vetores irão expandir mais nos
	// primeiros ciclos.
    list_segments.clear();
    list_landmarks.clear();
    list_landmarks_corners.clear();
    list_landmarks_goalposts.clear();
    list_feet_contact_points.clear();
    list_known_markers.clear();
    list_unknown_markers.clear();
    list_known_segments.clear();
    list_ground_markers.clear();
    list_weighted_ground_markers.clear();

    // Preparando as list's landmarks
    for(
    	int i = 0; 
    	i < 8; 
    	i++
    ){
        sFixedMkr    *landmark8;  // Há 8 possibilidades no total.
        const sPonto *ponto_de_referencia;
        World::gMkr  *landmark = &mundo_existente.landmark[i];

        // Identifica qual dos 8 pontos fixos está sendo observado
        if      (landmark->pos_absoluta.x == -15 && landmark->pos_absoluta.y == -10) { landmark8 = &gMkrs::_corner_mm; ponto_de_referencia = &cPontos::corner_mm; }
        else if (landmark->pos_absoluta.x == -15 && landmark->pos_absoluta.y == +10) { landmark8 = &gMkrs::_corner_mp; ponto_de_referencia = &cPontos::corner_mp; }
        else if (landmark->pos_absoluta.x == +15 && landmark->pos_absoluta.y == -10) { landmark8 = &gMkrs::_corner_pm; ponto_de_referencia = &cPontos::corner_pm; }
        else if (landmark->pos_absoluta.x == +15 && landmark->pos_absoluta.y == +10) { landmark8 = &gMkrs::_corner_pp; ponto_de_referencia = &cPontos::corner_pp; }
        else if (landmark->pos_absoluta.x == -15 && landmark->pos_absoluta.y < 0)    { landmark8 = &gMkrs::_goal_mm;   ponto_de_referencia = &cPontos::goal_mm; }
        else if (landmark->pos_absoluta.x == -15 && landmark->pos_absoluta.y > 0)    { landmark8 = &gMkrs::_goal_mp;   ponto_de_referencia = &cPontos::goal_mp; }
        else if (landmark->pos_absoluta.x == +15 && landmark->pos_absoluta.y < 0)    { landmark8 = &gMkrs::_goal_pm;   ponto_de_referencia = &cPontos::goal_pm; }
        else if (landmark->pos_absoluta.x == +15 && landmark->pos_absoluta.y > 0)    { landmark8 = &gMkrs::_goal_pp;   ponto_de_referencia = &cPontos::goal_pp; }
        else { return; }

        // Se o landmark foi visto, registra a posição relativa e o marca como visível
        if(
        	landmark->detectado
        ){

            landmark8->set_pos_rel( landmark->pos_relativa );
            landmark8->detectado = true;

            sMkr mkr_visto( ponto_de_referencia, landmark->pos_relativa );
            list_landmarks.push_back( mkr_visto );
            list_known_markers.push_back( mkr_visto );

            ( (landmark->eh_canto) ? list_landmarks_corners : list_landmarks_goalposts ).push_back(mkr_visto);
        }
        else{

            landmark8->detectado = false;
        }
    }

    // Preparando as list's lines
    for(
    	const auto& linha : mundo_existente.linhas_esfericas
    ){
        list_segments.emplace_back(linha.ponto_inicial, linha.ponto_final);
    }

    // Reúne os marcadores com coordenada Z absoluta conhecida
    // (extremos de linhas e pontos de contato dos pés)
    obter_marcadores_de_campo();
}

void
RobovizField::atualizar_marcadores_por_transformacao( const Matriz4D& Head_to_Field ){
	/*
	Descrição:
	    Atualiza os segmentos de linha visíveis com base em uma matriz de transformação,
	    identificando quais linhas do campo correspondem às linhas observadas.
	    
	    A identificação é realizada do segmento mais longo para o mais curto, com base na similaridade angular e proximidade.
	    Se uma linha visível se encaixa apenas em uma linha real do campo (ou se todas as outras candidatas já foram atribuídas),
	    então ela é considerada identificada.

	    A função também atualiza os pontos finais conhecidos e desconhecidos, e adiciona os segmentos corretamente ordenados
	    à lista de segmentos identificados.

	Parâmetros:
	    - Head_to_Field:
	    	Matriz 4x4 (Matrix4D) de transformação do sistema de coordenadas 
	    	da cabeça do agente para o campo.

	Retorno:
	    - Nenhum valor de retorno. 
	    Atualiza internamente os seguintes vetores:
	        - list_known_segments
	        - list_known_markers
	        - list_unknown_markers
	*/

	vector<Linha6D*> lines_descending_length;
    for(
    	Linha6D& linha_qualquer : list_segments
    ){

        lines_descending_length.push_back(
        								 &linha_qualquer
        								 );
    }	

    // Ordenamos da maior para menor.
    sort(
    	lines_descending_length.begin(),
    	lines_descending_length.end(), 
        []( // -> Função Anônima de Comparação
        	const Linha6D* a, const Linha6D* b
        ) { 

        	return (a->comprimento > b->comprimento); 
        }
    );

    // Identificamos as linhas 
    for(
    	const Linha6D* linha : lines_descending_length
    ){
        Vetor3D linha_absoluta[2] = {
        	Head_to_Field * linha->ponto_inicial_cartesiano, 
        	Head_to_Field * linha->ponto_final_cartesiano
       	}; 

        float l_angle = atan2f( // Em radianos!
        	linha_absoluta[1].y - linha_absoluta[0].y, 
        	linha_absoluta[1].x - linha_absoluta[0].x
        );

        // Calculamos a distância para a linha mais próxima
        const float min_error = 0.3; // Máxima Distância Permitida (startdist + enddist < 0.3m)
        const sSegmento* melhor_segm = nullptr;
		for(
			const sSegmento& segm : cSegmentos::list
		){ 

			if(
				(
					// Pular caso haja uma diferença de tamanho significativa
					(linha->comprimento - segm.comprimento) > 0.7
				) || (
					// Pular caso haja uma diferença de ângulação significativa
					normalize_line_angle_rad(l_angle - segm.ang) > 0.26  // 15° de diferença
				)
			){

				continue;
			}

            // Pular caso ela já tenha sido identificada
			bool ja_verificada = false;
			for(
				const sSegmMkr& elemento_de_linha : list_known_segments
			){

				if(
					elemento_de_linha.segm == &segm
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
			
			// Calcular os respectivos erros
			float error = calcular_dist_segm_para_pt2D_c( segm, linha_absoluta[0].to_2d() );
			if( error < min_error ) { error += calcular_dist_segm_para_pt2D_c( segm, linha_absoluta[1].to_2d() ); }

			// Não é possível fazer o operador terciário...
			if(
				melhor_segm == nullptr
			){

				melhor_segm = &segm;
			}
			else{

				/*
				Se houver mais de duas candidatas, devemos abortar.
				*/
				melhor_segm = nullptr;
				break;
			}
		}

		// Verificamos se não foi possível obter uma candidata
		if(
			melhor_segm == nullptr
		){

			continue;
		}

		// Caso tenhamos uma candidata
		
		// Vamos consertar a ordem dos pontos
		int ordem_dos_pontos[2] = {0, 1}; 
		if(
			normalize_vector_angle_rad(
				l_angle - melhor_segm->ang
			) > 1.57079633f
		){

			ordem_dos_pontos[0] = 1;
			ordem_dos_pontos[1] = 0;
		}

		const Vetor3D *linha_absoluta_correta[2] = {& linha_absoluta               [ ordem_dos_pontos[0] ], & linha_absoluta               [ ordem_dos_pontos[1] ]};
		const Vetor3D *linha_rel_esferica[2]     = {& linha->obter_ponto_esferico  ( ordem_dos_pontos[0] ), & linha->obter_ponto_esferico  ( ordem_dos_pontos[1] )};
		const Vetor3D *linha_rel_cartesiano[2]   = {& linha->obter_ponto_cartesiano( ordem_dos_pontos[0] ), & linha->obter_ponto_cartesiano( ordem_dos_pontos[1] )};
	
		// Primeira etapa: reconhecer se as extremidades da linha observada estão dentro do campo de visão (FoV)
		// e, se sim, registrá-las como marcadores conhecidos.

		/*
		Solução geral:

		Todos os pontos que estiverem razoavelmente dentro do campo de visão (FoV) são considerados válidos e identificáveis.

		Precisão do sensor (modelo de ruído aplicado):
		- Horizontal (ângulo phi): desvio padrão sigma = 0.1225 → P[-0.5 < x < 0.5] ≈ 99.996%
		- Vertical (ângulo theta): desvio padrão sigma = 0.1480 → P[-0.5 < x < 0.5] ≈ 99.928%

		Observações importantes:
		1. O servidor impõe um limite mínimo na distância focal de 10 cm (controlado pela coordenada X no sistema cartesiano).
		   → Solução atual: considerar apenas pontos cuja coordenada X relativa seja maior que 0.2 metros.

		2. Em alguns casos, os erros dos ângulos phi e theta podem ser maiores do que o previsto, resultando em pontos
		   distorcidos, por exemplo:
		      (coordenadas esféricas: 0.57, -36.36, -54.41) → (coordenadas cartesianas: 0.267, -0.1967, -0.4635)

		   Isso indica que o campo de visão (FoV) do sensor pode estar modelado como um cone, e não como um plano,
		   o que seria mais realista fisicamente.

		   → Solução atual: aceitar apenas pontos cujo ângulo do cone seja menor que (FoV_horizontal - 5 graus)
		*/

		bool extremidade_visivel[2] = {false, false};

		for(
			int i = 0;
				i < 2;
				i++
		){	
			// ângulo entre vetor e (1, 0, 0)
			float cone_angle = acosf( linha_rel_cartesiano[i]->x / linha_rel_esferica[i]->x ); 
            const float max_cone_angle = ( cHalfHorizontalFoV - 5 ) * M_PI / 180;

            if(
            	cone_angle < max_cone_angle && linha_rel_cartesiano[i]->x > 0.2
            ){

                list_known_markers.emplace_back( 
            								   melhor_segm->pt[i],
            								   *linha_rel_esferica[i],
            								   *linha_rel_cartesiano[i] 
            								   );
                extremidade_visivel[i] = true;
            }
        }

        // Segunda etapa: usar coordenadas reais se o ponto foi reconhecido. Caso não,
        // salvar como desconhecido.

        const Linha6D melhor_linha(
        						  melhor_segm->pt[0]->obter_vetor(),
        						  melhor_segm->pt[1]->obter_vetor(),
        						  melhor_segm->comprimento
        						  );

        // Usamos isso devido à maior precisão, já que as componentes estão em double
        sVetor3D pts_da_melhor_linha[2];

        for(
        	int i = 0;
        		i < 2;
        		i++
        ){

        	if(
        		extremidade_visivel[i]
        	){

        		pts_da_melhor_linha[i] = melhor_segm->pt[i]->svet;
        	}
        	else{

        		Vetor3D ponto = melhor_linha.ponto_mais_proximo_no_segmento_para_ponto_cartesiano( *linha_absoluta_correta[i] );

        		pts_da_melhor_linha[i].setar( ponto );

        		list_unknown_markers.emplace_back( melhor_segm, pts_da_melhor_linha[i], *linha_rel_esferica[i], *linha_rel_cartesiano[i] );
        	}
        }

        list_known_segments.emplace_back(
        							   sMkr( pts_da_melhor_linha[0], *linha_rel_esferica[0], *linha_rel_cartesiano[0] ),
        							   sMkr( pts_da_melhor_linha[1], *linha_rel_esferica[1], *linha_rel_cartesiano[1] ),
        							   linha->comprimento,
        							   melhor_segm
        							   );
	}	
}

void 
RobovizField::atualizar_marcadores_desconhecidos_por_transformacao( const Matriz4D& Head_to_Field ){
	/*
	Descrição:
	    Atualiza a posição absoluta de marcadores visuais desconhecidos (isto é, cujos pontos finais ainda
	    não foram completamente identificados) com base em uma matriz de transformação para o sistema de coordenadas do campo.

	    Para cada marcador desconhecido:
	    - A posição relativa observada (em coordenadas cartesianas) é transformada para o referencial do campo (mundo).
	    - Essa posição é então projetada na linha do campo à qual o marcador está associado, garantindo que o ponto final
	      fique corretamente posicionado sobre um segmento real do campo.

	Parâmetros:
	    - Head_to_Field:
	        Matriz de transformação 4x4 (Matrix4D) que representa a transformação do sistema de coordenadas
	        do agente (por exemplo, a cabeça) para o sistema de coordenadas global do campo.

	Retorno:
	    - Sem valor de retorno.
	     A função atualiza diretamente os atributos `pos_abs` dos marcadores presentes no vetor `list_unknown_markers`.
	*/

	for(
		sMkr& mkr : list_unknown_markers
	){

		// Transformar o mkr para coordenadas do mundo
		Vetor3D pos_abs_crua = Head_to_Field * mkr.pos_rel_cart;
		// Obter a linha correspondente ao marcador
		const Linha6D linha_correspondente(
										  mkr.segm->pt[0]->obter_vetor(),
										  mkr.segm->pt[1]->obter_vetor(),
										  mkr.segm->comprimento
										  );
		Vetor3D pos_abs_fixed = linha_correspondente.ponto_mais_proximo_no_segmento_para_ponto_cartesiano( pos_abs_crua );

		mkr.pos_abs = sVetor3D({pos_abs_fixed.x, pos_abs_fixed.y, pos_abs_fixed.z});
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////

void 
RobovizField::ilustrador( const Matriz4D& Head_to_Field, bool is_right_side ) const {
	/*
	Descrição:
	    Responsável por ilustrar graficamente, via RobovizLogger, os elementos visíveis no campo de visão do agente.
	    Esta função desenha:
	        - Todas as linhas visíveis (segmentos observados)
	        - Segmentos identificados, com suas coordenadas absolutas corrigidas
	        - Pontos marcadores conhecidos (com nomes de referência)
	        - Pontos marcadores desconhecidos (representados por "?")
	        - A posição e orientação atual do agente (com uma seta)
	        - Um marcador para a bola (se necessário, pode ser incluído)

	    O desenho é feito no buffer identificado por "ambientacao", que é posteriormente trocado para exibição.

	    Caso o lado do campo seja invertido (`is_right_side == true`), haverá as conversões
	    necessárias.

	Parâmetros:
	    - Head_to_Field:
	        Matriz de transformação (Matriz4D) que converte coordenadas relativas do agente para coordenadas absolutas no campo.

	    - is_right_side:
	        Booleano que indica se o agente está no lado direito do campo. 

	Retorno:
	    - Nenhum valor é retornado.
	      A função realiza chamadas ao RobovizLogger para desenhar graficamente os elementos no campo.
	*/

	int compensador = (is_right_side) ? -1 : 1;

    string nome_buffer = "ambientacao";

    RobovizLogger* roboviz = RobovizLogger::obter_instancia();
    roboviz->init(); // Em casos de necessidade, há um valor retornado para indicar se tudo correu bem.
    
    // Desenha todas as linhas, identificadas ou não
    for(
    	const Linha6D& linha : list_segments
    ){

        Vetor3D ponto_inicial = Head_to_Field * linha.ponto_inicial_cartesiano;
        Vetor3D ponto_final   = Head_to_Field * linha.ponto_final_cartesiano;

        roboviz->desenhar_linha(
        					   compensador * ponto_inicial.x, compensador * ponto_inicial.y, ponto_inicial.z,
        					   compensador * ponto_final.x,   compensador * ponto_final.y,   ponto_final.z,
        					   1,
        					   0.8,0,0,
        					   &nome_buffer
        					   );
    }  

    // Desenha os segmentos de linha identificados, com coordenadas absolutas corrigidas
    for(
    	const sSegmMkr& segm_mkr : list_known_segments
    ){

		Vetor3D mid = Vetor3D::obter_ponto_medio( segm_mkr.pt_mkr[0].pos_abs.obter_vetor(), segm_mkr.pt_mkr[1].pos_abs.obter_vetor());
        
        string nome_da_linha(segm_mkr.segm->tag);
		roboviz->desenhar_anotacao(
								  &nome_da_linha,
								  compensador * mid.x, compensador * mid.y, mid.z,
								  0,1,0,
								  &nome_buffer
								  );
        roboviz->desenhar_linha(
        						compensador * segm_mkr.pt_mkr[0].pos_abs.x, compensador * segm_mkr.pt_mkr[0].pos_abs.y, segm_mkr.pt_mkr[0].pos_abs.z, 
	                            compensador * segm_mkr.pt_mkr[1].pos_abs.x, compensador * segm_mkr.pt_mkr[1].pos_abs.y, segm_mkr.pt_mkr[1].pos_abs.z,
	                            3,
	                            0,0.8,0, 
	                            &nome_buffer
	                            );
	}

	for(
		const sMkr& mkr : list_known_markers
	){

	    string nome_da_linha(mkr.pt->tag);
	    roboviz->desenhar_anotacao(
	        &nome_da_linha,
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z + 1,
	        1, 0, 0,
	        &nome_buffer
	    );

	    roboviz->desenhar_linha(
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z,
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z + 0.5,
	        1,
	        0.8, 0.8, 0.8,
	        &nome_buffer
	    );
	}

	for(
		const sMkr& mkr : list_unknown_markers
	){
	    string nome_da_linha = "?";

	    roboviz->desenhar_anotacao(
	        &nome_da_linha,
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z + 1,
	        1, 0, 0,
	        &nome_buffer
	    );

	    roboviz->desenhar_linha(
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z,
	        compensador * mkr.pos_abs.x, compensador * mkr.pos_abs.y, mkr.pos_abs.z + 0.5,
	        1,
	        0.8, 0.8, 0.8,
	        &nome_buffer
	    );
	}

    // Desenhar flechas para o jogador

    Vetor3D me = Head_to_Field.obter_vetor_de_translacao();

    roboviz->desenhar_linha(
	    compensador * me.x,     compensador * me.y, me.z,
	    compensador * me.x,     compensador * me.y, me.z + 0.5,
	    2,
	    1, 0, 0,
	    &nome_buffer
	);

	roboviz->desenhar_linha(
	    compensador * me.x,       compensador * me.y, me.z,
	    compensador * (me.x-0.2), compensador * me.y, me.z + 0.2,
	    2,
	    1, 0, 0,
	    &nome_buffer
	);

	roboviz->desenhar_linha(
	    compensador * me.x,       compensador * me.y, me.z,
	    compensador * (me.x+0.2), compensador * me.y, me.z + 0.2,
	    2,
	    1, 0, 0,
	    &nome_buffer
	);
    
    roboviz->criar_buffer_limpo(&nome_buffer);
}
