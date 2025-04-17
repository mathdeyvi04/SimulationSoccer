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
	
	Vetor3D ponto_na_reta_mais_perto_cart(
		const Vetor3D& ponto_qualquer_cart
	){
		/*
		A partir de um ponto qualquer, achará o ponto na RETA INFINITA
		definida pela linha, this.
		
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

	/*
	No código original, esta função e outra posterior são utilizadas.
	Entretanto, observe que ela força que a entrada da função anterior seja
	um valor de cópia, não seja um valor de referência.
	
	Tal fato configura erro, por isso achou-se melhor não fazê-lo.
	
	Vetor3D ponto_na_linha_mais_perto_esf(
		Vetor3D& ponto_qualquer_esf
	){
		
		// Note que estamos recebendo em esf e retornaremos em cartesiana!!!
		
		return ponto_na_linha_mais_perto_cart(ponto_qualquer_esf.para_cartesiano());
	}
	*/
	
	float distancia_ate_ponto_cart(
		const Vetor3D& ponto_qualquer_cart
	){
		/*
		Distância de um ponto a uma linha. 
		
		Suponha B - A o segmento de linha pertencente à linha e 
		Q o ponto qualquer.
		
		Note que:
		
		2 * Area = |(Q - inicio_c) X (Q - final_c)| = Base Altura
		
		Altura = |(Q - inicio_c) X (Q - final_c)| / |final_c - inicio_c|
		*/
		
		return ((ponto_qualquer_cart - inicio_c).CrossProduct(ponto_qualquer_cart - final_c)).modulo() / comprimento;
	}

	/*
	float distancia_ate_ponto_esf(
		const Vetor3D ponto_qualquer_esf
	){
		return distancia_ate_ponto_cart(ponto_qualquer_esf.para_cartesiano());
	}
	*/
	
	
	float distancia_ate_linha(
		const Linha& linha
	)  {
		/*
		Suponha duas retas definidas pelas linhas, (*this) e a dada.
		De tal forma que:
		
		P = A + t(B - A)       		Q = N + r(M - N)
		
		São pontos quaisquer respectivamente nas mesmas. Se desejamos a
		distância entre essas linhas, devemos buscar P - Q de tal forma
		que essa linha seja perpendendicar à B - A e à M - N simutalneamente.
		
		< P - Q, B - A > = 0
		< P - Q, M - N > = 0
		
		Em seguida, abrir:
		
		t< B - A, B - A > - r< M - N, B - A > = < N - A, B - A >
		t< B - A, M - N > - r< M - N, M - N > = < N - A, M - N >
		
		Observe que temos um sistema linear. Há três opções.
		
		i) Não há solução.
			
			Como estamos dimensões no máximo tridimensionais, sempre haverá
			pelo menos uma solução.
		
		ii) Infinitas soluções.
			
			Caso em que as retas estão paralelas. Há infinitos pontos que
			satisfazem a perpendicularidade.
			
			Basta que o determinante da matriz dos coeficientes seja zero.
			
			Então basta escolher um ponto da linha dada como fixo, N por exemplo,
			e obtermos a distância à esse ponto. Neste caso, a solução é trivial.
		
		iii) Uma única solução.
		
			Para facilitar, podemos denotar os coeficientes como parâmetros e
			assim resolver o sistema 2x2.
		*/

		if(
			// Verificar se são paralelas ou pelo menos QUASE paralelas
			fabs(
				(linha.final_c - linha.inicio_c).InnerProduct((*this).final_c - (*this).inicio_c)
			) - ((*this).comprimento * linha.comprimento) < 1e-4			
		){
			
			Vetor3D elemento_inicial = linha.inicio_c;
			return distancia_ate_ponto_cart(elemento_inicial);
		}
		
		return 1;
		
	}
	
	
	
};











#endif // LINHA_H
