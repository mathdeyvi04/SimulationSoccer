#ifndef B_FUNCOESFRONT_H
#define B_FUNCOESFRONT_H

#include "CA_Functions_Passive.h"

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
	double*
);

int
render(
	Display
);

int 
sub_render(
	Display
);

#endif // B_FUNCOESFRONT_H
