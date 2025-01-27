#ifndef CA_FUNCTIONS_PASSIVE_H
#define CA_FUNCTIONS_PASSIVE_H

#include "CB_Functions_Ative.h"

typedef struct {
	/*
	For the argument's threads
	*/
	int team_indicator;
	int *simulation_indicator;
	double *delta_time;
	Display *display;
} Arg_Coach;

SDL_Rect for_draw_information[
	NUMBER_OF_BLOCKS_PER_PLAYER  // Players will be defined by 5 blocks.
];

pthread_t coachs[2];
pthread_barrier_t coachs_command_flow;
pthread_mutex_t access_blocker_to_set_color;
#define NUMBER_OF_COACHS 3  // Main Include

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

#endif // C_PLAYER_FUNCTIONS_PASSIVE_H

