#include "Linha.h"

int main(){

	Vetor3D A(1, 2, 3);
	Vetor3D B(1, 2, -2);
	Vetor3D N(2, -1, 3);
	Vetor3D M(2, -1, -5);
	
	Linha L(A, B, A.obter_distancia(B));
	Linha S(N, M, N.obter_distancia(M));
	
	float valor = L.distancia_ate_linha(S);
	printf("%f", valor);






	return 0;	
}

