#include "RobovizField.h"
//#include "RobovizLogger.h"
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
			}


		}
	}
}

























int main(){




	printf("\n\nCompilado e executado com sucesso.\n\n");
	return 0;
}

