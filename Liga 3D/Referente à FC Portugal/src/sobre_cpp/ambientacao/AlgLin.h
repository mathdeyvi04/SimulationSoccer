/*
Matheus Deyvisson, 2025

Utilizei apenas um arquivo header a fim de evitar a repeti��o 
desnecess�ria de v�rios arquivos.

Inclui os arquivos:
    -> Vector3f.cpp
    -> Vector3f.h
    -> Vector3f.cpp
    -> Line6f.cpp
    -> Line6f.h
    -> Geometry.cpp
    -> Geometry.h
    -> Matrix4D.cpp
    -> Matrix4D.h

Fiz algumas modifica��es para aprimorar legibilidade.
Testes j� foram realizados.
*/
#ifndef GEOMETRIA_H
#define GEOMETRIA_H

#include <cstdio>
#include <cmath>
#include <math.h>
#include <algorithm>

using namespace std;

// Para manipula��es da classe Matriz
#define M_LINHAS  4
#define M_COLUNAS 4
#define M_TAMANHO M_COLUNAS * M_LINHAS

// Para manipula��es das classes de Vetores
#define EPSILON 1e-6  // No Original, era 1e-10, o que eu julgo desnecessariamente pequeno...
#define PI 3.1415926535f  // Sim, sei da exist�ncia do M_PI da biblioteca padr�o.

/*
Fun��es que ser�o amplamente utilizadas
*/

inline float cos_deg(
	float ang_degraus
){
	/*
	Descri��o:
		A fun��o base da biblioteca somente aceita radianos.
	*/
	return cos( ang_degraus * PI / 180 );
}


inline float sin_deg(
	float ang_degraus
){
	return sin( ang_degraus * PI / 180 );
}


inline float atan2_deg(
	float img,
	float real
){
	/*
	Descri��o:
		Calcula o arco cuja tangente � img / real, em degraus.
	*/
	
	if(
		fabs(img) < EPSILON && fabs(real) < EPSILON
	){
		return 0.0f;
	}
	
	return atan2(img, real) * 180 / PI;
}


class Vetor2D {
public:
	/*
	Descri��o:
		Representa a posi��o no espa�o 2D.
		
		Uma posi��o ser� representada em coordenadas
		cartesianas, (x, y).
		
		Cr�ditos para o Alto N�vel de Trabalho:
		- @author Hugo Picado (hugopicado@ua.pt)
		- @author Nuno Almeida (nuno.alm@ua.pt)
		- Adapted - Miguel Abreu
	*/
	
	float x;
	float y;

	Vetor2D() : x(0), y(0) {}
	
	Vetor2D(
		/*
		Iniciamos um vetor com componentes x e y.
		Caso nenhum valor seja passado, assumir� 0.
		*/
		float valor_x,
		float valor_y
	) : x(valor_x), y(valor_y) {}
	
	////////////////////////////////////////////////////////////////////////
	/// Operadores de Aritm�tica
	////////////////////////////////////////////////////////////////////////
	
	/*
	Observe que passaremos argumentos por refer�ncia! Como se fossem
	ponteiros.
	
	O const no final indica que n�o estaremos modificando o vetor dado, 
	estaremos criando um novo!
	*/
	
	// - Vector2D( 1, 2 ) -> Vector2D( -1, -2 )
	Vetor2D operator-() const {
		return Vetor2D( - x, - y );
	}
	
	
	Vetor2D operator+( const float   &escalar     ) const {
		/*
		Soma com escalar resulta em componentes somadas ao escalar.
		*/
	    return Vetor2D( x + escalar, y + escalar );
	}
	Vetor2D operator+( const Vetor2D &outro_vetor ) const {
		/*
		Soma de outros vetores � a soma de suas componentes.
		*/
		return Vetor2D( x + outro_vetor.x, y + outro_vetor.y );
	}
	
	
	Vetor2D operator-( const float   &escalar     ) const {
		return Vetor2D( x - escalar, y - escalar);
	}
	Vetor2D operator-( const Vetor2D &outro_vetor ) const {
		return Vetor2D( x - outro_vetor.x, y - outro_vetor.y );
	}
	
	
	Vetor2D operator*( const float   &escalar     ) const {
		return Vetor2D( x * escalar, y * escalar );
	}
	Vetor2D operator*( const Vetor2D &outro_vetor ) const {
		/*
		N�o � produto escalar pois gerar� novo vetor.
		*/
		return Vetor2D( x * outro_vetor.x, y * outro_vetor.y);
	}
	
	
	Vetor2D operator/( const float   &escalar     ) const {
		/*
		Assim como no produto com escalar, aqui faremos divis�o com escalar.
		*/
		return Vetor2D( x / escalar, y / escalar );
	}
	Vetor2D operator/( const Vetor2D &outro_vetor ) const {
		/*
		Divis�o de componente por componente.
		*/
		return Vetor2D( x / outro_vetor.x, y / outro_vetor.y);
	}
	
	////////////////////////////////////////////////////////////////////////
	/// Operadores de Atribui��o
	////////////////////////////////////////////////////////////////////////
	
	/*
	Os m�todos a seguir alteram o seu pr�prio valor.
	N�o criam novos.
	*/
	
	// Vetor2D vetor = 1 -> Vector2D( 1, 1 )
	void operator =( const float   &escalar     ) {
		x = escalar;
		y = escalar;
	}
	
	void operator+=( const float   &escalar     ){
		/*
		Soma o escalar �s componentes.
		*/
		x += escalar;
		y += escalar;
	}
	void operator+=( const Vetor2D &outro_vetor ) {
		/*
		Somar� as respectivas componentes
		*/
		x += outro_vetor.x;
		y += outro_vetor.y;
	}
	
	void operator-=( const float   &escalar     ) {
		x -= escalar;
		y -= escalar;
	}
	void operator-=( const Vetor2D &outro_vetor ) {
		x -= outro_vetor.x;
		y -= outro_vetor.y;
	}
	
	void operator*=( const float   &escalar     ) {
		x *= escalar;
		y *= escalar;
	}
	void operator*=( const Vetor2D &outro_vetor ) {
		x *= outro_vetor.x;
		y *= outro_vetor.y;
	}
	
	void operator/=( const float   &escalar     ) {
		x /= escalar;
		y /= escalar;
	}
	void operator/=( const Vetor2D &outro_vetor ) {
		x /= outro_vetor.x;
		y /= outro_vetor.y;		
	}
	
	bool operator!=( const float   &escalar     ) {
		return ( x != escalar ) || ( y != escalar );
	}
	bool operator!=( const Vetor2D &outro_vetor ) {
		return ( x != outro_vetor.x ) || ( y != outro_vetor.y );
	}
	
	bool operator==( const float   &escalar     ) {
		return ( x == escalar) && ( y == escalar );
	}
	bool operator==( const Vetor2D &outro_vetor ) {
		return ( x == outro_vetor.x ) && ( y == outro_vetor.y );
	}
	
	//////////////////////////////////////////////////////////////////////
	/// M�todos
	//////////////////////////////////////////////////////////////////////
	
	// Admito que os nomes em ingl�s s�o mais fodas.
	float InnerProduct( const Vetor2D &outro_vetor) const {
		//	N�o far� modifica��es nos vetores.
		return x * outro_vetor.x + y * outro_vetor.x;
	}
	
	float CrossProduct( const Vetor2D outro_vetor ) {
		/*
		Componente da 3� dimens�o vetor produto vetorial dos vetores dados.
		*/
		return x * outro_vetor.y - y * outro_vetor.x;
	}
	
	float modulo() const {
		return sqrt(x * x + y * y);
	}
	
	float obter_distancia    ( const Vetor2D outro_vetor ) {
		/*
		Note que � o m�dulo do vetor diferen�a.
		*/
		return (*this - outro_vetor).modulo();
	}
};


class Vetor3D {
	/*
	Descri��o:
		Representar� a posi��o em 3 valores float.
		
	Cr�ditos:
	* \author Hugo Picado (hugopicado@ua.pt)
 	* \author Nuno Almeida (nuno.alm@ua.pt)
 	* Adapted - Miguel Abreu
	*/
	
public:
	
	float x;
	float y;
	float z;
	
	/////////////////////////////////////////////////////////////////////
	/// Construtores e Destrutores
	/////////////////////////////////////////////////////////////////////		
	
	// Defini��o vazia
	Vetor3D() : x(0), y(0), z(0) {}
	
	
	// Defini��o usando n�meros como entrada
	Vetor3D(
		float v_x, 
		float v_y,
		float v_z
	) : x(v_x), y(v_y), z(v_z) {}
	
	
	// Defini��o usando outro vetor, criar� uma c�pia do mesmo
	Vetor3D(
		const Vetor3D& outro_vetor
	) : x(outro_vetor.x), y(outro_vetor.y), z(outro_vetor.z) {}
	
	
	// Defini��o usando um vetor 2D, criando um novo vetor e de componente z nula
	Vetor3D(
		const Vetor2D& outro_vetor
	) : x(outro_vetor.x), y(outro_vetor.y), z(0) {}
		
	~Vetor3D() {} // Apenas o destrutor.
	
	///////////////////////////////////////////////////////////////////////
	/// Outros M�todos de Defini��o
	///////////////////////////////////////////////////////////////////////
	
	float obter_x() const { return x; }
	float obter_y() const { return y; }
	float obter_z() const { return z; }

	void setar_todos(
		float v_x,
		float v_y,
		float v_z
	){

		x = v_x;
		y = v_y;
		z = v_z;
	}
	void setar_x( float v_x ) { x = v_x; }
	void setar_y( float v_y ) { y = v_y; }
	void setar_z( float v_z ) { z = v_z; }
	
	float operator[]( const int index ) const {
		/*
		Acessar elementos do vetor a partir de indexs de um array.
		*/
		float valor = 0;
		switch ( index ){
			case 0:
				valor = x;
				break;
			case 1:
				valor = y;
				break;
			case 2:
				valor = z;
				break;

			default:
				/*Houve um erro obviamente.*/
				break;
		}

		return valor;
	}
	
	///////////////////////////////////////////////////////////////////////
	/// Operadores de Aritm�tica
	///////////////////////////////////////////////////////////////////////
	
	Vetor3D operator+( const Vetor3D& outro_vetor ) const {
		return Vetor3D(
						x + outro_vetor.x,
						y + outro_vetor.y,
						z + outro_vetor.z
					  );
	}   
	Vetor3D operator+( float escalar ) const {
		return Vetor3D(
						x + escalar,
						y + escalar,
						z + escalar
					  );
	}                   
	            
				          
	Vetor3D operator-( const Vetor3D& outro_vetor ) const {
		return Vetor3D(
						x - outro_vetor.x,
						y - outro_vetor.y,
						z - outro_vetor.z
					  );
	}
	Vetor3D operator-( float escalar ) const {
		return Vetor3D(
						x - escalar,
						y - escalar,
						z - escalar
					  );
	} 
	Vetor3D operator-() const {
		return Vetor3D(
						- x,
						- y,
						- z
			   		  );
	}
	
	
	Vetor3D operator*( const Vetor3D& outro_vetor ) const {
		return Vetor3D(
						x * outro_vetor.x,
						y * outro_vetor.y,
						z * outro_vetor.z
					  );
	}
	Vetor3D operator*( float escalar ) const {
		return Vetor3D(
						x * escalar,
						y * escalar,
						z * escalar
					  );
	}
	
	
	Vetor3D operator/( const Vetor3D& outro_vetor ) const {
		return Vetor3D(
						x / outro_vetor.x,
						y / outro_vetor.y,
						z / outro_vetor.z
					  );
	}
	Vetor3D operator/( float escalar) const {
		return Vetor3D(
						x / escalar,
						y / escalar,
						z / escalar
					  );
	}
	
	
	Vetor3D operator%( float escalar ) const {
		return Vetor3D(
						fmod(x, escalar),
						fmod(y, escalar),
						fmod(z, escalar)
					  );
	}
	
	//////////////////////////////////////////////////////////////////////
	/// Operadores de Atribui��o
	//////////////////////////////////////////////////////////////////////
	
	bool operator==( const Vetor3D& outro_vetor ) const {
		return x == outro_vetor.x && y == outro_vetor.y && z == outro_vetor.z;
	}
	
	
	Vetor3D operator+=( const Vetor3D& outro_vetor) {
		x += outro_vetor.x;
		y += outro_vetor.y;
		z += outro_vetor.z;
		
		return *this;  // Garantir a sem�ntica	
	}
	Vetor3D operator+=( float escalar ) {
		x += escalar;
		y += escalar;
		z += escalar;	
		
		return *this;  // Garantir a sem�ntica
	}
	
	
	Vetor3D operator-=( const Vetor3D& outro_vetor) {
		x -= outro_vetor.x;
		y -= outro_vetor.y;
		z -= outro_vetor.z;
		
		return *this;	
	}
	Vetor3D operator-=( float escalar ) {
		x -= escalar;
		y -= escalar;
		z -= escalar;
		
		return *this;	
	}
	
	
	Vetor3D operator/=( float escalar ) {
		x /= escalar;
		y /= escalar;
		z /= escalar;
		
		return *this;
	}
	
	//////////////////////////////////////////////////////////////////////
	/// M�todos de C�lculo
	//////////////////////////////////////////////////////////////////////
	
	float InnerProduct( const Vetor3D& outro_vetor ) const {
		return x * outro_vetor.x + y * outro_vetor.y + z * outro_vetor.z;
	}
	
	
	Vetor3D CrossProduct( const Vetor3D& outro_vetor ) const {
		/*
		Retornar� o produto vetorial.
		*/
		return Vetor3D(
						y * outro_vetor.z - z * outro_vetor.y,
						z * outro_vetor.x - x * outro_vetor.z,
						x * outro_vetor.y - y * outro_vetor.x
					  );
	}
	
	
	float modulo() const {
		return sqrt(x * x + y * y + z * z);
	}
		
	
	Vetor3D normalize() const {
		/*
		Vamos pegar versor unit�rio do vetor.
		*/
		return (*this) / (*this).modulo();
	}
	
	
	Vetor3D para_cartesiano() const {
		/*
		Receberemos um vetor em coordenadas esfericas, o qual:
			x -> distancia
			y -> theta, em graus, �ngulo horizontal segundo original.
			z -> phi, em graus,   �ngulo vertical   segundo original.
		Para transform�-lo em cartesianas.
		*/
		return Vetor3D(
						x * cos_deg( z ) * cos_deg( y ),
						x * cos_deg( z ) * sin_deg( y ),
						x * sin_deg( z )
					  );
	} 
	
	
	Vetor3D para_esferica() const {
		/*
		Receberemos um vetor em coordenadas cartesianas e 
		o levamos para coordenadas esf�ricas.
		x -> distancia
		y -> theta
		z -> phi
		*/
		
		return Vetor3D(
						(*this).modulo(),
						atan2_deg( y, x ),
						atan2_deg( z, sqrt(x * x + y * y))
					  );
	}
	
	
	float obter_distancia( const Vetor3D& outro_vetor ) const {
		return (*this - outro_vetor).modulo();
	}
	
	
	Vetor2D para_2D() const {
		/*
		Transformar� o vetor de 3 dimens�es em de 2 dimens�es,
		ignorando a terceira.
		*/
		return Vetor2D(
						x,
						y
					  );
	}
	
	/*
	Esse static � como se fosse o staticmethod do Python. Garante
	que n�o seja necess�rio uma inst�ncia da classe para execu��o 
	do mesmo.
	*/
	static Vetor3D ponto_medio(
		const Vetor3D v1,
		const Vetor3D v2
	) {
		return (v1 + v2) / 2;
	}
};


class Linha   {
	/*
	Descri��o:
		Classe respons�vel por represetar um segmento de linha a partir de 
		2 vetores tridimensionais.
		
	Cr�ditos: Miguel Abreu (m.abreu@fe.up.pt)
	*/
	
public:
	
	// Para coordenadas esf�ricas.
	/*
	Para coordenadas esf�ricas. Basta ver em Geometria.h e no original
	Vector3f.cpp que ele se refere a vetores com raio, phi e theta. Mas em
	polares � para 2D, para 3D nos referimos a esfericas.
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
	/// M�todos de C�lculo Espec�fico
	//////////////////////////////////////////////////////////////////////
	
	/*
	Aten��o, existem fun��es que recebem par�metros em esf�ricas e 
	retornam pontos em cartesianas.
	*/
	
	Vetor3D ponto_na_reta_mais_perto_cart(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
		A partir de um ponto qualquer, achar� o ponto na RETA INFINITA
		definida pela linha, this.
		
		�lgebra Linear pura:
			
			Q -> Ponto Qualquer
			P = A + t(B - A) -> Linha
			
			Por defini��o, o mais perto � perpendicular:
			
			< Q - P , P - A > = 0
			< Q - A - t(B - A) , t(B - A) > = 0
			t< Q - A, B - A > - t^2 < B - A, B - A > = 0
			
			t = < Q - A, B - A > / < B - A, B - A >
			
			Por fim, basta substituir na linha.
		*/

		float param = (ponto_qualquer_cart - inicio_c).InnerProduct(final_c - inicio_c) / (comprimento * comprimento);	
		
		return inicio_c + (final_c - inicio_c) * param;
	}
	Vetor3D ponto_na_reta_mais_perto_esf(
		Vetor3D& ponto_qualquer_esf
	) const {
		
		// Note que estamos recebendo em esf e retornaremos em cartesiana!!!
		
		return ponto_na_reta_mais_perto_cart(ponto_qualquer_esf.para_cartesiano());
	}
	
	
	float distancia_ate_ponto_cart(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
		Dist�ncia de um ponto a uma linha. 
		
		Suponha B - A o segmento de linha pertencente � linha e 
		Q o ponto qualquer.
		
		Note que:
		
		2 * Area = |(Q - inicio_c) X (Q - final_c)| = Base Altura
		
		Altura = |(Q - inicio_c) X (Q - final_c)| / |final_c - inicio_c|
		*/
		
		return ((ponto_qualquer_cart - inicio_c).CrossProduct(ponto_qualquer_cart - final_c)).modulo() / comprimento;
	}
	float distancia_ate_ponto_esf(
		const Vetor3D ponto_qualquer_esf
	) const {
		return distancia_ate_ponto_cart(ponto_qualquer_esf.para_cartesiano());
	}
	
	
	float distancia_ate_linha(
		const Linha& linha
	) const {
		/*
		Suponha duas retas definidas pelas linhas, (*this) e a dada.
		De tal forma que:
		
		P = A + t(B - A)       		Q = N + r(M - N)
		
		S�o pontos quaisquer respectivamente nas mesmas. Se desejamos a
		dist�ncia entre essas linhas, devemos buscar P - Q de tal forma
		que essa linha seja perpendendicar � B - A e � M - N simutalneamente.
		
		< P - Q, B - A > = 0
		< P - Q, M - N > = 0
		
		Em seguida, manipular algebricamente:
		
		t< B - A, B - A > - r< M - N, B - A > = < N - A, B - A >
		t< B - A, M - N > - r< M - N, M - N > = < N - A, M - N >
		
		Observe que temos um sistema linear. H� tr�s op��es.
		
		i) N�o h� solu��o.
			
			Como estamos dimens�es no m�ximo tridimensionais, sempre haver�
			pelo menos uma solu��o.
		
		ii) Infinitas solu��es.
			
			Caso em que as retas est�o paralelas. H� infinitos pontos que
			satisfazem a perpendicularidade.
			
			Basta que o determinante da matriz dos coeficientes seja zero.
			
			Ent�o basta escolher um ponto da linha dada como fixo, N por exemplo,
			e obtermos a dist�ncia � esse ponto. Neste caso, a solu��o � trivial.
		
		iii) Uma �nica solu��o.
		
			Para facilitar, podemos denotar os coeficientes como par�metros e
			assim resolver o sistema 2x2.
		*/

		float prod_esc_entre_vet_diretores = (linha.final_c - linha.inicio_c).InnerProduct((*this).final_c - (*this).inicio_c);
		
		float Det = prod_esc_entre_vet_diretores * prod_esc_entre_vet_diretores - (*this).comprimento * (*this).comprimento * linha.comprimento * linha.comprimento; 
		
		if(
			// Caso sejam paralelas ou pelo menos quase paralelas.
			fabs(Det) < 1e-5
		){

			return (*this).distancia_ate_ponto_cart(linha.inicio_c);
		}
		
		/*
		Produto escalar entre posi��o relativa do segmento, N - A, e
		vetor diretor da nossa linha, B - A.
		*/
		float g_this = (linha.inicio_c - (*this).inicio_c).InnerProduct((*this).final_c - (*this).inicio_c);
		/*
		Produto escalar entre posi��o relativa do segmento, N - A, e
		vetor diretor da linha dada, M - N.
		*/
		float g_other = (linha.inicio_c - (*this).inicio_c).InnerProduct(linha.final_c - linha.inicio_c);
	
		// Respectivos Par�metros:
		float param_t = (
			g_other * prod_esc_entre_vet_diretores - g_this * linha.comprimento * linha.comprimento
		) / Det;
		
		float param_r = (fabs(prod_esc_entre_vet_diretores) < 1e-5) ? - g_other / (linha.comprimento * linha.comprimento) : (
			param_t * (*this).comprimento * (*this).comprimento - g_this
		) / prod_esc_entre_vet_diretores;
		
		// Respectivos pontos de dist�ncia m�nima.
		Vetor3D P = inicio_c + (final_c - inicio_c) * param_t;
		Vetor3D Q = linha.inicio_c + (linha.final_c - linha.inicio_c) * param_r;
		
		return P.obter_distancia(Q);
	}	
	
	
	//////////////////////////////////////////////////////////////////
	
	
	Vetor3D segment_ponto_na_reta_mais_perto_cart(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
		Basicamente, mesma fun��o que a anterior, entretanto, agora limitamos
		os pontos poss�veis para o segmento de reta definido por inicio_c e 
		final_c.
		*/
		
		float param = (ponto_qualquer_cart - inicio_c).InnerProduct(final_c - inicio_c) / (comprimento * comprimento);	
		
		// Ambas condi��es nunca ser�o verdadeiras ao mesmo tempo.
		param += (param < 0) * ( - param ) + (param > 1) * (1 - param); // Simplesmente goat
		
		return inicio_c + (final_c - inicio_c) * param;
	}
	Vetor3D segment_ponto_na_reta_mais_perto_esf(
		const Vetor3D& ponto_qualquer_esf
	) const {
		
		return segment_ponto_na_reta_mais_perto_cart(ponto_qualquer_esf.para_cartesiano());
	}
	
	
	float segment_distancia_ate_ponto_cart(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
		Mesma fun��o que a nome semelhante, entretanto, n�o pensaremos mais
		na reta infinita, e sim no segmento de reta.
		
		Imagine um ponto qualquer Q e um SEGMENTO DE RETA definido por B - A.
		
				  .B
		Q.       /
		       /
			A.
		
		Caso Q esteja para "tr�s" de A, a dist�ncia |Q - A| representa a dist�ncia
		m�nima de Q ao segmento.
		
		Caso Q esteja para "frente" de B, a dist�ncia |Q - B| representa a dist�ncia
		m�nima de Q ao segmento.
		
		Caso Q esteja na regi�o entre A e B, teremos o tri�ngulo.
		*/
		
		if(	
			(ponto_qualquer_cart - (*this).inicio_c).InnerProduct((*this).final_c - (*this).inicio_c) <= 0
		){
			
			return (*this).inicio_c.obter_distancia(ponto_qualquer_cart);
		}
		
		if(
			(ponto_qualquer_cart - (*this).final_c).InnerProduct((*this).final_c - (*this).inicio_c) >= 0
		){
			
			return (*this).final_c.obter_distancia(ponto_qualquer_cart);
		}
		
		return ((ponto_qualquer_cart - inicio_c).CrossProduct(ponto_qualquer_cart - final_c)).modulo() / comprimento;
	}
	float segment_distancia_ate_ponto_esf(
		const Vetor3D& ponto_qualquer_esf
	) const {
		return segment_distancia_ate_ponto_cart(ponto_qualquer_esf.para_cartesiano());
	}

	
	float segment_distancia_ate_segment(
		const Linha& linha
	) const {
		/*
		Obter dist�ncia entre dois segmentos de reta definidos respectivamente
		por B - A e M - N.
		
		Analisando o algoritmo da fun��o imediatamente anterior e a ideia do
		que esta fun��o se prop�e, podemos reutilizar o algoritmo das retas 
		infinitas e verificar os valores dos par�metros.
		
		Caso N�o Sejam Parelalos:
		
		Trivialmente reutilizaremos o algoritmo da fun��o semelhante
		e analisaremos se os par�metros s�o menores que 0 ou maiores
		que 1.
		
		Caso sejam paralelos:
		
		M.
		
		
		
		N.
		
				.B
				
				.A
		
		Observe que se dependendo se M - N est� "� frente" ou "� tr�s"
		temos configura��es diferentes de dist�ncias.
		
		Caso B n�o esteja esteja para tr�s de N e A n�o esteja para 
		frente de M, teremos:
		
		M.
			.B
		
			.A
		N.
		
		Neste caso, teremos um trap�zio.
		
		*/
		
		float prod_esc_entre_vet_diretores = (linha.final_c - linha.inicio_c).InnerProduct((*this).final_c - (*this).inicio_c);
		
		float Det = prod_esc_entre_vet_diretores * prod_esc_entre_vet_diretores - (*this).comprimento * (*this).comprimento * linha.comprimento * linha.comprimento; 
		
		if(
			// Caso sejam paralelas ou pelo menos quase paralelas.
			fabs(Det) < 1e-3
		){
			
			if(
				/*
				Se B estiver para tr�s de N, a dist�ncia ser� |B - N|.
				*/
				((*this).final_c - linha.inicio_c).InnerProduct(linha.final_c - linha.inicio_c) <= 0
			){
				
				return (*this).final_c.obter_distancia(linha.inicio_c);
			}
			
			if(
				/*
				Se A estiver para frente de M, a dist�ncia ser� |A - M|.
				*/
				((*this).inicio_c - linha.final_c).InnerProduct(linha.final_c - linha.inicio_c) >= 1
			){
				
				return (*this).inicio_c.obter_distancia(linha.final_c);
			}
			
			/*
			Obter altura a partir da �rea do trap�zio formado pelas linhas.
			*/
			
			float semi_triang_1 = (linha.inicio_c - (*this).inicio_c).CrossProduct((*this).final_c - (*this).inicio_c).modulo();
			float semi_triang_2 = (linha.inicio_c -    linha.final_c).CrossProduct((*this).final_c -   linha.inicio_c).modulo();
			
			return (semi_triang_1 + semi_triang_2) / (linha.comprimento + (*this).comprimento);
		}
		
		/*
		Produto escalar entre posi��o relativa do segmento, N - A, e
		vetor diretor da nossa linha, B - A.
		*/
		float g_this = (linha.inicio_c - (*this).inicio_c).InnerProduct((*this).final_c - (*this).inicio_c);
		/*
		Produto escalar entre posi��o relativa do segmento, N - A, e
		vetor diretor da linha dada, M - N.
		*/
		float g_other = (linha.inicio_c - (*this).inicio_c).InnerProduct(linha.final_c - linha.inicio_c);
	
		// Respectivos Par�metros:
		float param_t = (
			g_other * prod_esc_entre_vet_diretores - g_this * linha.comprimento * linha.comprimento
		) / Det;
		
		float param_r = (fabs(prod_esc_entre_vet_diretores) < 1e-5) ? - g_other / (linha.comprimento * linha.comprimento) : (
			param_t * (*this).comprimento * (*this).comprimento - g_this
		) / prod_esc_entre_vet_diretores;
		
		// Corre��o para Segmentos de Reta.
		param_t += (param_t < 0) * (- param_t) + (param_t > 1) * (1 - param_t);
		
		param_r += (param_r < 0) * (- param_r) + (param_r > 1) * (1 - param_r);
		
		// Respectivos pontos de dist�ncia m�nima.
		Vetor3D P = inicio_c + (final_c - inicio_c) * param_t;
		Vetor3D Q = linha.inicio_c + (linha.final_c - linha.inicio_c) * param_r;
		
		return P.obter_distancia(Q);
	}
	
	//////////////////////////////////////////////////////////////////////
	/// Operador de Acesso
	//////////////////////////////////////////////////////////////////////
	
	
	Vetor3D obter_ponto_medio_cart() const {
		return (inicio_c + final_c) / 2;
	}
	Vetor3D obter_ponto_medio_esf() const {
		return (*this).obter_ponto_medio_cart().para_esferica();
	}
	
	const Vetor3D& obter_ponto_cart(const int index) const {
		/*
		O const no inicio e no final garantem que:
		
		O objeto Linha n�o seja modificado e os membros retornados
		n�o sejam modificados.
		*/
		
		if(
			index == 0
		){
			
			return inicio_c;
		}
		
		return final_c;
	}
	
	const Vetor3D& obter_ponto_esf(const int index) const {
		if(
			index == 0
		){
			
			return inicio_e;
		}
		
		return final_e;
	}
	
	Vetor3D obter_diretor_cart() const {
		return final_c - inicio_c;
	}
	
	Vetor3D obter_diretor_esf() const {
		return final_e - inicio_e;
	}
	
	//////////////////////////////////////////////////////////////////////
	/// Operador de Condi��es
	//////////////////////////////////////////////////////////////////////
	
	bool operator==(const Linha& outra_linha) const {
		/*
		O sistema nos fornecer� em esf�ricas. Acredito que seja melhor assim
		para evitarmos erros de conta devido � opera��es, oq poderia tornar 
		valores um pouco diferentes.
		*/
		return (inicio_e == outra_linha.inicio_e) && (final_e == outra_linha.final_e);
	}
};


class Matriz  {
	/*
	Descri��o:
		Classe respons�vel por representar matrizes em geral, em especial
		matrizes 4x4 e providenciar ferramentas necess�rias para
		transforma��es de transla��o, rota��o e escalamento.
		
		Apesar de ser uma matriz, usaremos um vetor para represent�-la.
				
		Este c�digo tratar� de uma matem�tica mais profunda. Para isso:
		
		- Sugiro que verifique o seguinte link para melhor embasamento: 
	    
		Site explicando sobre essas transforma��es em matrizes 4x4:
			https://www.brainvoyager.com/bv/doc/UsersGuide/CoordsAndTransforms/SpatialTransformationMatrices.html
		
		- V�deo no Youtube explicando sobre porque utilizamos 4x4:
			https://youtu.be/Do_vEjd6gF0
		
			Resposta: 
				Matriz 3x3 conseguem representar perfeitamente transforma��es 
				lineares como rota��o e escalamento. Entretanto, n�o conseguem
				representar transforma��es n�o lineares, como TRANSLA��O.
				Portanto, usamos 4x4 que conseguem ambos.
		
	Testes:
		Foi criado um arquivo .cpp que executou cada uma das aplica��es
		implementas aqui. Caso novas atualiza��es sejam providenciadas, sugiro o mesmo.
		
	Cr�ditos:
	- @author Nuno Almeida (nuno.alm@ua.pt)
   	- Adapted - Miguel Abreu
	*/
public:
	
	float conteudo[M_TAMANHO]; 
	
	///////////////////////////////////////////////////////////////////////
	/// Construtores e Destrutores
	///////////////////////////////////////////////////////////////////////
	
	Matriz() {
		/*
		Vamos retornar uma esp�cie de matriz identidade. Digo esp�cie
		porque n�o necessariamente ser� quadrada, depender� se 
		M_LINHAS = M_COLUNAS.
		
		Suponha M_LINHAS e M_COLUNAS, n�o necessariamente iguais.
		
		Imagine um loop percorrendo pelo index:
		
		  0,           1,             2, ..., M_C - 1
		M_C,     M_C + 1,       M_C + 2, ..., 2 * M_C - 1
	2 * M_C, 2 * M_C + 1, 2 * (M_C + 1), ...
		
		Observe que o termo que recebe 1 necessariamente � um m�ltiplo
		da quantidade M_COLUNAS + 1.
		
		Sendo assim, temos nosso algoritmo.
		*/
		
		for(
			int index = 0;
			index < M_TAMANHO;
			index++
		){
			
			conteudo[ index ] = index % (M_COLUNAS + 1) == 0;
		}
	}
	
	
	Matriz(
		const float entradas[]
	) {
		/*
		Contruir matriz baseado no valores presentes neste vetor de entrada.
		
		A matriz receber� apenas at� que a quantidade dispon�vel se estabele�a.
		*/
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			conteudo[i] = entradas[i];
		}
	}
	
	
	Matriz(
		const Matriz& outra_matriz
	) {
		/*
		Construir matriz baseado em outra.
		*/
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			conteudo[i] = outra_matriz.conteudo[i];
		}
	}
	
	// Matriz de Transla��o
	Matriz(
		const Vetor3D& vetor_3d
	) {
		/*
		ATEN��O: Matriz de Transla��o!
		
		Criar� uma matriz, neste caso de 4 dimens�es, a partir de um
		vetor (tx, ty, tz).
		
		O que essa matriz ser� capaz de fazer:
		
		(Matriz)(vx, vy, vz, 1)^T = (vx + tx, vy + ty, vz + tz, 1)^T
		*/
		
		float vetor_4d[] = {
			vetor_3d.x,
			vetor_3d.y,
			vetor_3d.z,
			1
		};
		int indicador_de_componente = 0;  // Temos garantio que ser� 0, 1, ou 2.
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			conteudo[i] = ( 
			
				// Condi��o: Se n�o estiver na �ltima coluna.
				i != (M_COLUNAS * (indicador_de_componente + 1) - 1)
				
			) ? (
			
				// Se verdadeiro, preenchemos como se fosse identidade.
				i % (M_COLUNAS + 1) == 0
				
			) : (
			
				// Se false, preenchemos com a componente.
				vetor_4d[indicador_de_componente++]
				
			);
		}
	}
	
	
	// Outra forma de matriz de transla��o
	static Matriz obter_matriz_de_translacao(
		float tx,
		float ty,
		float tz
	) {

		const float temp[M_TAMANHO] = {
								        1, 0, 0, tx,
								        0, 1, 0, ty,
								        0, 0, 1, tz,
								        0, 0, 0,  1
								   	  };
								   	  
		return Matriz(temp);
	}
	
    ~Matriz() {}
	
	///////////////////////////////////////////////////////////////////////
	/// Manipuladores
	///////////////////////////////////////////////////////////////////////
	
	void setar(
		unsigned int index_a_ser_setado,
		float valor_a_ser_setado
	) {
		/*
		Caso esteja sendo manipulado como um vetor.
		*/
		conteudo[index_a_ser_setado] = valor_a_ser_setado;
	}
	void setar(
		unsigned int linha_a_ser_setada,
		unsigned int coluna_a_ser_setada,
		float valor_a_ser_setado
	) {
		/*
		Caso esteja sendo manipulado como matriz.
		*/
		
		conteudo[
			M_COLUNAS * linha_a_ser_setada + coluna_a_ser_setada
		] = valor_a_ser_setado;
	}
	
	
	float obter(
		unsigned int index_a_ser_obtido
	) {
		/*
		Caso esteja sendo manipulado como um vetor.
		*/
		return conteudo[index_a_ser_obtido];
	}
	float obter(
		unsigned int linha_a_ser_obtido,
		unsigned int coluna_a_ser_obtido
	) {
		/*
		Caso esteja sendo manipulado como matriz.
		*/
		
		return conteudo[
			M_COLUNAS * linha_a_ser_obtido + coluna_a_ser_obtido
		];
	}
	
	
	///////////////////////////////////////////////////////////////////////
	/// M�todos de Aritm�tica
	///////////////////////////////////////////////////////////////////////
	
	
	Matriz operator+(
		const Matriz& outra_matriz
	) const {
		/*
		Descri��o:
			Gerar� nova matriz soma.
		*/
		
		float temp[M_TAMANHO];
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			temp[i] = (*this).conteudo[i] + outra_matriz.conteudo[i];
		}
		
		return Matriz(temp);
	}
	
	
	Matriz operator-(
		const Matriz& outra_matriz
	) {
		/*
		Gerar� matriz diferen�a.
		*/
		
		float temp[M_TAMANHO];
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			temp[i] = (*this).conteudo[i] - outra_matriz.conteudo[i];
		}
		
		return Matriz(temp);
	}
	
	
	/*
	Multiplica��o de Matrizes
	*/
	Matriz operator*(
		const Matriz& outra_matriz
	) const {

		float temp[M_TAMANHO];
		
		for(
			int linha = 0;
			linha < M_LINHAS;
			linha++
		){
			for(
				int coluna = 0;
				coluna < M_COLUNAS;
				coluna++
			){
				// Setamos como se fosse matriz nula.
				temp[M_COLUNAS * linha + coluna] = 0;
				
				// Construimos a soma para cada entrada.
				for(
					int lin_col_ja_somada = 0;
					lin_col_ja_somada < M_COLUNAS;
					lin_col_ja_somada++
				){
					
					temp[
						M_COLUNAS * linha + coluna
					] += (*this).conteudo[
						M_COLUNAS * linha  + lin_col_ja_somada
					] * outra_matriz.conteudo[
						M_COLUNAS * coluna + lin_col_ja_somada
					];
				}
				
			}
		}
		
		return Matriz(temp);
	}
	
	
	/*
	Multiplica��o de Matriz por escalar
	*/
	Matriz operator*(
		const float escalar
	) const {
		
		float temp[M_TAMANHO];
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			temp[i] = conteudo[i] * escalar;
		}
		
		return Matriz(temp);
	}
	
	
	/*
	Multiplica��o de Matriz de Transla��o por Vetor Quadridimensional
	*/
	Vetor3D operator*(
		const Vetor3D& vetor
	) const {
		/*
		Aplica��o do algoritmo da matriz de transla��o.
		Verifique:
		https://www.brainvoyager.com/bv/doc/UsersGuide/CoordsAndTransforms/SpatialTransformationMatrices.html
		
		Foi um aprimoramento gigante no algoritmo, experimente ver como 
		est� no original.
		
		N�o � exatamente garantido que a submatriz do lado esquerdo
		seja a identidade 3x3.
		
		*/
		
		float temp[3] = {0, 0, 0};
		
		int index = 0;
		for(
			// Iterar sobre elementos da matriz
			int i = 0;
			i < 12;  // garantia de que � 4x4
			i++
		){
			if(
				i > 4 * (index + 1)
			){
				index++;
			}
			
			temp[index] += conteudo[ i ] * ( (i + 1) % 4 != 0 ? vetor[index] : 1);
		}
		
		return Vetor3D(
						temp[0], 
						temp[1], 
						temp[2]
					  );
	}

	
	///////////////////////////////////////////////////////////////////////
	/// M�todos de Atribui��o
	///////////////////////////////////////////////////////////////////////
	
	
	void operator=(
		const Matriz& outra_matriz
	){
		/*
		Assimilar� outra matriz � esta.
		*/
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			(*this).conteudo[i] = outra_matriz.conteudo[i];
		}
	}

	
	bool operator==(
		const Matriz& outra_matriz
	){
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			if(
				(*this).conteudo[ i ] != outra_matriz.conteudo[ i ]
			){
				return false;
			}
		}
		
		return true;		
	}


	void operator+=(
		const Matriz& outra_matriz
	){
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			(*this).conteudo[ i ] += outra_matriz.conteudo[ i ];
		}
	}
	
	
	void operator-=(
		const Matriz& outra_matriz
	){
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			(*this).conteudo[ i ] -= outra_matriz.conteudo[ i ];
		}
	}

	
	float& operator[](
		const unsigned int index
	){
		/*
		Retornamos uma refer�ncia para permitir posterior altera��o do valor.
		Al�m disso, � mais eficiente, dado que n�o cria uma c�pia.
		*/
		return conteudo[index];
	}
	
	
	///////////////////////////////////////////////////////////////////////
	/// M�todos de Opera��es Espec�ficas
	///////////////////////////////////////////////////////////////////////
	
	
	Vetor3D obter_vetor_de_translacao() const {
		
		return Vetor3D(
						/*
						Garantia que � 4D.
						*/
						conteudo[ 3 ],
						conteudo[ 7 ],
						conteudo[ 11 ]
					  );
	}

	
	Matriz transposta() {
		// Devemos gerar uma nova matriz transposta.
		
		Matriz transp;
		
		for(
			int linha = 0;
			linha < M_LINHAS;
			linha++
		){
			
			for(
				int coluna = 0;
				coluna < M_COLUNAS;
				coluna++
			){
				
				transp.setar(
							coluna, 
							linha,
							(*this).obter(
										 linha,
										 coluna
									  )
							);
			}
		}
		
		return transp;
	}

	
	Matriz obter_matriz_de_transformacao_inversa() const {
		/*
		Suponha que eu tenha uma matriz que representa uma Transforma��o
		Linear, basta verificar novamente o estilo que estamos manipulando:
		https://www.brainvoyager.com/bv/doc/UsersGuide/CoordsAndTransforms/SpatialTransformationMatrices.html
		
		Eu desejo obter a matriz que representa a Transforma��o Inversa da mesma.
		Para quem olhou no site, desejamos a matriz inversa da apresentada
		com as entradas em a_ij.
		*/
		
		Matriz inv;  // Temos garantia que this � 4x4.
		
		// Transposta da SubMatriz 3x3
		int sub_index = 0;
		int index     = 0;
		while(
			index < 11
		){
			if(
				(index + 1) % 4 == 0
			){
				index++;
				sub_index++;
			}
			
			inv[
				4 * ( index % 4 ) + sub_index
			] = conteudo[
				index
			];
			
			index++;
		}
		
		// Transla��o de Valores
		index     = 0;
		while(
			index < 3
		){
			
			inv[
				4 * (index + 1) - 1
			] = (
				- conteudo[     index ] * conteudo[ 3  ]
			) + (
				- conteudo[ 4 + index ] * conteudo[ 7  ]
			) + (
				- conteudo[ 8 + index ] * conteudo[ 11 ]
			);
			
			index++;
		}
		
		return inv;
	}

	bool obter_inversa(
		Matriz& matriz_que_sera_inversa 
	) const {
		/*
		Obter� os coeficientes da inversa de uma matriz 4x4 qualquer.
		Al�m disso, verifica se o determinante desta matriz � 0:
		
		Retorna false caso o det seja 0, isto �, n�o existe inversa.
		Retorna true caso o det n�o seja 0 e teremos a matriz.
		
		N�o vou me atrever a alterar esse c�digo e buscar um algoritmo.
		(m.abreu@2020)
		*/
		
		float inv[16], det;
		// Apenas para garantirmos que n�o modificaremos o conte�do
	    const float* m = conteudo; 
	    
	    inv[ 0  ] =   m[5] * m[ 10 ] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	    inv[ 4  ] = - m[4] * m[ 10 ] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	    inv[ 8  ] =   m[4] * m[ 9  ] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9 ];
	    inv[ 12 ] = - m[4] * m[ 9  ] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9 ];
	    inv[ 1  ] = - m[1] * m[ 10 ] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	    inv[ 5  ] =   m[0] * m[ 10 ] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	    inv[ 9  ] = - m[0] * m[ 9  ] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9 ];
	    inv[ 13 ] =   m[0] * m[ 9  ] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9 ];
	    inv[ 2  ] =   m[1] * m[ 6  ] * m[15] - m[1] * m[7 ] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7 ] - m[13] * m[3] * m[6 ];
	    inv[ 6  ] = - m[0] * m[ 6  ] * m[15] + m[0] * m[7 ] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7 ] + m[12] * m[3] * m[6 ];
	    inv[ 10 ] =   m[0] * m[ 5  ] * m[15] - m[0] * m[7 ] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7 ] - m[12] * m[3] * m[5 ];
	    inv[ 14 ] = - m[0] * m[ 5  ] * m[14] + m[0] * m[6 ] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6 ] + m[12] * m[2] * m[5 ];
	    inv[ 3  ] = - m[1] * m[ 6  ] * m[11] + m[1] * m[7 ] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9 ] * m[2] * m[7 ] + m[9 ] * m[3] * m[6 ];
	    inv[ 7  ] =   m[0] * m[ 6  ] * m[11] - m[0] * m[7 ] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8 ] * m[2] * m[7 ] - m[8 ] * m[3] * m[6 ];
	    inv[ 11 ] = - m[0] * m[ 5  ] * m[11] + m[0] * m[7 ] * m[9 ] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9 ] - m[8 ] * m[1] * m[7 ] + m[8 ] * m[3] * m[5 ];
	    inv[ 15 ] =   m[0] * m[ 5  ] * m[10] - m[0] * m[6 ] * m[9 ] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9 ] + m[8 ] * m[1] * m[6 ] - m[8 ] * m[2] * m[5 ];
	
	    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	    
		if (det == 0){
	    	
	    	return false;
		}
	
	    det = 1.0 / det;
	
	    for( 
			int i = 0;
			i < 16;
			i++
		){
			
			matriz_que_sera_inversa.setar(i, inv[i] * det);
		}
	
	    return true;
	}
	
	
	/*
	Por que escrevemos tr�s matrizes e n�o apenas uma?
		A multiplica��o dessas matrizes 
		N�O S�O COMUTATIVAS!
		
		Por exemplo, fazer uma rota��o de X e em seguida em Y, n�o resultar�
		na mesma coisa que fazer primeiro em Y e dps em X.
	*/
	static Matriz matriz_de_rotacao_x(
		float angle
	){
		/*
		Providencia a matriz que representa a rota��o do eixo x
		neste �ngulo em degraus.
		*/
		const float temp[M_TAMANHO] = {
			1,              0,                0,      0,
        	0, cos_deg(angle), - sin_deg(angle),      0,
        	0, sin_deg(angle),   cos_deg(angle),      0,
        	0,              0,                0,      1
		};
		
		return Matriz(temp);
	}
	
	
	static Matriz matriz_de_rotacao_y(
		float angle
	){
		const float temp[M_TAMANHO] = {
        cos_deg(angle),     0,   sin_deg(angle),      0,
        			 0,     1,                0,      0,
      - sin_deg(angle),     0,   cos_deg(angle),      0,
                     0,     0,  	          0,      1
    	};
		
		return Matriz(temp);
	}


	static Matriz matriz_de_rotacao_z(
		float angle
	){
		const float temp[M_TAMANHO] = {
        cos_deg(angle), - sin_deg(angle),     0,      0,
        sin_deg(angle),   cos_deg(angle),     0,      0,
                     0,                0,     1,      0,
                     0,                0,     0,      1
    	};
		
		return Matriz(temp);
	}
	
	
	static Matriz matriz_de_rotacao_em_torno_de_eixo(
		Vetor3D eixo_representado_como_vetor_unitario,
		float angle
	){
		/*
		Obter matriz de rota��o em torno de um eixo espec�fico.
		Este vetor TEM QUE SER UNIT�RIO.
		*/
		
		float x = eixo_representado_como_vetor_unitario.x;
		float y = eixo_representado_como_vetor_unitario.y;
		float z = eixo_representado_como_vetor_unitario.z;
		
		const float temp[M_TAMANHO] = {
	(x * x * (1 - cos_deg(angle)) +     cos_deg(angle)), (x * y * (1 - cos_deg(angle)) - z * sin_deg(angle)), (x * z * (1 - cos_deg(angle)) + y * sin_deg(angle)), 0,
	(x * y * (1 - cos_deg(angle)) + z * sin_deg(angle)), (y * y * (1 - cos_deg(angle)) +     cos_deg(angle)), (y * z * (1 - cos_deg(angle)) - x * sin_deg(angle)), 0,
	(x * z * (1 - cos_deg(angle)) - y * sin_deg(angle)), (y * z * (1 - cos_deg(angle)) + x * sin_deg(angle)), (z * z * (1 - cos_deg(angle)) +     cos_deg(angle)), 0,
													  0,                                                   0,                                                   0, 1
	    };
	    
	    return Matriz(temp);
	}
};

#endif // GEOMETRIA_H
