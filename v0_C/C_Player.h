
#ifndef C_PLAYER_H
#define C_PLAYER_H

#include "D_FuncoesBasicas.h"

typedef struct {
	
	// Visual Attributes
	int side;
	int color[3];  // RGB
	
	// Cinematic Attributes
	int mass;
	double pos[N_DIMENSIONS];  // Will be the center
	double vel[N_DIMENSIONS];
	double acel[N_DIMENSIONS];
	
} Player;

typedef struct {
	/*
	For the argument's threads
	*/
	int team_indicator;
	int *simulation_indicator;
	double *delta_time;
	Display *display;
	
} Arg_Coach;

Player playables[
	NUMBER_OF_PLAYERS
];

pthread_t coachs[2];
pthread_barrier_t coachs_command_flow;
pthread_mutex_t access_blocker_to_set_color;
#define NUMBER_OF_COACHS 3  // Main Include

///////////////////////////////////////////////////////////////////////////////
/// Variáveis de Jogadores
///////////////////////////////////////////////////////////////////////////////

SDL_Texture *texture_ball;
#define RADIUS_BALL 20
#define MASS_BALL 1 // Only the ball has this mass.

#define SIDE_PLAYER 16  // Should be a multiple of 4.
#define MASS_PLAYER 10
#define NUMBER_OF_BLOCKS_PER_PLAYER 5
#define MINOR_SIZE_SQUARE SIDE_PLAYER / 4
#define COEF_DRAG 0.25

#define NUMBER_OF_PLAYERS_IN_EACH_TEAM (NUMBER_OF_PLAYERS - 1) / 2

///////////////////////////////////////////////////////////////////////////////
/// Protótipos de Funções em Ordem
///////////////////////////////////////////////////////////////////////////////

int
generate_players(
	Player*,
	int*,
	double*,
	Display*
);

int
draw_a_player(
	Player,
	Display
);

int
moviment(
	Player*,
	double
);

int
secure_player(
	Player*
);

void*
managing_team(
	void*
);


#endif // C_PLAYER_H

