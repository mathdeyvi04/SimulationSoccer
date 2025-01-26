#include "D_FuncoesBasicas.h"


/////////////////////////////////////////////////////////////////////////////
///// Funções Gerais
/////////////////////////////////////////////////////////////////////////////

void
display_error(
	const char *error_display_text
){
	/*
	Description:
		Function responsible for displaying the error in red.
	*/
	printf("\033[1;31m");
	
    printf("%s", error_display_text);   
    printf("\033[0m");   
}

/////////////////////////////////////////////////////////////////////////////
///// Funções Relacionadas à Vetores
/////////////////////////////////////////////////////////////////////////////

int apresentation_vector(
	double *vector
){
	/*
	Description:
		Function responsabile for presenting the vector.
	
	Parameters:
		Self Explained
	
	Return:
		Vector presented.
	*/
	
	for(
		int i = 0;
		i < N_DIMENSIONS;
		i++
	){
		if(
			i == 0
		){
			printf("(");
		}
		
		if(
			i == (N_DIMENSIONS - 1)
		){
			printf("%lf)", vector[i]);
			
			return 1;
		}
		
		printf("%lf, ", vector[i]);
	}
	
}

double*
sum_vectors(
	double *vector_1,
	double *vector_2
){
	/*
	Description:
		Function responsible for adding two vectors, reusing
		the memory space used by the first vector, for not 
		explode our simulation.
			
	Parâmetros:
		Self Explained.
	
	Return:
		Vector summed in the same location that vector_1.
	*/
	
	for(
		int i = 0;
		i < N_DIMENSIONS;
		i++
	){
		vector_1[i] = vector_1[i] + vector_2[i];
	}
	
	return vector_1;
}

double*
multiple_vector_by_scalar(
	double *vector_1,
	double scalar
){
	/*
	Description:
		Self Explained.
		The return vector use the same space that 
		vector_1.
	
	Parameters:
		Self Explaneid.
	
	Return:
		Vector multipled by the scalar. Allocaded in 
		the vector_1 adress.
	*/
	
	for(
		int i = 0;
		i < N_DIMENSIONS;
		i++
	){
		vector_1[i] = vector_1[i] * scalar;
	}
	
	return vector_1;
}

double
get_module_squared(
	double *vector_1
){
	/*
	Description:
		Function responsable for get the module of a vector
		squared. The reasion I made this form is only for 
		perfomance.
	
	Parameters:
		Self Explained.
	
	Return:
		Module Squared of the vector.
	*/
	
	double module_squared = 0;	
	
	for(
		int i = 0;
		i < N_DIMENSIONS;
		i++
	){
		module_squared = module_squared + vector_1[i] * vector_1[i];	
	}
	
	return module_squared;
}


































