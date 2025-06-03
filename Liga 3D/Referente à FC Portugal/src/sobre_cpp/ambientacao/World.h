/*
Créditos:
	* AUTHOR:       Miguel Abreu (m.abreu@fe.up.pt)
    * DATE:         2021
*/
#ifndef WORLD_H
#define WORLD_H

#include "Singular.h"
#include "AlgLin.h"
#include <iostream>
#include <vector>

using namespace std;

class World {
	/*
	Descrição:
		A classe World representa o estado atual do mundo percebido pelo agente
		durante a simulação. Armazena informações sensoriais e estimadas, como:
			- contato dos pés com o solo
			- posição relativa da bola
			- posição absoluta (idealizada) do agente e da bola
			- marcadores de referência no campo (landmarks)
			- linhas detectadas em coordenadas esféricas

	Atributos:
		- foot_touch[2]:
		    Booleanos indicando se os pés esquerdo (0) e direito (1) estão tocando o solo.
		- foot_contact_rel_pos[2]:
		    Vetores 3D com as posições relativas dos pontos de contato dos pés.
		- ball_seen:
		    Indica se a bola está visível para o agente.
		- ball_rel_pos_cart:
		    Posição relativa da bola em coordenadas cartesianas.
		- ball_cheat_abs_cart_pos:
		    Posição absoluta idealizada da bola (usada como referência para debug).
		- my_cheat_abs_cart_pos:
		    Posição absoluta idealizada do agente (para fins de análise).

		- landmark[8]:
		    Vetor contponto_finalo até 8 marcadores visíveis no campo, com estrutura `sLMark`, que inclui:
		      - seen: se foi detectado
		      - isCorner: se representa um canto do campo
		      - pos: posição absoluta
		      - rel_pos: posição relativa

		- linhas_esfericas:
		    Vetor de linhas detectadas em coordenadas esféricas.
	*/

    friend class Singular<World>;

private:

    World(){};

public:

    // Variáveis dos pés: (0) esquerdo, (1) direito

    /*
    Indica se cada pé está em contato com o solo.
    */
    bool pe_em_contato[2]; 

    /*
    Posição relativa do ponto de contato de cada pé com o solo.
    Vetor no referencial do pé.
    */
    Vetor3D pos_rel_contato_pe[2]; 

    /*
    Indica se a bola foi detectada na percepção.
    */
    bool bola_detectada; 

    /*
    Posição relativa da bola em coordenadas cartesianas.
    */
    Vetor3D pos_rel_bola_cartesiana; 

    /*
    Posição absoluta (cheat) da bola em coordenadas cartesianas.
    Usada para depuração ou simulações em que a posição real é conhecida.
    */
    Vetor3D pos_abs_bola_cartesiana_cheat; 

    /*
    Posição absoluta (cheat) do agente em coordenadas cartesianas.
    Usada para depuração ou simulações em que a posição real é conhecida.
    */
    Vetor3D pos_abs_agente_cartesiana_cheat; 

    /*
    Representa um marco de referência no campo.
    */
    struct gMkr {
        /*
        Indica se o marco foi detectado.
        */
        bool detectado;
        /*
        Indica se o marco é um canto do campo.
        */
        bool eh_canto;
        /*
        Posição absoluta do marco.
        */
        Vetor3D pos_absoluta;
        /*
        Posição relativa do marco ao agente.
        Temos garantia que é esférica! Basta ver oq é feito em RobovizField.cpp
        */
        Vetor3D pos_relativa;
    };

    /*
    Vetor com os marcos de referência percebidos (máx 8).
    */
    gMkr landmark[8];

    /*
    Representa uma linha do campo, definida por dois pontos.
    */
    struct gLinha {

        Vetor3D ponto_inicial;

        Vetor3D ponto_final;

        gLinha(const Vetor3D& inicio, const Vetor3D& fim) : ponto_inicial(inicio), ponto_final(fim) {};
    };

    /*
    Vetor de linhas percebidas no campo (em coordenadas esfericas).
    */
    vector<gLinha> linhas_esfericas;
};

#endif // WORLD_H
