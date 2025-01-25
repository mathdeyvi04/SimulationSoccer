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
//// Funções Base dos Jogadores
//////////////////////////////////////////////////////////////////////////////


int
generate_players( 
	Player *list_of_playable,
	int *simulation_is_running,
	double *delta_time,
	Display *display
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
	
	// Because setcolor sets a color for the entire window, a mutex was required.
	pthread_mutex_init(
		&access_blocker_to_set_color,
		NULL
	);
	
	// Initialize the variable.
	*ball_dominator = 0;
	*kick_charge = 0;
	
	for(
		int i = 0;
		i < 2;
		i++
	){
		
		Arg_Coach *arg = (Arg_Coach*) malloc(sizeof(Arg_Coach));
		(*arg).team_indicator = i + 1;
		(*arg).simulation_indicator = simulation_is_running;
		(*arg).delta_time = delta_time;
		(*arg).display = display;
			
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
	}

	return 1;
}


int
draw_a_player(
	Player playable,
	Display display
){
	/*efef
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
		
		SDL_RenderCopy(
			display.renderer,
			texture_ball,
			NULL,
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
		// We will add acceleration to prevent all players from moving infinitely.
		(*playable).acel[i] = - COEF_DRAG * (*playable).vel[i];
		
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
		
		Flow Of Thread
		
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
			.vel = {rand() % 100, rand() % 100},
			.acel = {0, 0}
		};	
	}
	
	
	while(
		*(coach_info.simulation_indicator)
	){
		pthread_barrier_wait(
			/*
			At first:
				To ensure all players are initialized before the simulation begins.
			
			Others:
				To synchronize.
			*/
			&coachs_command_flow
		);
		
		pthread_barrier_wait(
			/*
			Waits for input and update initiated by main.
			*/
			&coachs_command_flow
		);
		
		for(
			int i = 0;
			(i / 2) < NUMBER_OF_PLAYERS_IN_EACH_TEAM;
			i = i + 2
		){
			// Calculates each interation between players e the ball.
			
			// Calculates the moviment each player.
			secure_player(
				playables + coach_info.team_indicator + i
			);
			
			moviment(
				playables + coach_info.team_indicator + i,
				*(coach_info.delta_time)
			);
			
			// render each player.
			pthread_mutex_lock(
				&access_blocker_to_set_color
			);
			draw_a_player(
				playables[
					coach_info.team_indicator + i
				],
				*(coach_info.display)
			);
			pthread_mutex_unlock(
				&access_blocker_to_set_color
			);
		}
		
		// Updates the ball.
		pthread_barrier_wait(
			/*
			Frees main thread to mov and to render ball.
			*/
			&coachs_command_flow
		);
		
		// All three free.		
	}
	
	free(arg);
	pthread_exit(0);
}

//////////////////////////////////////////////////////////////////////////////
//// Funções de Ação
//////////////////////////////////////////////////////////////////////////////

int 
close_enough(
	Player *playable
){
	/*
	Description:
		Verify if the player is close enough to the ball.
		
		If sucess:
			Returns 1.
		
		else:
			Returns 0.
	*/
	
	double part_1 = ((*playable).pos[0] - playables[0].pos[0]) * ((*playable).pos[0] - playables[0].pos[0]);
	double part_2 = ((*playable).pos[1] - playables[0].pos[1]) * ((*playable).pos[1] - playables[0].pos[1]);
	
	if(
		(part_1 + part_2) < MIN_POW_DIST_2_FOR_CATCH
	){	
		return 1;
	}
	
	return 0;
}


int
kick_ball(
	int force
){
	/*
	Description:
		Once you have the last mouse position, kick the ball with force in its direction.
	*/
	
	// Releases the ball.
	*ball_dominator = 0;
	
	double verser_ball_to_mouse[2] = {
		last_position_mouse[0] - playables[0].pos[0],
		last_position_mouse[1] - playables[0].pos[1]
	};
	
	double module_direction = sqrt(
		get_module_squared(
			verser_ball_to_mouse
		)
	);
	
	verser_ball_to_mouse[0] /= module_direction;
	verser_ball_to_mouse[1] /= module_direction;
	
	playables[0].vel[0] = COEF_KICK_CHARGE * (*kick_charge) * verser_ball_to_mouse[0];
	playables[0].vel[1] = COEF_KICK_CHARGE * (*kick_charge) * verser_ball_to_mouse[1];
	
	*kick_charge = 0;
	
	return 0;
}


int 
verify_goal(){
	/*
	Check if it is a goal.
	
	If yes, returns 1.
	*/
	
	if(
		(
			playables[0].pos[0] <= TOPLEFT_X
		) && (
			playables[0].pos[1] >= TOP_GOAL
		) && (
			playables[0].pos[1] <= BOTTOM_GOAL
		)
	){
		// Esquerdo
		
		goals[0]++;
		
		return 1;
	}
	
	if(
		(
			playables[0].pos[0] >= BOTTOMRIGHT_X
		) && (
			playables[0].pos[1] >= TOP_GOAL
		) && (
			playables[0].pos[1] <= BOTTOM_GOAL
		)
	){
		// Direito
		
		goals[1]++;
		
		return 1;
	}
	
	return 0;
}




