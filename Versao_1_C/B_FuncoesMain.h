#ifndef B_FUNCOESFRONT_H
#define B_FUNCOESFRONT_H

#include "C_Player.h"

SDL_Texture*
load_field(
	Display,
	const char*
);

Display
initialize_display(
	void
);

int 
destroy_display(
	Display
);

int 
input_user(
	int*
);

int 
update(
	void
);

int
render(
	Display
);

#endif // B_FUNCOESFRONT_H
