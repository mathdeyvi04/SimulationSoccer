#ifndef C_JOGADOR_H
#define C_JOGADOR_H

#include "D_Bola.h"

typedef struct {
	// Visual na Tela
	double radius;
	
	
	/*
	This variable is basically a ID that specify
	what kind of player this Jogador is, as Goleiro,
	Juiz(?) e Titular ou Reserva(?).
	
	In theory, each type of player corresponds to a number.
	From this number, all will be diferent.
	
	Unfortunately, this is necessary only because do not exist
	class and heritage.
	*/
	int type;
	
	// Cinematic Attributes
	int mass;
	double pos[N_DIMENSIONS];
	double vel[N_DIMENSIONS];
	double acel[N_DIMENSIONS];
} Jogador;










#endif // C_JOGADOR_H
