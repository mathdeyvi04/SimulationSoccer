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

	Não vou fazer nenhuma alteração, pois:

	- A nomenclatura já está boa.
	- Os números são imutáveis.
	- Desnecessário qualquer explicação complementar.
	*/

    static constexpr double cFieldLength = 30.0, cFieldWidth = 20.0, cPenaltyLength = 1.8, cPenaltyWidth = 6.0;
	static constexpr double cGoalWidth   =  2.1, cGoalDepth  =  0.6, cGoalHeight    = 0.8;

	static constexpr double cHalfFieldLength  =  cFieldLength/2.0, cHalfFieldWidth    =    cFieldWidth/2.0;
	static constexpr double cHalfGoalWidth    =    cGoalWidth/2.0, cHalfPenaltyLength = cPenaltyLength/2.0;
	static constexpr double cHalfPenaltyWidth = cPenaltyWidth/2.0;

	static constexpr double cPenaltyBoxDistX = cHalfFieldLength - cPenaltyLength;
    static constexpr double cRingLineLength  =                1.2360679774997897;

    static constexpr float cHalfHorizontalFoV = 60;
    static constexpr float cHalfVerticalFoV   = 60;

    static constexpr float stdev_distance      =       0.0965; //st. deviation of error ed  (distance error=d/100*ed)
    static constexpr float var_distance        =   0.00931225; //     variance of error ed  (distance error=d/100*ed)
    static constexpr float var_round_hundredth = 0.01*0.01/12; //variance of uniformly distributed random variable [-0.005,0.005]

	/*//////////////////////////////////////////////////////////////////////
	
	Precisamos construir structs que representarão partes importantes do 
	Campo.

	Todas as structs se referem ao campo, compreenda seus respectivos contextos.

	Abaixo, uma conversão de nomes utilizada:

	Original  ------ Aqui    ----- Convenção 
	----------------------------------------
	sVector3d ------ Vetor3D ----- pt_vet
	sFieldPoint ---- sPonto  ----- spt
	sFieldSegment -- sSegmento --- segm
	sMarker -------- sMarcador --- smkr
	sSegment ------- sSegmMkr ---- segm_mkr
	sFixedMarker --- sFixedMkr --- fixed_mkr -> Inglês é bem mais bonito
	cFieldPoints --- cPontos ----- cpts
	
	*///////////////////////////////////////////////////////////////////////

	/*
	No original, é criado uma struct chamada sVector3d que possuem componentes em
	double e 4 operações, montar um vector3f a partir dela, setar a partir de outro
	sVector3d, setar a partir de vector3f e calcular distâncias.

	Acredito que seja feito isso para reduzir carga de memória usada por uma classe com
	tantos métodos. Entretanto, não será feito aqui.
	*/

	struct sPonto    {
		/*
		Responsável por representar pontos no campo.
		*/

		const Vetor3D pt_vet;

		/*
		Para identificarmos o ponto ou apresentá-lo no 
		Drawing do RoboViz pelo buffer, lembra?
		*/
		const char tag[10]; 
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
		Vetor3D pos_abs;


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

		sMarcador() : pos_abs     ( Vetor3D() ), 
					  pos_rel_cart( Vetor3D() ),
					  pos_rel_esf ( Vetor3D() )
					  {};

		/*
		Note que não faz sentido haver um construtor com 
		a posição em cartesiano de entrada se o simulador 
		nos dá apenas em esférica.
		*/
		sMarcador(
			const Vetor3D& pos_abs_,
			const Vetor3D& pos_rel_esf_
		) : pos_abs     ( pos_abs_                      ),
			pos_rel_esf ( pos_rel_esf_                  ),
			pos_rel_cart( pos_rel_esf_.para_cartesiano())
			{};


		sMarcador(
			const sPonto  *spt_,
			const Vetor3D& pos_rel_esf_
		) : pos_abs     ( (*spt).pt_vet  				),
			spt         ( spt_           				),
			pos_rel_esf ( pos_rel_esf_   				),
			pos_rel_cart( pos_rel_esf_.para_cartesiano())
			{};


		sMarcador(
			const Vetor3D& pos_abs_,
			const Vetor3D& pos_rel_esf_,
			const Vetor3D& pos_rel_cart_
		) : pos_abs     ( pos_abs_                     ),
			pos_rel_esf ( pos_rel_esf_                 ),
			pos_rel_cart( pos_rel_cart_                )
			{};


		sMarcador(
			const sSegmento *segm_,
			const Vetor3D& 	 pos_abs_,
			const Vetor3D& 	 pos_rel_esf_,
			const Vetor3D& 	 pos_rel_cart_
		) : segm        ( segm_                        ),
			pos_abs     ( pos_abs_                     ),
			pos_rel_esf ( pos_rel_esf_                 ),
			pos_rel_cart( pos_rel_cart_                )
			{};


		sMarcador(
			const sPonto  *spt_,
			const Vetor3D& pos_rel_esf_,
			const Vetor3D& pos_rel_cart_
		) : spt         ( spt_           ),
			pos_abs     ( (*spt_).pt_vet ),
			pos_rel_esf ( pos_rel_esf_   ),
			pos_rel_cart( pos_rel_cart_  )
			{};

	// Fazemos isso para evitar cálculos desnecessários durante execuções.
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


    class cPontos    {
    	public:
    	/*
		Precisamos definir quais serão os pontos importantes.
		Usaremos essa sub classe para eles.

		Lista constante de pontos.

		Convenção:

        * "PT1-.-PT2" midpoint between PT1 and PT2 (2D/3D)
        * "PT1-PT2" point between PT1 and PT2 (in 2D only)
    	*/

    		/*Gera erro pq sPonto não é literal.*/

    	static constexpr std::array<sPonto,28> list {{
            {-cHalfFieldLength,-cHalfGoalWidth, cGoalHeight, "post--"}, {-cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post-+"}, //Goalposts x<0
            { cHalfFieldLength,-cHalfGoalWidth, cGoalHeight, "post+-"}, { cHalfFieldLength, cHalfGoalWidth, cGoalHeight, "post++"}, //Goalposts x>0
            {-cHalfFieldLength,-cHalfFieldWidth,0, "corner--"}, {-cHalfFieldLength, cHalfFieldWidth,0, "corner-+"}, //Corners x<0
            { cHalfFieldLength,-cHalfFieldWidth,0, "corner+-"}, { cHalfFieldLength, cHalfFieldWidth,0, "corner++"}, //Corners x>0
            {0,-cHalfFieldWidth, 0, "halfway-"}, // Halfway line ending y<0
            {0, cHalfFieldWidth, 0, "halfway+"}, // Halfway line ending y>0
            {-cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack--"}, {-cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack-+"}, //Penalty box goal line corner x<0
            { cHalfFieldLength, -cHalfPenaltyWidth, 0, "boxBack+-"}, { cHalfFieldLength,  cHalfPenaltyWidth, 0, "boxBack++"}, //Penalty box goal line corner x>0
            {-cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt--"}, {-cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt-+"}, //Penalty box front corner x<0
            { cPenaltyBoxDistX, -cHalfPenaltyWidth, 0, "boxFrnt+-"}, { cPenaltyBoxDistX,  cHalfPenaltyWidth, 0, "boxFrnt++"}, //Penalty box front corner x>0
            {2, 0, 0, "r0"},                                       { 1.6180339887498948,   1.1755705045849463, 0, "r36" }, //(18,19) Ring 0/36 deg
            {0.61803398874989485,  1.9021130325903071, 0, "r72" }, {-0.61803398874989485,  1.9021130325903071, 0, "r108"}, //(20,21) Ring 72/108 deg
            {-1.6180339887498948,  1.1755705045849463, 0, "r144"}, {-2, 0, 0, "r180"},                                     //(22,23) Ring 144/180 deg
            {-1.6180339887498948, -1.1755705045849463, 0, "r216"}, {-0.61803398874989485, -1.9021130325903071, 0, "r252"}, //(24,25) Ring 216/252 deg
            {0.61803398874989485, -1.9021130325903071, 0, "r288"}, { 1.6180339887498948,  -1.1755705045849463, 0, "r324"}  //(26,27) Ring 288/324 deg
        }};







    };










};






































#endif // CAMPO_H