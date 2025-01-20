#include "C_Player.h"

//////////////////////////////////////////////////////////////////////////////
//// Variables for Testing
//////////////////////////////////////////////////////////////////////////////

/*
In the function 'draw_a_player', we use this variable to visualize the total
of iterations or the total pixels necessary to print the object.
*/
int FOR_CHECKING_PIXEL_NUMBER = 0;

//////////////////////////////////////////////////////////////////////////////
//// Funções 
//////////////////////////////////////////////////////////////////////////////


int
generate_players( 
	Player *list_of_playable
){
	/*
	Description:
		Function responsible for create a generation of playables.
		Each one with respectives attributes. 
	*/
	
	for(
		int i = 0;
		i < NUMBER_OF_PLAYERS;
		i++
	){ 
		if(
			!i // Only for i == 0.
		){
			// The Ball
			
			list_of_playable[0] = (Player) {
				.side = RADIUS_BALL, 
				.color = {255, 255, 255},
				.mass = MASS_BALL,
				// Initial position ball
				.pos = {MEDIUM_X, MEDIUM_Y},
				.vel = {0, 0},
				.acel = {0, 0}
			};
			
			continue;
		}
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
	
	int total_pixels = 0;
	
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
			playable.pos[0] - playable.side,
			playable.pos[1] + playable.side,
			playable.side,
			playable.side
		};
		
		SDL_RenderFillRect(
			display.renderer,
			&rect
		);
		
		return 1;
	}
	
	for(
		int w = 0;
		w < 2 * playable.side;
		w = w + TAM_MINOR_RECT
	){
		for(
			int h = 0;
			h < 2 * playable.side;
			h = h + TAM_MINOR_RECT
		){
			
			int dx = playable.side - w;
			int dy = playable.side - h;
			
			if(
				(dx * dx + dy * dy) <= (playable.side * playable.side)	
			){
				SDL_Rect rect = {
					playable.pos[0] + dx,
					playable.pos[1] + dy,
					TAM_MINOR_RECT,
					TAM_MINOR_RECT
				};
				
				SDL_RenderFillRect(
					display.renderer,
					&rect
				);		
				
				total_pixels++;
	
			}
			
		}
		
	}
	
	if(
		!FOR_CHECKING_PIXEL_NUMBER
	){
		FOR_CHECKING_PIXEL_NUMBER += 1;
		printf("\nFiz %d iteracoes.", total_pixels);
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











