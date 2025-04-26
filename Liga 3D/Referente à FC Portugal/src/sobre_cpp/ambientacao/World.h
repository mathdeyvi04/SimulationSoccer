/*
Créditos mantidos: Miguel Abreu, um gênio, (m.abreu@fe.up.pt)
*/
#ifndef WORLD_H
#define WORLD_H

#include "Geometria.h"
#include "Singular.h"
#include "Matriz.h"
#include "Linha.h"
#include <vector>
#include <array>

using namespace std;

class World {
	/*
	Classe responsável por providenciar variáveis e structs
	essenciais para compreensão do mundo externo ao robô.
	*/
	
	friend class Singular<World>;

private:

	World() {};

public:

	/*
	Atributos de Pé, não ironicamente.

	0 -> Pé Esquerdo
	1 -> Pé Direito
	*/
	bool pe_esta_tocando_chao[2];
	/*
	Observe que é posição relativa, ou seja, devemos utilizar 
	um dado recebido do sistema, provavelmente coordenadas absolutas,
	e realizar transformações.
	*/
	Vetor3D pos_rel_do_ponto_de_contato_com_chao[2]; 
	/* Posição relativa do ponto de contato do pé com chão */


	bool se_bola_esta_na_vista;
	/* Posição rel da bola em coordenadas cartesianas. */
	Vetor3D pos_real_da_bola_cart;  

	/*
	O cheat é indicador que este valor não foi obtido pelos sensores
	virtuais do robô, obtivemos por 'trapaça', direto do simulador.
	*/
	Vetor3D pos_abs_da_bola_cart_cheat;
	Vetor3D pos_abs_do_robo_cart_cheat;


	/*
	Atributos de Mundo:

	Linhas e cantos.
	*/

	struct gMkr{ // ground markers
		bool se_esta_na_vista;
		bool se_eh_canto;

		Vetor3D pos_abs;
		Vetor3D pos_rel;
	};

	gMkr marcadores_de_chao[8];


	struct sLine {
		Vetor3D inicio, final;

		sLine(
			const Vetor3D& inicio,
			const Vetor3D& final
		) : inicio(inicio), final(final) {};
	};

	/*
	Apenas para organizarmos as linhas em coodernadas esféricas.
	*/
	vector<sLine> linhas_esfericas;
};

/* 
Seria apenas para economizarmos nomenclatura,
mas não acho válido ocultar essa informação. 
*/
// typedef Singular<World> SWorld; /* Seria apenas para economizarmos nomenclatura */


#endif // WORLD_H