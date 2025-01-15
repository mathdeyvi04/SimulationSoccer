#ifndef B_FUNCOESFRONT_H
#define B_FUNCOESFRONT_H

#include "C_Player.h"

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
render(
	Display
);


int 
update(
	void
);


#endif // B_FUNCOESFRONT_H
