#include "CA_Functions_Passive.h"

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

extern TTF_Font *font_to_be_used;

//////////////////////////////////////////////////////////////////////////////
//// Funções Base dos Jogadores
//////////////////////////////////////////////////////////////////////////////


int 
generate_location(){
	/*
	Description:
		Function responsible for generate each location's player at 
		the beginning.
		
		Modify the variable location_at_beginning.
		
		In the future, the ideal is to have an interface with the
		checkered field, so the user can choose the initial locations.
	*/
	
	int 
	row_and_column_player[
		2 * NUMBER_OF_PLAYERS_IN_EACH_TEAM
	][
		2
	] = {
		{2, 17},  // Goleiro
		{8, 8},
		{8, 17},
		{8, 26},
		
		{30, 17}, // GoalKeeper
		{23, 8},
		{23, 17},
		{23, 26}
	};
	
	for(
		int index_location_entrance = 0;
		index_location_entrance < (2 * NUMBER_OF_PLAYERS_IN_EACH_TEAM);
		index_location_entrance++
	){
		
		location_at_beginning[
			index_location_entrance
		][
			0
		] = row_and_column_player[
			index_location_entrance
		][
			0
		] * NUMBER_OF_ROWS;
		
		location_at_beginning[
			index_location_entrance
		][
			1
		] = row_and_column_player[
			index_location_entrance
		][
			1
		] * NUMBER_OF_COLUMNS;
		
	}
	
	return 0;
}


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
	
	// Location Initial
	generate_location();
	
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
	Display display,
	int number
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
	
	// Puttin the number on the player.
	
	if(
		number > 9
	){
		snprintf(
			buffer_number,
			2,
			"%d",
			number
		);
	}
	else{
		snprintf(
			buffer_number,
			2,
			"%d\0",
			number
		);
	}
	
	SDL_Surface *surface_text = TTF_RenderText_Solid(
		font_to_be_used,
		buffer_number,
		(SDL_Color){255, 255, 255}
	);
	
	SDL_Texture *texture_time_match = SDL_CreateTextureFromSurface(
		display.renderer,
		surface_text
	);
	
	SDL_RenderCopy(
		display.renderer,
		texture_time_match,
		NULL,
		&for_draw_information[2]
	);
	
	SDL_FreeSurface(surface_text);
	
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
				location_at_beginning[
					NUMBER_OF_PLAYERS_IN_EACH_TEAM * (coach_info.team_indicator - 1) + (i / 2)
				][0],
				location_at_beginning[
					NUMBER_OF_PLAYERS_IN_EACH_TEAM * (coach_info.team_indicator - 1) + (i / 2)
				][1]
			},
			.vel = {0, 0},
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
				*(coach_info.display),
				1 + (i / 2)
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








