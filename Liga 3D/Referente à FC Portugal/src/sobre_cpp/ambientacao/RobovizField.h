/*
Funcionalidades Agasalhadas devido ao inexistente uso:

fieldLineSegmentDistToCartPoint() 

*/

#ifndef ROBOVIZFIELD_H
#define ROBOVIZFIELD_H

#include "Singular.h"
#include "AlgLin.h"
#include "World.h"
#include <vector>  // Alocação Dinâmica
#include <array>   // Alocação Estática

using namespace std;

class RobovizField {
	/*
	Descrição:
		Classe responsável por gerenciar e manutenir TODOS OS ELEMENTOS DO CAMPO.
	*/

private:

	RobovizField(){};

	void
	obter_marcadores_de_campo();

	friend class Singular<RobovizField>;

public:

	/*
	Definição de Dimensões de Campo

	Por que não escrever em define?

	- Acredito que à necessidade de relacionar fielmente ao Campo.
	- Necessidade do tempo de compilação, afinal, constexpr.
	- É basicamente a mesma coisa que o define, só que com vantagens.

	Não vou fazer nenhuma alteração, pois:

	- A nomenclatura já está boa.
	- Os números são imutáveis.
	- Desnecessário qualquer explicação complementar.
	*/

	static constexpr double cFieldLength       = 30.0, cFieldWidth = 20.0, cPenaltyLength = 1.8, cPenaltyWidth = 6.0;
	static constexpr double cGoalWidth         = 2.10, cGoalDepth  = 0.60, cGoalHeight    = 0.8;
	static constexpr double cHalfFieldLength   =  cFieldLength/2.0, cHalfFieldWidth    =    cFieldWidth/2.0;
	static constexpr double cHalfGoalWidth     =    cGoalWidth/2.0, cHalfPenaltyLength = cPenaltyLength/2.0;
	static constexpr double cHalfPenaltyWidth  = cPenaltyWidth/2.0;
	static constexpr double cPenaltyBoxDistX   = cHalfFieldLength - cPenaltyLength;
    static constexpr double cRingLineLength    =                1.2360679774997897;
    static constexpr float cHalfHorizontalFoV  = 60;
    static constexpr float cHalfVerticalFoV    = 60;
    /* Para nos auxiliar na confiabilidade das medidas */
    static constexpr float stdev_distance      =            0.0965; //st. deviation of error ed  (distance error=d/100*ed)
    static constexpr float var_distance        =        0.00931225; //     variance of error ed  (distance error=d/100*ed)
    static constexpr float var_round_hundredth =      0.01*0.01/12; //variance of uniformly distributed random variable [-0.005,0.005]

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct sVetor3D  {
		/*
		Struct para representar a classe vetorial Vetor3D.
		
		Não podíamos apenas assumir que é apenas Vetor3D devido à necessidade
		de escrevermos static constexpr, isso obriga que tenhamos apenas
		tipos literais nas estruturas.
		*/

		double x, y, z;
		Vetor3D obter_vetor() const { return Vetor3D(x, y, z); }
		void setar(	const sVetor3D& svet ){ x =  svet.x;  y = svet.y;  z = svet.z; }
		void setar(	const Vetor3D& vetor ){ x = vetor.x; y = vetor.y; z = vetor.z; }
	}; 

	struct sPonto    {
		/*
		Responsável por representar pontos no campo.
		*/
		const sVetor3D svet;
		/*
		Para identificarmos o ponto ou apresentá-lo no 
		Drawing do RoboViz pelo buffer, lembra?

		O compilador literalmente não funcionava pq eu não
		posso, aparentemente, inicializar o array com 0.
		*/
		const char tag[10]; 
		Vetor3D obter_vetor() const { return svet.obter_vetor(); }
	};

	struct sSegmento {
		/*
		Responsável por representar segmentos de linha do campo.
		*/

		/*
		A sintaxe mais cachorra que você verá.
		
		- Cria um array de 2 ponteiros.
		- Cada ponteiro é constante: não pode ser alterado para apontar para outro lugar.
		- Cada ponteiro aponta para um sFieldPoint constante: não pode modificar o objeto apontado.
		- Ou seja: array de ponteiros constantes para objetos constantes.
		*/
		const sPonto * const pt[2];
		const double comprimento;
		/*
		Sempre positivo, em [0, 180°] e correspondente ao vetor final_c - inicio_c.
		*/
		const double ang;
		const char tag[8];
	};

	struct sMkr      {
		/*
		Determinados pontos e segmentos são intrínsecamente mais
		importantes que outros. É para isso servirá esta struct, 
		para marcar quais pontos e quais segmentos são importantes.
		*/

		/* Posição Absoluta Estimada pelo Robô A Partir de Lógicas Específicas */
		sVetor3D pos_abs;

		/*
		Ponteiro para ponto correspondente no campo.
		*/
		const sPonto *pt  = nullptr;

		/*
		Ponteiro para segmento correspondente no campo.
		*/
		const sSegmento *segm = nullptr;

		Vetor3D pos_rel_cart;
		Vetor3D pos_rel_esf;

		/*/////////////////////////////////
		Diversos Construtores Mínimos 
		*//////////////////////////////////

		sMkr() : pos_abs      ({0, 0, 0}), 
			     pos_rel_cart ( 0, 0, 0 ),
			     pos_rel_esf  ( 0, 0, 0 )
			     {};

		/*
		Note que não faz sentido haver um construtor com 
		a posição em cartesiano de entrada se o simulador 
		nos dá apenas em esférica.
		*/
		sMkr(
			const sVetor3D& pos_abs_,
			const  Vetor3D& pos_rel_esf_
		) : pos_abs      ( pos_abs_                     ),
			pos_rel_cart ( pos_rel_esf_.to_cart()       ),
			pos_rel_esf  ( pos_rel_esf_                 )
			{};


		sMkr(
			const sPonto  *pt_,
			const Vetor3D& pos_rel_esf_
		) : pos_abs      ( pt_->svet			 ),
			pt           ( pt_        			 ),
			pos_rel_cart ( pos_rel_esf_.to_cart()),
			pos_rel_esf  ( pos_rel_esf_          )
			{};


		sMkr(
			const Vetor3D& pos_abs_,
			const Vetor3D& pos_rel_esf_
		) : pos_abs     ({pos_abs_.x, pos_abs_.y, pos_abs_.z}),
			pos_rel_cart(pos_rel_esf_.to_cart()              ),
			pos_rel_esf (pos_rel_esf_               		 )
			{};



		sMkr(
			const sVetor3D& pos_abs_,
			const Vetor3D&  pos_rel_esf_,
			const Vetor3D&  pos_rel_cart_
		) : pos_abs     ( pos_abs_      ),
			pos_rel_cart( pos_rel_cart_ ),
			pos_rel_esf ( pos_rel_esf_  )
			{};


		sMkr(
			const sSegmento *segm_,
			const sVetor3D&  pos_abs_,
			const Vetor3D& 	 pos_rel_esf_,
			const Vetor3D& 	 pos_rel_cart_
		) : pos_abs     ( pos_abs_      ),
			segm        ( segm_         ),
			pos_rel_cart( pos_rel_cart_ ),
			pos_rel_esf ( pos_rel_esf_  )
			{};


		sMkr(
			const sPonto  *pt_,
			const Vetor3D& pos_rel_esf_,
			const Vetor3D& pos_rel_cart_
		) :	pos_abs     ( pt_->svet      ),
		    pt          ( pt_            ),
			pos_rel_cart( pos_rel_cart_  ),
			pos_rel_esf ( pos_rel_esf_   )
			{};
	};

	struct sSegmMkr  {
		// 0-> inicial
		// 1-> final
		sMkr pt_mkr[2];
		float comprimento;
		const sSegmento* segm;
		sSegmMkr( 
			const sMkr& pt_mkr_inicial_, 
			const sMkr& pt_mkr_fnal_, 
			float comprimento_,
			const sSegmento* segm_
		) : pt_mkr{pt_mkr_inicial_, pt_mkr_fnal_}, comprimento(comprimento_), segm(segm_) {}
	};

	struct sFixedMkr {

		bool detectado = false;
		Vetor3D pos_rel_esf;
		Vetor3D pos_rel_cart;

		sFixedMkr() : pos_rel_esf({0, 0, 0}), pos_rel_cart({0, 0, 0}) {};

		void
		set_pos_rel( Vetor3D& pos_rel_esf_ ) { pos_rel_esf = pos_rel_esf_; pos_rel_cart = pos_rel_esf_.to_cart(); }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class cPontos    {
    	public:
    	/*
		Precisamos definir quais serão os pontos importantes.
		Usaremos essa sub classe para eles.

		Lista constante de pontos.
    	
    	Após muito tempo tentando descobrir pq sPonto não era um tipo
    	literal, descobri que era porque eu tentei cartear a struct sVetor3D
    	para ser a classe Vetor3D, porém o constexpr obriga que seja literal.
		
		Note que apesar de inicializarmos assim, não há um construtor na struct sPonto, 
		entretanto, lembre-se de como inicializávamos structs em C puro!
		
		Não iremos alterar nomes, pois:

		- Desnecessário
		- Um trabalho de risco, dada importância desses dados.
		*/

		static constexpr array<sPonto,28> list = {{
            /* Traves dos Gols 
			
				post-+           post++
			       |               |
			       |               |
			    post--           post+-
          	*/
            {-cHalfFieldLength,-cHalfGoalWidth, cGoalHeight,   "post--"}, {-cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post-+"}, /* Gol Esquerdo, x < 0 */
            { cHalfFieldLength,-cHalfGoalWidth, cGoalHeight,   "post+-"}, { cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post++"}, /* Gol  Direito, x > 0 */

			/* Cantos Extremos do Campo

				corner-+           corner++
			       +----------------+
			       |                |
			       |                |
			       +----------------+
			    corner--           corner+-
			 */
            {-cHalfFieldLength,-cHalfFieldWidth,          0, "corner--"}, {-cHalfFieldLength, cHalfFieldWidth,0, "corner-+"}, /* Cantos Esquerdos do Campo, x < 0 */
            { cHalfFieldLength,-cHalfFieldWidth,          0, "corner+-"}, { cHalfFieldLength, cHalfFieldWidth,0, "corner++"}, /* Cantos  Direitos do Campo, x > 0 */
            
            /* Pontos Mediais Horizontais
				
            	halfway+
		           |
		           |
		           |
		        halfway-
            */
            {                0,-cHalfFieldWidth,          0, "halfway-"},
            {                0, cHalfFieldWidth,          0, "halfway+"},
            
			/*  Área de Penâlti do Gol 
				Representação Deitada, logo: 

				- linhas horizontais são verticais;
				- linhas verticais são horizontais.
				

				   +-----Fundo do Gol----+
				   |                     |   ← (post)
			boxBack--            boxBack-+
			   +-----------------------------+  ← Linha do gol / fundo da área
			   |                         	 |
			   |            ÁREA        	 |
			   |                           	 |
			   |                         	 |
			boxFrnt--                   boxFrnt-+
			   +-----------------------------+  ← Linha frontal da área
			*/
            {-cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack--"}, {-cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack-+"}, // Penalty box goal line corner x<0
            { cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack+-"}, { cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack++"}, // Penalty box goal line corner x>0
            {-cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt--"}, {-cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt-+"}, // Penalty box front corner x<0
            { cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt+-"}, { cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt++"}, // Penalty box front corner x>0

            /*  Pontos De Um Círculo de Raio 2, note as angulações específicas e em graus 
				
				Acredito que este é muito autodescritivo.
            */
            {                  2,                   0, 0,   "r0"}, { 1.6180339887498948,   1.1755705045849463, 0,  "r36"}, // Ring 0/36    deg
            {0.61803398874989485,  1.9021130325903071, 0,  "r72"}, {-0.61803398874989485,  1.9021130325903071, 0, "r108"}, // Ring 72/108  deg
            {-1.6180339887498948,  1.1755705045849463, 0, "r144"}, {                  -2,                   0, 0, "r180"}, // Ring 144/180 deg
            {-1.6180339887498948, -1.1755705045849463, 0, "r216"}, {-0.61803398874989485, -1.9021130325903071, 0, "r252"}, // Ring 216/252 deg
            {0.61803398874989485, -1.9021130325903071, 0, "r288"}, {  1.6180339887498948, -1.1755705045849463, 0, "r324"}  // Ring 288/324 deg
        }};


        // Tudo está em ordem para facilitar o encontro.
        static constexpr const sPonto &goal_mm = list[0];     //Goalpost x<0 y<0
        static constexpr const sPonto &goal_mp = list[1];     //Goalpost x<0 y>0
        static constexpr const sPonto &goal_pm = list[2];     //Goalpost x>0 y<0
        static constexpr const sPonto &goal_pp = list[3];     //Goalpost x>0 y>0

        static constexpr const sPonto &corner_mm = list[4];   //Corner x<0 y<0
        static constexpr const sPonto &corner_mp = list[5];   //Corner x<0 y>0
        static constexpr const sPonto &corner_pm = list[6];   //Corner x>0 y<0
        static constexpr const sPonto &corner_pp = list[7];   //Corner x>0 y>0

        static constexpr const sPonto &halfway_m = list[8];   //Halfway line ending y<0
        static constexpr const sPonto &halfway_p = list[9];   //Halfway line ending y>0

        static constexpr const sPonto &boxgoal_mm = list[10]; //Penalty box goal line corner x<0 y<0
        static constexpr const sPonto &boxgoal_mp = list[11]; //Penalty box goal line corner x<0 y>0
        static constexpr const sPonto &boxgoal_pm = list[12]; //Penalty box goal line corner x>0 y<0
        static constexpr const sPonto &boxgoal_pp = list[13]; //Penalty box goal line corner x>0 y>0
        static constexpr const sPonto &box_mm = list[14];     //Penalty box front corner x<0 y<0
        static constexpr const sPonto &box_mp = list[15];     //Penalty box front corner x<0 y>0
        static constexpr const sPonto &box_pm = list[16];     //Penalty box front corner x>0 y<0
        static constexpr const sPonto &box_pp = list[17];     //Penalty box front corner x>0 y>0

        static constexpr const sPonto *rings = &list[18];     //iterator for 10 ring points
    };

    class cSegmentos {
    	public:
    	/*
		Lista constante de segmentos de campo importante.

		Cada elemento terá 3 caracterísiticas:

		csegm = {
			inicio_c, 
			final_c,

			comprimento,
			angulo,

			tag_name
		}
    	*/

    	// Apenas definiremos os ângulos clássicos em radianos de uma vez.
    	static constexpr double c0deg  = 0, c36deg = 0.62831853071795865, c72deg = 1.2566370614359173;
        static constexpr double c90deg = 1.5707963267948966, c108deg = 1.8849555921538759, c144deg = 2.5132741228718346;

        static constexpr std::array<sSegmento,21> list = {{    
        	/*  Linhas Laterais Horizontais 
				
				[corner_mp] ●-----------------------● [corner_pp]  ← "side+" (y > 0)
                            |                       |
                            |                       |
                [corner_mm] ●-----------------------● [corner_pm]  ← "side-" (y < 0)
        	*/
            {&cPontos::corner_mm,  &cPontos::corner_pm,  cFieldLength,    c0deg  , "side-"  },  // Sideline y<0
            {&cPontos::corner_mp,  &cPontos::corner_pp,  cFieldLength,    c0deg  , "side+"  },  // Sideline y>0

            /* Linhas Laterais Verticais  

				[corner_mm] ●                     ● [corner_mp]
				            |                     |
				            |  ← "goal+" (x < 0)  |  ← "goal-" (x > 0)
				            |                     |
				[corner_pm] ●                     ● [corner_pp] 
            */
            {&cPontos::corner_mm,  &cPontos::corner_mp,  cFieldWidth,     c90deg , "goal-"  },  // Goal line x<0
            {&cPontos::corner_pm,  &cPontos::corner_pp,  cFieldWidth,     c90deg , "goal+"  },  // Goal line x>0

            /*  Linha Medial Vertical

				[halfway_p] ●                                                                          )
				            |          
				       ...  |  ...
				            |
				[halfway_m] ●

            */
            {&cPontos::halfway_m,  &cPontos::halfway_p,  cFieldWidth,     c90deg , "halfway"},  // Halfway line 

            /*  Área de Penâlti
            	Representação Deitada

            	         poste--   GOL    poste-+
				            |               |
				            |               |
				    boxgoal_mm --------- boxgoal_mp   (linha do gol)
				         |                   |
				         |                   |
				      box_mm ------------- box_mp   (frente da área)
            */
            {&cPontos::boxgoal_mm, &cPontos::box_mm,     cPenaltyLength,  c0deg  , "box--"  },  // Penalty box sideline x<0 y<0
            {&cPontos::boxgoal_mp, &cPontos::box_mp,     cPenaltyLength,  c0deg  , "box-+"  },  // Penalty box sideline x<0 y>0
            {&cPontos::box_pm,     &cPontos::boxgoal_pm, cPenaltyLength,  c0deg  , "box+-"  },  // Penalty box sideline x>0 y<0
            {&cPontos::box_pp,     &cPontos::boxgoal_pp, cPenaltyLength,  c0deg  , "box++"  },  // Penalty box sideline x>0 y>0
            {&cPontos::box_mm,     &cPontos::box_mp,     cPenaltyWidth,   c90deg , "box-"   },  // Penalty box front line x<0
            {&cPontos::box_pm,     &cPontos::box_pp,     cPenaltyWidth,   c90deg , "box+"   },  // Penalty box front line x>0
            
            /* Representãção dos Lados do Polígono definido pelos pontos
			
			ATENTE-SE ÀS DENOMINAÇÕES DE SENTIDO, afinal fizemos de tudo para que os ângulos ficassem positivos.
            */
            {&cPontos::rings[0],   &cPontos::rings[1],   cRingLineLength, c108deg, "rL0"    },  // Ring line 0   -> 36 
            {&cPontos::rings[1],   &cPontos::rings[2],   cRingLineLength, c144deg, "rL1"    },  // Ring line 36  -> 72 
            {&cPontos::rings[3],   &cPontos::rings[2],   cRingLineLength, c0deg  , "rL2"    },  // Ring line 72  <- 108
            {&cPontos::rings[4],   &cPontos::rings[3],   cRingLineLength, c36deg , "rL3"    },  // Ring line 108 <- 144
            {&cPontos::rings[5],   &cPontos::rings[4],   cRingLineLength, c72deg , "rL4"    },  // Ring line 144 <- 180
            {&cPontos::rings[6],   &cPontos::rings[5],   cRingLineLength, c108deg, "rL5"    },  // Ring line 180 <- 216
            {&cPontos::rings[7],   &cPontos::rings[6],   cRingLineLength, c144deg, "rL6"    },  // Ring line 216 <- 252
            {&cPontos::rings[7],   &cPontos::rings[8],   cRingLineLength, c0deg  , "rL7"    },  // Ring line 252 -> 288
            {&cPontos::rings[8],   &cPontos::rings[9],   cRingLineLength, c36deg , "rL8"    },  // Ring line 288 -> 324
            {&cPontos::rings[9],   &cPontos::rings[0],   cRingLineLength, c72deg , "rL9"    }   // Ring line 324 -> 0
        }};

        static constexpr const sSegmento &side_m = list[0]; // Sideline y<0
        static constexpr const sSegmento &side_p = list[1]; // Sideline y>0
        static constexpr const sSegmento &goal_m = list[2]; // Goal line x<0
        static constexpr const sSegmento &goal_p = list[3]; // Goal line x>0

        static constexpr const sSegmento &halfway = list[4];//Halfway line

        static constexpr const sSegmento &box_mm = list[5]; // Penalty box sideline x<0 y<0
        static constexpr const sSegmento &box_mp = list[6]; // Penalty box sideline x<0 y>0
        static constexpr const sSegmento &box_pm = list[7]; // Penalty box sideline x>0 y<0
        static constexpr const sSegmento &box_pp = list[8]; // Penalty box sideline x>0 y>0

        static constexpr const sSegmento &box_m = list[9];  // Penalty box front line x<0
        static constexpr const sSegmento &box_p = list[10]; // Penalty box front line x>0

        static constexpr const sSegmento *rings = &list[11];//iterator for 10 ring lines
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Não mudei os nomes pois já estão bons.

    /*
	Pontos de referência visíveis: incluem cantos e postes do gol.
	*/
	vector<sMkr> list_landmarks;

	/*
	Cantos do campo visíveis.
	*/
	vector<sMkr> list_landmarks_corners;

	/*
	Postes do gol visíveis.
	*/
	vector<sMkr> list_landmarks_goalposts;

	/*
	Segmentos de linha identificados e visíveis.
	A ordem dos pontos inicial e final é a mesma da linha correspondente no campo.
	*/
	vector<sSegmMkr> list_known_segments;

	/*
	Marcadores visíveis (extremidades de linhas + marcadores fixos) com referência ao ponto real do campo.
	*/
	vector<sMkr> list_known_markers;

	/*
	Extremidades visíveis de segmentos de linha cuja correspondência no campo ainda é desconhecida.
	Cada marcador mantém referência ao segmento de linha correspondente.
	Nota: marcadores_identificados + marcadores_desconhecidos excluem pontos de linhas completamente desconhecidas.
	*/
	vector<sMkr> list_unknown_markers;

	/*
	Marcadores no solo visíveis, incluindo extremidades de linhas, pontos de contato dos pés e bandeiras de canto.
	A posição absoluta armazenada é sempre (0,0,0).
	*/
	vector<sMkr> list_ground_markers;

	/*
	Número de marcadores no solo visíveis que não são colineares.
	Nota: colinearidade entre linhas não ocorre, mas entre pés e linhas pode ocorrer, embora improvável.
	*/
	int marcadores_nao_colineares;

	/*
	Versão ponderada de marcadores_de_solo.
	Pontos mais próximos são repetidos mais vezes, proporcionalmente à sua distância.
	*/
	vector<sMkr> list_weighted_ground_markers;

	/*
	Pontos de contato dos pés com o solo.
	*/
	vector<sMkr> list_feet_contact_points;

	/*
	Segmentos de linha visíveis, ainda que não tenham sido identificados.
	*/
	vector<Linha6D> list_segments;

	class gMkrs {
		/*
		Descrição:
		    Armazena uma lista fixa e redundante contendo os 8 marcadores de referência (landmarks) do campo
		    em coordenadas cartesianas relativas, organizados por posição e não por nome.

		Detalhes:
		    - Diferente da lista de marcadores presente em `world.landmarks`, esta lista é ordenada com base na posição no campo,
		      o que é essencial para manter consistência quando os times trocam de lado.
		    - Todos os marcadores são definidos como constantes de leitura pública para fácil acesso direto ao elemento desejado.
		    - O uso de variáveis estáticas garante alta performance no acesso sem duplicação de memória.
		*/
        friend class RobovizField;

        private:
            static sFixedMkr list[8];
            static constexpr sFixedMkr &_corner_mm = list[0];
            static constexpr sFixedMkr &_corner_mp = list[1];
            static constexpr sFixedMkr &_corner_pm = list[2];
            static constexpr sFixedMkr &_corner_pp = list[3];
            static constexpr sFixedMkr &_goal_mm   = list[4];
            static constexpr sFixedMkr &_goal_mp   = list[5];
            static constexpr sFixedMkr &_goal_pm   = list[6];
            static constexpr sFixedMkr &_goal_pp   = list[7];
        public:
            static constexpr const sFixedMkr &corner_mm = list[0];
            static constexpr const sFixedMkr &corner_mp = list[1];
            static constexpr const sFixedMkr &corner_pm = list[2];
            static constexpr const sFixedMkr &corner_pp = list[3];
            static constexpr const sFixedMkr &goal_mm   = list[4];
            static constexpr const sFixedMkr &goal_mp   = list[5];
            static constexpr const sFixedMkr &goal_pm   = list[6];
            static constexpr const sFixedMkr &goal_pp   = list[7];
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
	Atualiza os marcadores com base nos marcos e segmentos de linha já detectados.

	Esta função processa as informações visuais disponíveis no momento, como linhas e landmarks,
	para estimar posições relativas ou absolutas de pontos visíveis no campo.
	*/
	void 
	atualizar_marcadores();

	/*
	Atualiza os marcadores visíveis utilizando uma matriz de transformação do agente para o campo.

	A matriz de transformação define a posição e orientação do agente, permitindo atualizar os marcadores 
	em campo com base nessa referência.
	*/
	void 
	atualizar_marcadores_por_transformacao( const Matriz4D& Head_to_Field );

	/*
	Atualiza a posição absoluta de marcadores ainda não identificados, com base na matriz de transformação
	e nas linhas visíveis.

	É utilizada quando o agente observa pontos de linha sem saber sua correspondência no campo, mas consegue
	inferir sua posição com base em geometria relativa.
	*/
	void 
	atualizar_marcadores_desconhecidos_por_transformacao( const Matriz4D& Head_to_Field );

	void 
	ilustrador( const Matriz4D& Head_to_Field, bool lado_direito ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static float 
    calcular_dist_segm_para_pt2D_c(
    	const sSegmento& segm_de_linha,
    	const Vetor2D& ponto_cart_2d
    ){
    	/*
		Descrição:
			Mesmo que anterior, mas considerando agora que além do segmento,
			o ponto também está em plano z = 0.
    	*/

    	const Vetor2D inicio(
    		segm_de_linha.pt[0]->svet.x,
    		segm_de_linha.pt[0]->svet.y
    	);

    	const Vetor2D final(
    		segm_de_linha.pt[1]->svet.x,
    		segm_de_linha.pt[1]->svet.y
    	);

    	Vetor2D w1(ponto_cart_2d - inicio);

    	if(
    		w1.InnerProduct(final - inicio) <= 0
    	){

    		return w1.obter_modulo();
    	}

    	Vetor2D w2(ponto_cart_2d - final);

    	if(
    		w2.InnerProduct(final - inicio) >= 0
    	){

    		return w2.obter_modulo();
    	}

    	return fabsf(
    		(final - inicio).CrossProduct(w1) / segm_de_linha.comprimento 
    	);
    }

    // Não vou modificar nada destas pq são muito simples e rápidas.

    /*
     * Normalize angle between 2 lines
     * @return angle between 0 and 90 deg
     */
    static inline float normalize_line_angle_deg(float deg){
        return 90.f-fabsf(fmodf(fabsf(deg), 180.f) - 90.f);
    }

    /*
     * Normalize angle between 2 lines
     * @return angle between 0 and pi/2 rad
     */
    static inline float normalize_line_angle_rad(float rad){
        return 1.57079633f-fabsf(fmod(fabsf(rad), 3.14159265f) - 1.57079633f);
    }

    /*
     * Normalize angle between 2 vectors
     * @return angle between 0 and 180 deg
     */
    static inline float normalize_vector_angle_deg(float deg){
        return 180.f-fabsf(fmodf(fabsf(deg), 360.f) - 180.f);
    }

    /*
     * Normalize angle between 2 vectors
     * @return angle between 0 and pi rad
     */
    static inline float normalize_vector_angle_rad(float rad){
        return 3.14159265f-fabsf(fmod(fabsf(rad), 6.28318531f) - 3.14159265f);
    }
};

#endif // ROBOVIZFIELD_H
