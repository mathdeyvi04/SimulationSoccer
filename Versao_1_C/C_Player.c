#include "C_Player.h"

//////////////////////////////////////////////////////////////////////////////
//// Variables for Testing
//////////////////////////////////////////////////////////////////////////////

int COLOR[3][3] = {
	{255, 255, 255},
	{255, 0, 0},
	{0, 0, 255}	
};

SDL_Rect for_draw_information[
	NUMBER_OF_BLOCKS_PER_PLAYER  // Players will be defined by 5 blocks.
];

//////////////////////////////////////////////////////////////////////////////
//// Funções 
//////////////////////////////////////////////////////////////////////////////


int
generate_players( 
	Player *list_of_playable,
	int *simulation_is_running
){
	/*
	Description:
		Function responsible for create the ball and the threads.
		
		These threads will be responsible for generate each team 
		and managing them.
	*/
	
	list_of_playable[0] = (Player) {
		.side = RADIUS_BALL, 
		.color = {COLOR[0][0], COLOR[0][1], COLOR[0][2]},
		.mass = MASS_BALL,
		
		// Initial position ball
		.pos = {MEDIUM_X, MEDIUM_Y},
		.vel = {0, 0},
		.acel = {0, 0}
	};
	
	for(
		int i = 0;
		i < 2;
		i++
	){
		
		Arg_Coach *arg = (Arg_Coach*) malloc(sizeof(Arg_Coach));
		(*arg).team_indicator = i + 1;
		(*arg).simulation_indicator = simulation_is_running;
			
		if(
			pthread_create(
				coachs,
				NULL,
				managing_team,
				arg  // Indicates the team's thread.
			) != 0
		){
			display_error(
				"\n(C) Error in create threads coachs."
			);
			
			*simulation_is_running = 0;
			break;
		}
		
		pthread_detach(
			coachs[i]
		);
		
		*simulation_is_running = 0;
	}

	return 1;
}


int
draw_a_player(
	Player playable,
	Display display
){
	/*
	Description:
		Function responsable for draw a player thinking about the index's player.
		
	Return:
		Returns 1 if ending sucessfuly.
	*/
	
	SDL_SetRenderDrawColor(
		display.renderer,
		playable.color[0],
		playable.color[1],
		playable.color[2],
		255
	);

	if(
		playable.mass == 1
	){
		SDL_Rect rect = {
			playable.pos[0] - playable.side / 2,
			playable.pos[1] - playable.side / 2,
			playable.side,
			playable.side
		};

		SDL_RenderFillRect(
			display.renderer,
			&rect
		);

		return 1;
	}
	
	for_draw_information[0] = (SDL_Rect) {
		playable.pos[0] - MINOR_SIZE_SQUARE / 2 - 3 * MINOR_SIZE_SQUARE,
		playable.pos[1] - MINOR_SIZE_SQUARE / 2 - 1 * MINOR_SIZE_SQUARE,
		MINOR_SIZE_SQUARE,
		3 * MINOR_SIZE_SQUARE
	};
	
	for_draw_information[1] = (SDL_Rect) {
		playable.pos[0] - MINOR_SIZE_SQUARE / 2 - 2 * MINOR_SIZE_SQUARE,
		playable.pos[1] - MINOR_SIZE_SQUARE / 2 - 2 * MINOR_SIZE_SQUARE,
		MINOR_SIZE_SQUARE,
		5 * MINOR_SIZE_SQUARE
	};
 
	for_draw_information[2] = (SDL_Rect) {
		playable.pos[0] - MINOR_SIZE_SQUARE / 2 - MINOR_SIZE_SQUARE,
		playable.pos[1] - MINOR_SIZE_SQUARE / 2 - 3 * MINOR_SIZE_SQUARE,
		3 * MINOR_SIZE_SQUARE,
		7 * MINOR_SIZE_SQUARE
	};
	
	for_draw_information[3] = (SDL_Rect) {
		playable.pos[0] - MINOR_SIZE_SQUARE / 2 + 2 * MINOR_SIZE_SQUARE,
		playable.pos[1] - MINOR_SIZE_SQUARE / 2 - 2 * MINOR_SIZE_SQUARE,
		MINOR_SIZE_SQUARE,
		5 * MINOR_SIZE_SQUARE
	};
	
	for_draw_information[4] = (SDL_Rect) {
		playable.pos[0] - MINOR_SIZE_SQUARE / 2 + 3 * MINOR_SIZE_SQUARE,
		playable.pos[1] - MINOR_SIZE_SQUARE / 2 - 1 * MINOR_SIZE_SQUARE,
		MINOR_SIZE_SQUARE,
		3 * MINOR_SIZE_SQUARE
	};
	
	for(
		int i = 0;
		i < NUMBER_OF_BLOCKS_PER_PLAYER;
		i++
	){
		SDL_RenderFillRect(
			display.renderer,
			for_draw_information + i
		);
	} 
	
	return 1;
}


int 
moviment(
	Player *playable,
	double delta_time
){
	/*
	Description:
		Function responsible for the moviment's playable.
	*/
	
	for(
		int i = 0;
		i < 2;
		i++
	){
		(*playable).pos[i] += (*playable).vel[i] * delta_time + 0.5 * (*playable).acel[i] * delta_time * delta_time;
		
		(*playable).vel[i] += (*playable).acel[i] * delta_time;
	}
	
	return 1;
}


int 
secure_player(
	Player *playable
){
	/*
	Description:
		Function responsible for ensuring that the player is kept within
		the boundaries of the field. 
		
	Return:
		0 if within field soccer.
		1 if not.
	*/
	
	if(
		(*playable).pos[0] <= TOPLEFT_X
	){
		(*playable).vel[0] = (double) (*playable).vel[0] * (-1);
		(*playable).pos[0] = TOPLEFT_X + 1;
		
		return 1;
	}
	
	if(
		(*playable).pos[0] >= BOTTOMRIGHT_X
	){
		(*playable).vel[0] = (double) (*playable).vel[0] * (-1);
		(*playable).pos[0] = BOTTOMRIGHT_X - 1;
		
		return 1;
	}
	
	if(
		(*playable).pos[1] <= TOPLEFT_Y
	){
		(*playable).vel[1] = (double) (*playable).vel[1] * (-1);
		(*playable).pos[1] = TOPLEFT_Y + 1;
		   
  		return 1;
	}
	
	if(
		(*playable).pos[1] >= BOTTOMRIGHT_Y
	){
		(*playable).vel[1] = (double) (*playable).vel[1] * (-1);
		(*playable).pos[1] = BOTTOMRIGHT_Y - 1;
		
		return 1;
	}
	
	return 0;
}


void*
managing_team(
	void *arg
){
	/*
	Description:
		Function responsible for be able to self execute with another in parallel.
		It will manage the entire team.
		
	Parameters:
		-> arg:
			Indicates the index's of the player's team.
	*/
	
	Arg_Coach coach_info = *(Arg_Coach*) arg;
	
	srand(
		time(NULL) + coach_info.team_indicator
	);
	
	// First, generate each team.
	for(
		int i = 0;
		(i / 2) < NUMBER_OF_PLAYERS_IN_EACH_TEAM;
		i = i + 2
	){
		playables[
			coach_info.team_indicator + i
		] = (Player) {
			.side = SIDE_PLAYER, 
			.color = {
				COLOR[coach_info.team_indicator][0],
				COLOR[coach_info.team_indicator][1],
				COLOR[coach_info.team_indicator][2]
			},
			.mass = MASS_PLAYER,

			// Initial position ball
			.pos = {
				rand() % (WIDTH_SCREEN - 100) + TOPLEFT_X,
				rand() % (HEIGHT_SCREEN - 100) + TOPLEFT_Y
			},
			.vel = {0, 0},
			.acel = {0, 0}
		};	
	}
	
	pthread_barrier_wait(
		/*
		To ensure all players are initialized before the simulation begins.
		*/
		&coachs_command_flow
	);
	
	printf("\nSimu = %d.", *(coach_info.simulation_indicator));
	
	free(arg);
	pthread_exit(0);
}








