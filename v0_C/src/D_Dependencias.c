#include "D_Dependencias.h"


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













