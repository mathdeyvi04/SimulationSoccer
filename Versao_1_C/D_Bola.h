#ifndef D_BOLA_H
#define D_BOLA_H

#include "F_FuncoesBasicas.h"

typedef struct {
	
	// Visual na Tela
	double radius;
	
	// Cinematic Attributes
	int mass;
	double pos[N_DIMENSIONS];
	double vel[N_DIMENSIONS];
	double acel[N_DIMENSIONS];
} Bola;





#endif // D_BOLA_H
