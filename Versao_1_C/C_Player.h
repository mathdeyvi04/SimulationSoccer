#ifndef C_PLAYER_H
#define C_PLAYER_H

#include "F_FuncoesBasicas.h"

typedef struct {
	// Visual na Tela
	double radius;
	
	// Cor na Tela.
	// xxxxxxxxxxxx
	
	// Cinematic Attributes
	int mass;
	double pos[N_DIMENSIONS];
	double vel[N_DIMENSIONS];
	double acel[N_DIMENSIONS];
} Player;


Player players[
	/*
	We will use the index as the type of the player.
	
	This number(index) is responsible for what kind of player we have.
	For example, 
	
		i = 0 -> Indicades the BALL;
		i = 1...x -> Jogadores, Goleiros, Reservas
		i = x + 1 -> Judge
	*/
	QUANTIDADE_DE_PLAYERS
];








#endif // C_PLAYER_H
