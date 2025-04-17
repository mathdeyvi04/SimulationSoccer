#ifndef LINHA_H
#define LINHA_H

#include "Geometria.h"
#include <stdio.h>

class Linha{
	/*
	Descrição:
		Classe responsável por represetar um segmento de linha a partir de 
		2 vetores tridimensionais.
		
	Créditos: Miguel Abreu (m.abreu@fe.up.pt)
	*/
	
public:
	
	// Para coordenadas esféricas.
	/*
	Para coordenadas esféricas. Basta ver em Geometria.h e no original
	Vector3f.cpp que ele se refere a vetores com raio, phi e theta. Mas em
	polares é para 2D, para 3D nos referimos a esfericas.
	*/
	const Vetor3D inicio_e;
	const Vetor3D final_e;
	
	// Para coordenadas cartesianas.
	const Vetor3D inicio_c;
	const Vetor3D final_c;

	const float comprimento;
	
	//////////////////////////////////////////////////////////////////////
	/// Construturos e Destrutores
	//////////////////////////////////////////////////////////////////////
	
	Linha(
		const Vetor3D& inicio_esf,
		const Vetor3D& final_esf
	) : inicio_e(inicio_esf ),
		 final_e(final_esf  ),
		inicio_c(inicio_esf.para_cartesiano()),
		 final_c(final_esf.para_cartesiano ()),
     comprimento(inicio_c.obter_distancia(final_c))
	{}
	
	Linha(
		const Vetor3D& inicio_cart,
		const Vetor3D& final_cart,
		float comprimento
	) : inicio_e(inicio_cart.para_esferica()),
		 final_e(final_cart.para_esferica()),
		inicio_c(inicio_cart),
		 final_c(final_cart),
	 comprimento(comprimento)
	{}
	
	Linha(
		const Linha& a_ser_copiado
	) : inicio_e(a_ser_copiado.inicio_e),
		 final_e(a_ser_copiado.final_e),
		inicio_c(a_ser_copiado.inicio_c),
		 final_c(a_ser_copiado.final_c),
	 comprimento(a_ser_copiado.comprimento)
	{}
	
	//////////////////////////////////////////////////////////////////////
	/// Métodos de Cálculo Específico
	//////////////////////////////////////////////////////////////////////
	
	Vetor3D ponto_na_linha_mais_perto_cart(
		Vetor3D& ponto_qualquer_cart
	){
		/*
		A partir de um ponto qualquer, achará o ponto em (*this) linha
		que está mais perto deste dado.
		
		Álgebra Linear pura:
			
			Q -> Ponto Qualquer
			P = A + t(B - A) -> Linha
			
			Por definição, o mais perto é perpendicular:
			
			< Q - P , P - A > = 0
			< Q - A - t(B - A) , t(B - A) > = 0
			t< Q - A, B - A > - t^2 < B - A, B - A > = 0
			
			t = < Q - A, B - A > / < B - A, B - A >
			
			Por fim, basta substituir na linha.
		*/

		float param = (ponto_qualquer_cart - inicio_c).InnerProduct(final_c - inicio_c) / (comprimento * comprimento);	
		
		return inicio_c + (final_c - inicio_c) * param;
	}

	
	
};











#endif // LINHA_H
