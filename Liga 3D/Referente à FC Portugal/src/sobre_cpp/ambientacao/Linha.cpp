#include "Linha.h"

int main(){

	Vetor3D A(1, 2, 3);
	Vetor3D B(5, 4, 9);
	
	Linha r(A, B, A.obter_distancia(B));

	Vetor3D Q(2, -20, -2);
	
	Vetor3D P = r.ponto_na_linha_mais_perto_cart(Q);

	Vetor3D::verificar(P);







	return 0;	
}

