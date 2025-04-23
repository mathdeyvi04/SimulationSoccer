/*
Matheus Deyvisson, 2025

Utilizei apenas um arquivo header a fim de evitar a repetição 
desnecessária de vários arquivos.

Inclui os arquivos:
    -> Vector3f.cpp
    -> Vector3f.h
    -> Geometry.cpp
    -> Geometry.h

Fiz algumas modificações para aprimorar legibilidade.
Testes já foram realizados.
*/
#ifndef GEOMETRIA_H
#define GEOMETRIA_H

#include <cmath>
#include <math.h>
#include <algorithm>
#include <stdio.h>

using namespace std;

#define EPSILON 1e-6  // No Original, era 1e-10, o que eu julgo desnecessariamente pequeno..
#define PI 3.1415926535f

/*
Funções que serão amplamente utilizadas
*/

inline float cos_deg(
	float ang_degraus
){
	/*
	Descrição:
		A função base da biblioteca somente aceita radianos.
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
	Descrição:
		Calcula o arco cuja tangente é img / real, em degraus.
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
	Descrição:
		Representa a posição no espaço 2D.
		
		Uma posição será representada em coordenadas
		cartesianas, (x, y).
		
		Créditos para o Alto Nível de Trabalho:
		- @author Hugo Picado (hugopicado@ua.pt)
		- @author Nuno Almeida (nuno.alm@ua.pt)
		- Adapted - Miguel Abreu
	*/
	
	float x;
	float y;
	
	Vetor2D(
		/*
		Iniciamos um vetor com componentes x e y.
		Caso nenhum valor seja passado, assumirá 0.
		*/
		float valor_x,
		float valor_y
	) : x(valor_x), y(valor_y) {}
	
	static void verificar(const Vetor2D& vetor){
		printf("(%f, %f)", vetor.x, vetor.y);
	}
	
	////////////////////////////////////////////////////////////////////////
	/// Operadores de Aritmética
	////////////////////////////////////////////////////////////////////////
	
	/*
	Observe que passaremos argumentos por referência! Como se fossem
	ponteiros.
	
	O const no final indica que não estaremos modificando o vetor dado, 
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
		Soma de outros vetores é a soma de suas componentes.
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
		Não é produto escalar pois gerará novo vetor.
		*/
		return Vetor2D( x * outro_vetor.x, y * outro_vetor.y);
	}
	
	
	Vetor2D operator/( const float   &escalar     ) const {
		/*
		Assim como no produto com escalar, aqui faremos divisão com escalar.
		*/
		return Vetor2D( x / escalar, y / escalar );
	}
	Vetor2D operator/( const Vetor2D &outro_vetor ) const {
		/*
		Divisão de componente por componente.
		*/
		return Vetor2D( x / outro_vetor.x, y / outro_vetor.y);
	}
	
	////////////////////////////////////////////////////////////////////////
	/// Operadores de Atribuição
	////////////////////////////////////////////////////////////////////////
	
	/*
	Os métodos a seguir alteram o seu próprio valor.
	Não criam novos.
	*/
	
	// Vetor2D vetor = 1 -> Vector2D( 1, 1 )
	void operator =( const float   &escalar     ) {
		x = escalar;
		y = escalar;
	}
	
	void operator+=( const float   &escalar     ){
		/*
		Soma o escalar às componentes.
		*/
		x += escalar;
		y += escalar;
	}
	void operator+=( const Vetor2D &outro_vetor ) {
		/*
		Somará as respectivas componentes
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
	/// Métodos
	//////////////////////////////////////////////////////////////////////
	
	// Admito que os nomes em inglês são mais fodas.
	float InnerProduct( const Vetor2D &outro_vetor) const {
		//	Não fará modificações nos vetores.
		return x * outro_vetor.x + y * outro_vetor.x;
	}
	
	float CrossProduct( const Vetor2D outro_vetor ) {
		/*
		Componente da 3° dimensão vetor produto vetorial dos vetores dados.
		*/
		return x * outro_vetor.y - y * outro_vetor.x;
	}
	
	float modulo() const {
		return sqrt(x * x + y * y);
	}
	
	float obter_distancia    ( const Vetor2D outro_vetor ) {
		/*
		Note que é o módulo do vetor diferença.
		*/
		return (*this - outro_vetor).modulo();
	}
};


class Vetor3D {
	/*
	Descrição:
		Representará a posição em 3 valores float.
		
	Créditos:
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
	
	// Definição vazia
	Vetor3D() : x(0), y(0), z(0) {}
	
	
	// Definição usando números como entrada
	Vetor3D(
		float v_x, 
		float v_y,
		float v_z
	) : x(v_x), y(v_y), z(v_z) {}
	
	
	// Definição usando outro vetor, criará uma cópia do mesmo
	Vetor3D(
		const Vetor3D& outro_vetor
	) : x(outro_vetor.x), y(outro_vetor.y), z(outro_vetor.z) {}
	
	
	// Definição usando um vetor 2D, criando um novo vetor e de componente z nula
	Vetor3D(
		const Vetor2D& outro_vetor
	) : x(outro_vetor.x), y(outro_vetor.y), z(0) {}
		
	~Vetor3D() {} // Apenas o destrutor.
	
	static void verificar(const Vetor3D& vetor){
		printf("(%f, %f, %f)", vetor.x, vetor.y, vetor.z);
	}
	
	///////////////////////////////////////////////////////////////////////
	/// Outros Métodos de Definição
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
	/// Operadores de Aritmética
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
	/// Operadores de Atribuição
	//////////////////////////////////////////////////////////////////////
	
	bool operator==( const Vetor3D& outro_vetor ) const {
		return x == outro_vetor.x && y == outro_vetor.y && z == outro_vetor.z;
	}
	
	
	Vetor3D operator+=( const Vetor3D& outro_vetor) {
		x += outro_vetor.x;
		y += outro_vetor.y;
		z += outro_vetor.z;
		
		return *this;  // Garantir a semântica	
	}
	Vetor3D operator+=( float escalar ) {
		x += escalar;
		y += escalar;
		z += escalar;	
		
		return *this;  // Garantir a semântica
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
	/// Métodos de Cálculo
	//////////////////////////////////////////////////////////////////////
	
	float InnerProduct( const Vetor3D& outro_vetor ) const {
		return x * outro_vetor.x + y * outro_vetor.y + z * outro_vetor.z;
	}
	
	
	Vetor3D CrossProduct( const Vetor3D& outro_vetor ) const {
		/*
		Retornará o produto vetorial.
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
		Vamos pegar versor unitário do vetor.
		*/
		return (*this) / (*this).modulo();
	}
	
	
	Vetor3D para_cartesiano() const {
		/*
		Receberemos um vetor em coordenadas esfericas, o qual:
			x -> distancia
			y -> theta, em graus, ângulo horizontal segundo original.
			z -> phi, em graus,   ângulo vertical   segundo original.
		Para transformá-lo em cartesianas.
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
		o levamos para coordenadas esféricas.
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
		Transformará o vetor de 3 dimensões em de 2 dimensões,
		ignorando a terceira.
		*/
		return Vetor2D(
						x,
						y
					  );
	}
	
	/*
	Esse static é como se fosse o staticmethod do Python. Garante
	que não seja necessário uma instância da classe para execução 
	do mesmo.
	*/
	static Vetor3D ponto_medio(
		const Vetor3D v1,
		const Vetor3D v2
	) {
		return (v1 + v2) / 2;
	}
};

#endif // GEOMETRIA_H
