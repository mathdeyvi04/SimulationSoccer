/*
Manter créditos ao Miguel Abreu e à equipe FC Portugal
*/
#ifndef CAMPO_H
#define CAMPO_H 

#include "Geometria.h"
#include "Singular.h"
#include "Matriz.h"
#include "Linha.h"
#include <vector>  // Alocação Dinâmica
#include <array>   // Alocação Estática

#include <cstdio>

using namespace std;

class Campo{
	/*
	Classe responsável gerenciar e manutenir TODOS OS ELEMENTOS DO CAMPO.
	*/

	friend class Singular<Campo>;

private: 

	// Impediremos:
	Campo(){};

	void obter_marcadores_de_campo();


public:

	/*
	Dimensões de Campo

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

    static constexpr float stdev_distance      =            0.0965; //st. deviation of error ed  (distance error=d/100*ed)
    static constexpr float var_distance        =        0.00931225; //     variance of error ed  (distance error=d/100*ed)
    static constexpr float var_round_hundredth =      0.01*0.01/12; //variance of uniformly distributed random variable [-0.005,0.005]

	/*//////////////////////////////////////////////////////////////////////
	
	Precisamos construir structs que representarão partes importantes do 
	Campo.

	Todas as structs se referem ao campo, compreenda seus respectivos contextos.

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

	// Fazemos isso para evitar cálculos desnecessários durante execuções.


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
            /* Traves dos Gols */
            {-cHalfFieldLength,-cHalfGoalWidth, cGoalHeight,   "post--"}, {-cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post-+"}, /* Gol Esquerdo, x < 0 */
            { cHalfFieldLength,-cHalfGoalWidth, cGoalHeight,   "post+-"}, { cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post++"}, /* Gol  Direito, x > 0 */

			/* Cantos Extremos do Campo */
            {-cHalfFieldLength,-cHalfFieldWidth,          0, "corner--"}, {-cHalfFieldLength, cHalfFieldWidth,0, "corner-+"}, /* Cantos Esquerdos do Campo, x < 0 */
            { cHalfFieldLength,-cHalfFieldWidth,          0, "corner+-"}, { cHalfFieldLength, cHalfFieldWidth,0, "corner++"}, /* Cantos  Direitos do Campo, x > 0 */
            
            /* Retas Horizontais Limitantes */
            {                0,-cHalfFieldWidth,          0, "halfway-"}, /* Ponto Que Define Reta Superior Limitante do Campo */
            {                0, cHalfFieldWidth,          0, "halfway+"}, /* Ponto Que Define Reta Inferior Limitante do Campo */
            
			/* Áreas de Penalidade do Gol, as quais permitem penâlti, por exemplo */
            {-cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack--"}, {-cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack-+"}, // Penalty box goal line corner x<0
            { cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack+-"}, { cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack++"}, // Penalty box goal line corner x>0
            {-cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt--"}, {-cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt-+"}, // Penalty box front corner x<0
            { cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt+-"}, { cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt++"}, // Penalty box front corner x>0

            /* Pontos De Um Círculo de Raio 2, note as angulações específicas e em graus */
            {                  2,                   0, 0,   "r0"}, { 1.6180339887498948,   1.1755705045849463, 0,  "r36"}, // Ring 0/36 deg
            {0.61803398874989485,  1.9021130325903071, 0,  "r72"}, {-0.61803398874989485,  1.9021130325903071, 0, "r108"}, // Ring 72/108 deg
            {-1.6180339887498948,  1.1755705045849463, 0, "r144"}, {                  -2,                   0, 0, "r180"}, // Ring 144/180 deg
            {-1.6180339887498948, -1.1755705045849463, 0, "r216"}, {-0.61803398874989485, -1.9021130325903071, 0, "r252"}, // Ring 216/252 deg
            {0.61803398874989485, -1.9021130325903071, 0, "r288"}, {  1.6180339887498948, -1.1755705045849463, 0, "r324"}  // Ring 288/324 deg
        }};


        static constexpr const sFieldPoint &goal_mm = lista_de_pontos[0];   //Goalpost x<0 y<0
        static constexpr const sFieldPoint &goal_mp = lista_de_pontos[1];   //Goalpost x<0 y>0
        static constexpr const sFieldPoint &goal_pm = lista_de_pontos[2];   //Goalpost x>0 y<0
        static constexpr const sFieldPoint &goal_pp = lista_de_pontos[3];   //Goalpost x>0 y>0

        static constexpr const sFieldPoint &corner_mm = lista_de_pontos[4]; //Corner x<0 y<0
        static constexpr const sFieldPoint &corner_mp = lista_de_pontos[5]; //Corner x<0 y>0
        static constexpr const sFieldPoint &corner_pm = lista_de_pontos[6]; //Corner x>0 y<0
        static constexpr const sFieldPoint &corner_pp = lista_de_pontos[7]; //Corner x>0 y>0

        static constexpr const sFieldPoint &halfway_m = lista_de_pontos[8]; //Halfway line ending y<0
        static constexpr const sFieldPoint &halfway_p = lista_de_pontos[9]; //Halfway line ending y>0

        static constexpr const sFieldPoint &boxgoal_mm = lista_de_pontos[10]; //Penalty box goal line corner x<0 y<0
        static constexpr const sFieldPoint &boxgoal_mp = lista_de_pontos[11]; //Penalty box goal line corner x<0 y>0
        static constexpr const sFieldPoint &boxgoal_pm = lista_de_pontos[12]; //Penalty box goal line corner x>0 y<0
        static constexpr const sFieldPoint &boxgoal_pp = lista_de_pontos[13]; //Penalty box goal line corner x>0 y>0

        static constexpr const sFieldPoint &box_mm = lista_de_pontos[14]; //Penalty box front corner x<0 y<0
        static constexpr const sFieldPoint &box_mp = lista_de_pontos[15]; //Penalty box front corner x<0 y>0
        static constexpr const sFieldPoint &box_pm = lista_de_pontos[16]; //Penalty box front corner x>0 y<0
        static constexpr const sFieldPoint &box_pp = lista_de_pontos[17]; //Penalty box front corner x>0 y>0

        static constexpr const sFieldPoint *rings = &lista_de_pontos[18]; //iterator for 10 ring points
    };


    class cSegmentos {
    	public:
    	/*
		
    	*/



    };








};







#endif // CAMPO_H