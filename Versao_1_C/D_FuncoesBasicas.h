#ifndef F_FUNCOESBASICAS_H
#define F_FUNCOESBASICAS_H

#include "E_Dependencias.h"

void
display_error(
	const char *error_display_text
);

///////////////// Funções de Vetores //////////////

int 
apresentation_vector(
	double*
);

double*
sum_vectors(
	double*, double*
);

double*
multiple_vector_by_scalar(
	double*,
	double
);

double
get_module_squared(
	double*
);


#endif  // F_FUNCOESBASICAS_H
