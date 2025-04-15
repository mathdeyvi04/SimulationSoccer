#include "Matriz.h"

int main(){
	
	Matriz m = Matriz(Vetor3D(11, 12, 13));
	
	for(
		int i = 0;
		i < M_TAMANHO;
		i++
	){
		
		printf("%f ", m.obter(i));
		if( (i + 1) % 4 == 0){
			printf("\n");
		}
	}
	return 0;
}






















