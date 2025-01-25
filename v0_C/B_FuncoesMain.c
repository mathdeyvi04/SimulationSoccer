#include "B_FuncoesMain.h"

#define VEL_ADD 40        
 
///////////////////////////////////////////////////////////////////////////////
//// Variáveis Main
///////////////////////////////////////////////////////////////////////////////

// Keep tracking the time of the last frame, in miliseconds.
int last_frame_time = 0;

extern int last_position_mouse[2];

extern Player playables[NUMBER_OF_PLAYERS];

extern SDL_Texture *texture_ball;

extern int ball_dominator[1];

extern int kick_charge[1];

///////////////////////////////////////////////////////////////////////////////
//// Funções Diversas
///////////////////////////////////////////////////////////////////////////////

SDL_Texture*
load_field(
	Display display,
	const char *name_image
){
	/*
	Description:
		Will load the image in the beginning.
	*/
	
	SDL_Surface *bmp_surface = SDL_LoadBMP(
		name_image
	);
	
	SDL_Surface *surface_ball = SDL_LoadBMP(
		"bola.bmp"
	);
	
	texture_ball = SDL_CreateTextureFromSurface(
		display.renderer,
		surface_ball
	);
	
	SDL_FreeSurface(
		surface_ball
	);
	
	if(
		!bmp_surface
	){
		printf(
			"\n(B) Error loading field image.\n"
		);
		
		destroy_display(
			display
		);
	}
	
	SDL_Texture *texture = SDL_CreateTextureFromSurface(
		display.renderer,
		bmp_surface
	);
	
	SDL_FreeSurface(
		bmp_surface
	);
	
	return texture;
}

void
get_mouse_position(){
	/*
	Description:
		Self Explained.
	*/
	
	SDL_GetMouseState(
		last_position_mouse,
		last_position_mouse + 1
	);
	
	printf(
		"\nÚltima Posição do Mouse -> (%d, %d).",
		last_position_mouse[0],
		last_position_mouse[1]
	);
}


///////////////////////////////////////////////////////////////////////////////
//// Funções Main
///////////////////////////////////////////////////////////////////////////////


Display
initialize_display(
	void
){
	/*
	Description:
		Function responsible for checking and initializing the window and the renderer.
		Since these are structures that we need together, it is more interesting to use
		a struct of the union of them.
	
	Return possibilities:
		Display in conditions to be used, being this composed of the window and the renderer;
		
		Display with NULL renderer, if only this one gave an error;
		
		NULL display.
	*/
	
	Display result = {
		NULL,
		NULL,
		NULL
	};
	
	if(
		SDL_Init(
			SDL_INIT_EVERYTHING
		) != 0
	){
		display_error(
			"\n(B) There was an error initializing SDL general tools.\n"
		);
		
		return result;
	}

	SDL_Window *window = SDL_CreateWindow(
		"My Window",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIDTH_SCREEN,
		HEIGHT_SCREEN,
		SDL_WINDOW_SHOWN
	);
	
	if(
		!window
	){
		display_error(
			"\n(B) There was an error trying to create the window.\n"
		);
		
		return result;
	}
	
	result.window = window;
	
	SDL_Renderer *renderer = SDL_CreateRenderer(
		window,  // The window we are going to attach it to;
		-1,  // -1 defautl;
		0  // No special features.
	);
	
	if(
		!renderer
	){
		display_error(
			"\n(B)There was an error trying to create renderer."
		);
		
		return result;
	}
	
	result.renderer = renderer;
	
	result.image_field = load_field(
		result,
		IMAGE_NAME
	);
	
	return result;
}


int
destroy_display(
	Display display
){
	/*
	Description:
		Function responsible for correctly closing the display.
	
	Parameters:
		Self-explanatory.
	
	Return:
		Display closes.
	*/
	
	
	if (
		!display.renderer
	){
		SDL_DestroyRenderer(
			display.renderer
		);
	}
	
	if (
		!display.window
	){
		SDL_DestroyWindow(
			display.window
		);
	}
	
	SDL_Quit();
	
	return 0;
}


int
input_user(
	int *simulation_is_running
){
	/*
	Description:
		Function responsible for managing user input.
	
	Parameters:
	
	Return:
		Call to the respective input functions.
	*/
	
	SDL_Event event;
	
	SDL_PollEvent(
		&event
	);
	
	switch(
		event.type
	){
		
		case SDL_QUIT:
			*simulation_is_running = 0;
			
			return 0;
			
		case SDL_KEYDOWN:
		////////////////////////////////////////////////////////////////////////
		///// Input de KeyDown
		////////////////////////////////////////////////////////////////////////
			switch(
				event.key.keysym.sym
			){
				case SDLK_ESCAPE:
					
					*simulation_is_running = 0;
					
					return 0;
					
				case SDLK_m:

					get_mouse_position();
					
					return 0;
					
				case SDLK_w:
					
					playables[1].vel[1] +=  - VEL_ADD;
					
					return 0;
					
				case SDLK_d:
					
					playables[1].vel[0] += VEL_ADD;
					
					return 0;
					
				case SDLK_a:
					
					playables[1].vel[0] += - VEL_ADD;
					
					return 0;
					
				case SDLK_s:
						
					playables[1].vel[1] += VEL_ADD;
					
					return 1;
									
				default:
					return 0;
			}
			
		////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		
		case SDL_MOUSEBUTTONDOWN:
			
			if(
				event.button.button == SDL_BUTTON_LEFT
			){
				// Direito 
				
				if(
					close_enough(
						playables + 1
					)
				){
					*kick_charge = 1;
				}
			}
			else{
				// Esquerdo
				// Segurar bola.
				
				if(
					close_enough(
						playables + 1
					)
				){
					*ball_dominator = 1;
				}
				
			}
			
			return 0;
			
		case SDL_MOUSEBUTTONUP:
			
			if(
				event.button.button == SDL_BUTTON_LEFT
			){
				// Direito Levantado
				
				get_mouse_position();
				
				kick_ball(
					*kick_charge
				);
				
				*kick_charge = 0;
	
			}
			else{
				// Esquerdo Levantado
				
				*ball_dominator = 0;
			}
			
			return 0;
			
		case SDL_MOUSEWHEEL:
			/*
			event.wheel.y is the amount scroll gives by the user.
			*/
						
			return 0;

		default:
			return 0;
	}
	
}


int
update(
	double *delta_time
){
	/*
	Description:
		Function responsible for managing the respective
		updates of each simulation member.

	Parameters:

	Return:
		Simulation members updated in a multiprocessor way.
	*/
	
	/*
	In the past:
	
		Waste time until we reach the frame target time
		while(
			!SDL_TICKS_PASSED(
				SDL_GetTicks(),
				last_frame_time + FRAME_TARGET_TIME
			)
		);
		We can put the pthread_cond_wait here!
	*/
	int time_to_wait = FRAME_TARGET_TIME - (
		SDL_GetTicks() - last_frame_time
	);
	
	if(
		time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME
	){
		SDL_Delay(
			time_to_wait
		);
	}
	
	*delta_time = (
		SDL_GetTicks() - last_frame_time
	) / 1000.0;
	
	last_frame_time = SDL_GetTicks();
	
	pthread_barrier_wait(
		/*
		Releases coach threads that were waiting for delta time
		*/
		&coachs_command_flow
	);
	
	pthread_barrier_wait(
		/*
		Wait for coachs threads to update and to move the players.
		*/
		&coachs_command_flow
	);
	
	// All three free.
	if(
		*ball_dominator
	){
		playables[0].vel[0] = playables[*ball_dominator].vel[0];
		playables[0].vel[1] = playables[*ball_dominator].vel[1];
	}
	
	if(
		*kick_charge
	){
		if(
			*ball_dominator
		){
			*kick_charge += 1;
		}
		else{
			// Unload the kick
			*kick_charge = 0;
		}
			
	}
	
	// Only the ball.
	if(
		secure_player(
			playables
		)
	){
		*ball_dominator = 0;
	}
	
	moviment(
		playables,
		*delta_time
	);
	
	return 0;
}


int
render(
	Display display
){
	/*
	Description:
		Function responsible for managing screen updates.

	Parameters:

	Return:
		Screen updated according to rendering needs.
	*/
	
	/*
	COACHS THREADS ESTÃO FAZENDO COLOCANDO E SÓ DPS A MAIN FAZ ISSO.
	
	POR ISSO ESTÁ DANDO ERRADO.
	*/

 	////////////////////////////////////////////////////////////////////////////
	//// Renderizações
	////////////////////////////////////////////////////////////////////////////
	
	draw_a_player(
		playables[0],
		display
	);
	
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	
	SDL_RenderPresent(
		display.renderer
	);
}


int
sub_render(
	Display display
){
	/*
	
	*/
	
	SDL_SetRenderDrawColor(
		/*
		Set the color used for drawing operations
		*/
		display.renderer,
		0,  // R
		0,  // G
		0,  // B
		255  // Transparency.
	);
	
	SDL_RenderClear(
		display.renderer
	);
	
	SDL_RenderCopy(
		display.renderer,
		display.image_field,
		NULL,
		NULL
	);
	
	return 1;
}























