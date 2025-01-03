#ifndef F_FUNCOESBASICAS_H
#define F_FUNCOESBASICAS_H

#include "H_Dependencias.h"

void
apresentar_erro(
	const char *texto_de_apresentacao_de_erro
);

///////////////// Funções de Vetores //////////////

int 
apresentation(
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
