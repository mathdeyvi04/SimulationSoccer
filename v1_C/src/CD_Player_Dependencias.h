#ifndef CD_PLAYER_DEPENDENCIAS_H
#define CD_PLAYER_DEPENDENCIAS_H

#include "D_Dependencias.h"

///////////////////////////////////////////////////////////////
/// Variables and Structs
///////////////////////////////////////////////////////////////

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

Player playables[
	NUMBER_OF_PLAYERS
];

///////////////////////////////////////////////////////////////
/// About Ball and Interations
///////////////////////////////////////////////////////////////

SDL_Texture *texture_ball;
#define RADIUS_BALL 20
#define MASS_BALL 1 // Only the ball has this mass.
#define MIN_POW_DIST_2_FOR_CATCH 400
int ball_dominator[1];  // Player's index will hold the ball.
int kick_charge[1];
#define COEF_KICK_CHARGE 2
int goals[2];


///////////////////////////////////////////////////////////////
/// About PLayer
///////////////////////////////////////////////////////////////

#define SIDE_PLAYER 16  // Should be a multiple of 4.
#define MASS_PLAYER 10
#define NUMBER_OF_BLOCKS_PER_PLAYER 5
#define MINOR_SIZE_SQUARE SIDE_PLAYER / 4
#define COEF_DRAG 0.25

#define NUMBER_OF_ROWS (BOTTOMRIGHT_X - TOPLEFT_X) / (7 * MINOR_SIZE_SQUARE)
#define NUMBER_OF_COLUMNS (BOTTOMRIGHT_Y - TOPLEFT_Y) / (7 * MINOR_SIZE_SQUARE)

#define NUMBER_OF_PLAYERS_IN_EACH_TEAM (NUMBER_OF_PLAYERS - 1) / 2


#endif
