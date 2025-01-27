#include "CC_Animation.h"

int
goal_animation(
	Display display
){
	/*
	Description:
		Performs all the consequences of a goal.
	*/
	
	playables[0].pos[0] = MEDIUM_X;
	playables[0].pos[1] = MEDIUM_Y;
	
	playables[0].vel[0] = 0;
	playables[0].vel[1] = 0;
	 
	if(
		*ball_dominator
	){
		*ball_dominator = 0;
	}
	
	if(
		*kick_charge
	){
		*kick_charge = 0;
	}
	
	SDL_Surface *surface_text = TTF_RenderText_Solid(
		font_to_be_used,
		"GOAL!!",
		(SDL_Color){0, 0, 0}
	);
	
	SDL_Rect position_goal_animation = {
		MEDIUM_X - 50 + 6,
		MEDIUM_Y,
		100,
		TOPLEFT_Y - 10
	};
	
	SDL_Texture *texture_goal_animation = SDL_CreateTextureFromSurface(
		display.renderer,
		surface_text
	);
	
	SDL_RenderCopy(
		display.renderer,
		texture_goal_animation,
		NULL,
		&position_goal_animation
	);
	
	SDL_FreeSurface(surface_text);
	
	return 0;
}


