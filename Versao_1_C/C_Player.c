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
	
	for(
		int w = 0;
		w < 2 * playable.radius;
		w = w + TAM_MINOR_RECT
	){
		for(
			int h = 0;
			h < 2 * playable.radius;
			h = h + TAM_MINOR_RECT
		){
			
			int dx = playable.radius - w;
			int dy = playable.radius - h;
			
			if(
				(dx * dx + dy * dy) <= (playable.radius * playable.radius)	
			){
				//SDL_RenderDrawPoint(
				//	display.renderer,
				//	playable.pos[0] + dx,
				//	playable.pos[1] + dy
				//);
				
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














