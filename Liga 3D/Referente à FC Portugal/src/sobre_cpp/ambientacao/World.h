/*
Créditos mantidos: Miguel Abreu, um gênio, (m.abreu@fe.up.pt)
*/
#ifndef WORLD_H
#define WORLD_H

#include "Singular.h"
#include "AlgLin.h"
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

	O pé pode tocar outras coisas além do chão.
	*/
	bool se_pe_esta_tocando[2];
	/*
	Observe que é posição relativa, ou seja, devemos utilizar 
	um dado recebido do sistema, provavelmente coordenadas absolutas,
	e realizar transformações.
	*/
	Vetor3D pos_rel_do_ponto_de_contato[2]; 
	/* Posição relativa do ponto de contato do pé com chão */


	bool se_bola_esta_visivel;
	/* Posição rel da bola em coordenadas cartesianas. */
	Vetor3D pos_rel_da_bola_cart;  

	/*
	O cheat é indicador que este valor não foi obtido pelos sensores
	virtuais do robô, obtivemos por 'trapaça', direto do simulador.
	*/
	Vetor3D pos_abs_da_bola_cart_cheat;
	Vetor3D pos_abs_do_robo_cart_cheat;


	/*
	Atributos de Mundo:

	Linhas e cantos.

	São amplamente utilizados dentro de LocalizerV2 e RobovizField.

	Experimente ver o arquivo de debug_main.cc para verificar como funciona a criação dos mesmos.

	Tenha noção de que estas structs serão criadas e atribuídas dentro do python, por isso
	vc não às encontrará nestes arquivos.
	*/

	struct gMkr{ // ground markers
		bool se_esta_visivel;
		bool se_eh_canto;

		Vetor3D pos_abs;
		Vetor3D pos_rel_esf;  // Temos garantia que é esférica, basta ver oq é feito em RobovizField.cpp, dentro de atualizar_marcadores()
	};

	gMkr marcadores_de_chao[8];

	struct gLinha {
		Vetor3D inicio, final;

		gLinha(
			const Vetor3D& inicio,
			const Vetor3D& final
		) : inicio(inicio), final(final) {};
	};

	/*
	Apenas para organizarmos as linhas em coodernadas esféricas.
	*/
	vector<gLinha> linhas_esfericas;
};

/* 
Seria apenas para economizarmos nomenclatura,
mas não acho válido ocultar essa informação. 
*/
// typedef Singular<World> SWorld; /* Seria apenas para economizarmos nomenclatura */


#endif // WORLD_H