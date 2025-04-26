/*
Manter créditos ao Miguel Abreu e à equipe FC Portugal
*/
#ifndef CAMPO_H
#define CAMPO_H 

#include "Singular.h"  
#include "Geometria.h"
#include "Matriz.h"
#include "Linha.h"
#include <vector>  // Alocação Dinâmica
#include <array>   // Alocação Estática

using namespace std;

class RobovizField{
	/*
	Classe responsável gerenciar e manutenir TODOS OS ELEMENTOS DO CAMPO.
	*/

	friend class Singular<RobovizField>;

private: 

	// Impediremos:
	RobovizField(){};

	void obter_marcadores_de_campo();

public:

	/*///////////////////////////////////////////////////////////////////////////////
	
	● Definição de Dimensões de Campo

	Por que não escrever em define?

	- Acredito que à necessidade de relacionar fielmente ao Campo.
	- Necessidade do tempo de compilação, afinal, constexpr.
	- É basicamente a mesma coisa que o define, só que com vantagens.

	Não vou fazer nenhuma alteração, pois:

	- A nomenclatura já está boa.
	- Os números são imutáveis.
	- Desnecessário qualquer explicação complementar.

	*//////////////////////////////////////////////////////////////////////////////

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

	/*//////////////////////////////////////////////////////////////////////
	
	● Structs Básicas de Representação de Campo

	Abaixo, uma conversão de nomes utilizada:

	Original  ------ Aqui    ----- Convenção 
	----------------------------------------
	sVector3d ------ sVetor3D ---- svet
	sFieldPoint ---- sPonto  ----- spt
	sFieldSegment -- sSegmento --- segm
	sMarker -------- sMarcador --- smkr
	sSegment ------- sSegmMkr ---- segm_mkr
	sFixedMarker --- sFixedMkr --- fixed_mkr -> Inglês é bem mais bonito
	cFieldPoints --- cPontos ----- cpts
	cFieldSegments - cSegmentos -- csegm
	list_8_landmarks gMkrs ------- * 
	
	*///////////////////////////////////////////////////////////////////////

	struct sVetor3D  {
		/*
		Struct para representar a classe vetorial Vetor3D.
		
		Não podíamos apenas assumir que é apenas Vetor3D devido à necessidade
		de escrevermos static constexpr, isso obriga que tenhamos apenas
		tipos literais nas estruturas.
		*/

		double x, y, z;

		Vetor3D obter_vetor() const {

			return Vetor3D(x, y, z);
		}


		void setar(
			const sVetor3D& svet
		){

			x = svet.x;
			y = svet.y;
			z = svet.z;
		}


		void setar(
			const Vetor3D& vetor 
		){
			x = vetor.x;
			y = vetor.y;
			z = vetor.z;
		}
	};


	struct sPonto    {
		/*
		Responsável por representar pontos no campo.
		*/

		const sVetor3D svet;

		/*
		Para identificarmos o ponto ou apresentá-lo no 
		Drawing do RoboViz pelo buffer, lembra?
		*/
		const char tag[10] = {0}; 

		Vetor3D obter_vetor() const {

			return svet.obter_vetor();
		}
	};


	struct sSegmento {
		/*
		Responsável por representar segmentos de linha do campo.
		*/

		/*
		A sintaxe mais cachorra que você verá.
		
		Trata-se de um array de 2 ponteiros constantes para
		objetos sPonto constantes.

		Não se pode alterar dados para os quais os ponteiros apontam.
		Não se pode alterar o ponteiro para outro endereço.
		*/
		const sPonto * const spt[2];

		const double comprimento;

		const double ang;
		/*
		Sempre positivo, em [0, 180°] e correspondente ao vetor final_c - inicio_c.
		*/

		const char tag[8];
	};


	struct sMarcador {
		/*
		Determinados pontos e segmentos são mais intrínsecamente
		importantes que outros. É para isso servirá esta struct, 
		para marcar quais pontos e quais segmentos são importantes.
		*/


		/* Posição Absoluta Estimada pelo Robô */
		sVetor3D spos_abs;


		/*
		Ponteiro para ponto correspondente no campo.
		*/
		const sPonto *spt  = nullptr;

		/*
		Ponteiro para segmento correspondente no campo.
		*/
		const sSegmento *segm = nullptr;

		Vetor3D pos_rel_cart;
		Vetor3D pos_rel_esf;

		/*/////////////////////////////////
		Diversos Construtores Mínimos 
		*//////////////////////////////////

		sMarcador() : spos_abs     ({0, 0, 0}), 
					  pos_rel_cart ( 0, 0, 0 ),
					  pos_rel_esf  ( 0, 0, 0 )
					  {};

		/*
		Note que não faz sentido haver um construtor com 
		a posição em cartesiano de entrada se o simulador 
		nos dá apenas em esférica.
		*/
		sMarcador(
			const sVetor3D& spos_abs_,
			const  Vetor3D& pos_rel_esf_
		) : spos_abs     ( spos_abs_                     ),
			pos_rel_esf  ( pos_rel_esf_                  ),
			pos_rel_cart ( pos_rel_esf_.para_cartesiano())
			{};


		sMarcador(
			const sPonto  *spt_,
			const Vetor3D& pos_rel_esf_
		) : spos_abs     ( (*spt_).svet 				 ),
			spt          ( spt_           				 ),
			pos_rel_esf  ( pos_rel_esf_   				 ),
			pos_rel_cart ( pos_rel_esf_.para_cartesiano())
			{};


		sMarcador(
			const Vetor3D& pos_abs_,
			const Vetor3D& pos_rel_esf_
		) : spos_abs    (sVetor3D({pos_abs_.x, pos_abs_.y, pos_abs_.z}) ),
			pos_rel_esf (pos_rel_esf_                   			    ),
			pos_rel_cart(pos_rel_esf_.para_cartesiano()				    )
			{};



		sMarcador(
			const sVetor3D& spos_abs_,
			const Vetor3D&  pos_rel_esf_,
			const Vetor3D&  pos_rel_cart_
		) : spos_abs     ( spos_abs_                   ),
			pos_rel_esf ( pos_rel_esf_                 ),
			pos_rel_cart( pos_rel_cart_                )
			{};


		sMarcador(
			const sSegmento *segm_,
			const sVetor3D&  spos_abs_,
			const Vetor3D& 	 pos_rel_esf_,
			const Vetor3D& 	 pos_rel_cart_
		) : segm        ( segm_                        ),
			spos_abs    ( spos_abs_                     ),
			pos_rel_esf ( pos_rel_esf_                 ),
			pos_rel_cart( pos_rel_cart_                )
			{};


		sMarcador(
			const sPonto  *spt_,
			const Vetor3D& pos_rel_esf_,
			const Vetor3D& pos_rel_cart_
		) : spt         ( spt_           ),
			spos_abs    ( (*spt_).svet  ),
			pos_rel_esf ( pos_rel_esf_   ),
			pos_rel_cart( pos_rel_cart_  )
			{};

	};


	struct sSegmMkr  {
		/*
		Representaremos um segmento de pontos marcadores.
		*/

		sMarcador pontos_marcadores[2];  // 0 -> inicio, 1 -> fim

		float comprimento;

		const sSegmento *segm;

		sSegmMkr(
			const sMarcador& inicio,
			const sMarcador& final,
			float comprimento_,
			const sSegmento *segm_
		) : pontos_marcadores{ inicio, final }, // Nunca havia visto essa sintaxe, simplemente lindo.
			comprimento      ( comprimento_  ),
			segm             ( segm_         )
			{};
	};


	struct sFixedMkr {
		/*

		*/

		bool se_esta_visivel;

		Vetor3D pos_rel_esf;
		Vetor3D pos_rel_cart;

		sFixedMkr() : pos_rel_esf(Vetor3D()),
					  pos_rel_cart(Vetor3D()),
					  se_esta_visivel(false)
					  {};

		void setar_pos_rel(
			Vetor3D pos_rel_esf_
		){
			pos_rel_esf  = pos_rel_esf_;
			pos_rel_cart = pos_rel_esf_.para_cartesiano();
		}
	};


	/*//////////////////////////////////////////////////////////////////////
	
	● Classes de Representação Coletiva

	*///////////////////////////////////////////////////////////////////////


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

		static constexpr array<sPonto,28> lista_de_pontos = {{
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
        static constexpr const sPonto &goal_mm = lista_de_pontos[0];     //Goalpost x<0 y<0
        static constexpr const sPonto &goal_mp = lista_de_pontos[1];     //Goalpost x<0 y>0
        static constexpr const sPonto &goal_pm = lista_de_pontos[2];     //Goalpost x>0 y<0
        static constexpr const sPonto &goal_pp = lista_de_pontos[3];     //Goalpost x>0 y>0

        static constexpr const sPonto &corner_mm = lista_de_pontos[4];   //Corner x<0 y<0
        static constexpr const sPonto &corner_mp = lista_de_pontos[5];   //Corner x<0 y>0
        static constexpr const sPonto &corner_pm = lista_de_pontos[6];   //Corner x>0 y<0
        static constexpr const sPonto &corner_pp = lista_de_pontos[7];   //Corner x>0 y>0

        static constexpr const sPonto &halfway_m = lista_de_pontos[8];   //Halfway line ending y<0
        static constexpr const sPonto &halfway_p = lista_de_pontos[9];   //Halfway line ending y>0

        static constexpr const sPonto &boxgoal_mm = lista_de_pontos[10]; //Penalty box goal line corner x<0 y<0
        static constexpr const sPonto &boxgoal_mp = lista_de_pontos[11]; //Penalty box goal line corner x<0 y>0
        static constexpr const sPonto &boxgoal_pm = lista_de_pontos[12]; //Penalty box goal line corner x>0 y<0
        static constexpr const sPonto &boxgoal_pp = lista_de_pontos[13]; //Penalty box goal line corner x>0 y>0
        static constexpr const sPonto &box_mm = lista_de_pontos[14];     //Penalty box front corner x<0 y<0
        static constexpr const sPonto &box_mp = lista_de_pontos[15];     //Penalty box front corner x<0 y>0
        static constexpr const sPonto &box_pm = lista_de_pontos[16];     //Penalty box front corner x>0 y<0
        static constexpr const sPonto &box_pp = lista_de_pontos[17];     //Penalty box front corner x>0 y>0

        static constexpr const sPonto *rings = &lista_de_pontos[18];     //iterator for 10 ring points
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

        static constexpr const sSegmento &halfway = list[4]; //Halfway line

        static constexpr const sSegmento &box_mm = list[5]; // Penalty box sideline x<0 y<0
        static constexpr const sSegmento &box_mp = list[6]; // Penalty box sideline x<0 y>0
        static constexpr const sSegmento &box_pm = list[7]; // Penalty box sideline x>0 y<0
        static constexpr const sSegmento &box_pp = list[8]; // Penalty box sideline x>0 y>0

        static constexpr const sSegmento &box_m = list[9];  // Penalty box front line x<0
        static constexpr const sSegmento &box_p = list[10]; // Penalty box front line x>0

        static constexpr const sSegmento *rings = &list[11]; //iterator for 10 ring lines
    };


    /*//////////////////////////////////////////////////////////////////////
	
	● Coleções de Elementos Vísiveis Em Um Instante t.

	Pq estamos usamos vector ao invés de array?

	- vector é uma lista dinâmica, enquanto array é estática.

	*///////////////////////////////////////////////////////////////////////
    

    /*
	Contém todos os marcos fixos visíveis do campo,
	como as extremidades (corners) e as traves (goalposts).

	Esses marcos são estáticos e sempre ocupam posições
	conhecidas do campo, então quando são identificados,
	ajudam diretamente na estimativa de posição e orientação.
    */
    vector<sMarcador> lista_de_corners_e_goalposts;
    vector<sMarcador> 			lista_de_corners; 
    vector<sMarcador> 		  lista_de_goalposts; 
    
    /*
	Segmentos de linha do campo identificados.

	Guarda apenas os que foram claramente reconhecidos
	como pertencentes a partes específicas do campo,
	o que permite associá-los com seu nome e função real.
    */
    vector<sSegmMkr> lista_de_segmentos_identificados;
    
    /*
	Contém os pontos visíveis que estão em segmentos
	identificados ou são marcos reconhecidos.

	Esses marcadores podem ser, por exemplo, as
	extremidades de linhas que já foram mapeadas
	corretamente ou as posições das quinas e traves.
    */
    vector<sMarcador> lista_de_marcadores_identificados;

    /*
	Armazena os pontos detectados nas imagens que pertencem
	a segmentos de linha ainda não identificados.

	Embora sejam visíveis, o sistema ainda não sabe exatamente a 
	qual linha ou local do campo eles pertencem, mas sabe
	que fazem parte de uma linha existente.
    */
    vector<sMarcador> lista_de_marcadores_desconhecidos;

    /*
	Pontos visíveis que ficam sobre o chão, como extremidades
	de linhas visíveis, pontos de contato com o pé do jogador
	e bandeiras de canto.
    */
    vector<sMarcador> lista_de_marcadores_de_chao;

    /*
	Raepresenta quantos marcadores do chão não estão em linha
	reta entre si.

	A questão é quanto desses marcadores formam uma geometria que
	permite localizar a posição de forma confiável, sem ambiguidade.
    */
    int quantidade_de_marcadores_de_chao_nao_colineares;

    /*
	Parecida com list_ground_markers, mas com repetições. Pontos que
	estão mais próximos da câmera ou sensor são inseridos mais vezes.

	Isso serve para dar mais peso a informações próximas, que costumam
	ser mais precisas e confiáveis.
    */
    vector<sMarcador> lista_de_marcadores_de_chao_pesados;

    /* Autoexplicativo */
    vector<sMarcador> lista_de_pontos_de_contato_do_pe;

    /*
	Lista de todos os segmentos de linha vísiveis, inclusive os
	não identificados.
    */
    vector<Linha> lista_de_todos_os_segmentos;


    class gMkrs{  // grounds markers -> marcadores de chão
    	/*
		Classe representante dos 8 marcadores de chão totais,
		para acelerar verificações, pois diferente de World.marcadores_de_chao
    	está organizado por posição e não nome.

    	*/
        friend class RobovizField;  // Para acessarmos elementos privados desta  

        private:
            
            static sFixedMkr marcadores_de_chao[8];
            //static array<sFixedMkr, 8> marcadores_de_chao;

            /* Observe que elementos dentro do privado são alteráveis,
            enquanto os públicos são constantes.*/

            static constexpr sFixedMkr &_corner_mm = marcadores_de_chao[0];
            static constexpr sFixedMkr &_corner_mp = marcadores_de_chao[1];
            static constexpr sFixedMkr &_corner_pm = marcadores_de_chao[2];
            static constexpr sFixedMkr &_corner_pp = marcadores_de_chao[3];
            static constexpr sFixedMkr &_goal_mm   = marcadores_de_chao[4];
            static constexpr sFixedMkr &_goal_mp   = marcadores_de_chao[5];
            static constexpr sFixedMkr &_goal_pm   = marcadores_de_chao[6];
            static constexpr sFixedMkr &_goal_pp   = marcadores_de_chao[7];
        public:
            static constexpr const sFixedMkr &corner_mm = marcadores_de_chao[0];
            static constexpr const sFixedMkr &corner_mp = marcadores_de_chao[1];
            static constexpr const sFixedMkr &corner_pm = marcadores_de_chao[2];
            static constexpr const sFixedMkr &corner_pp = marcadores_de_chao[3];
            static constexpr const sFixedMkr &goal_mm   = marcadores_de_chao[4];
            static constexpr const sFixedMkr &goal_mp   = marcadores_de_chao[5];
            static constexpr const sFixedMkr &goal_pm   = marcadores_de_chao[6];
            static constexpr const sFixedMkr &goal_pp   = marcadores_de_chao[7];
    };


    sSegmMkr* obter_segmento_conhecido(
    	const sSegmento& id_ptr
    ){
    	/*
		Obtermos o elemento no vetor de segmentos identificados.
    	*/

    	for(
    		sSegmMkr& ssegm_mkr : lista_de_segmentos_identificados
    	){

    		if(
    			ssegm_mkr.segm == &id_ptr
    		){

    			return &ssegm_mkr;
    		}
    	}
    }

    /*/////////////////////////////////////////////////////////
	

	Seguiremos apenas com a prototipação de métodos, pois faz-se
	necessário o uso de um arquivo .cpp para:

	- Variáveis static não constexpr não podem ser inicializadas em .h

    *//////////////////////////////////////////////////////////


    /*////////////////////////////////////////////////////////

    Métodos de Apresentação
	
    */////////////////////////////////////////////////////////


    void atualizar_marcadores();


    void atualizar_marcadores_a_partir_da_transformacao(
    	const Matriz& matriz_de_transformacao
    );


    void atualizar_marcadores_desconhecidos_a_partir_da_transformacao(
    	const Matriz& matriz_de_transformacao
    );

    /*
	Desenhar todas as linhas, marcadores, posição do robô e a bola que estão vísiveis.
    */
    void desenhar_visiveis(
    	const Matriz& Head_to_Field,  // -> Transformação de coordenadas da cabeça para o campo.
    	bool  se_eh_lado_direito      // -> Times trocam de lado após intervalo, por isso devemos saber informações como esta.
    ) const;

    /*
	Forçará o Roboviz a desenhar como se estivesse do outro lado do campo.
    */
    void desenhar_visiveis_trocados(
    	const Matriz& Head_to_Field
    ) const;





































};

#endif // CAMPO_H