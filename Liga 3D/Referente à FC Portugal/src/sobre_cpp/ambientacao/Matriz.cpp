#include "Matriz.h"

int main(){
	
	Matriz m = Matriz(Vetor3D(1001, 1002, 1003));
	
	for(
		int i = 0;
		i < M_TAMANHO;
		i++
	){
		printf("%f ", m.conteudo[i]);
	}
	
	return 0;
}






















