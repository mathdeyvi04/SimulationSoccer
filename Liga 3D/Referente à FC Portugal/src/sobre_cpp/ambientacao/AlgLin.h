/*
Este arquivo aglutina diversos outros:
	-> Vector3f.cpp
    -> Vector3f.h
    -> Vector3f.cpp
    -> Line6f.cpp
    -> Line6f.h
    -> Geometry.cpp
    -> Geometry.h
    -> Matrix4D.cpp
    -> Matrix4D.h

Acredito que seja melhor devido ao aprimoramento da:

- Legibilidade
- Manutenibilidade

Todos os créditos à:
	- @author Hugo Picado (hugopicado@ua.pt)
	- @author Nuno Almeida (nuno.alm@ua.pt)
	- Adapted - Miguel Abreu
*/
#ifndef GEOMETRIA_H
#define GEOMETRIA_H

#include <cmath>
#include <math.h>
#include <algorithm>

using namespace std;

#define EPSILON 1e-10

class Vetor2D {
	/*
	Descrição:
	    Representa uma posição bidimensional (2D) no espaço, podendo ser interpretada
	    tanto em coordenadas cartesianas (x, y) quanto em coordenadas polares (r, phi), 
	    embora internamente armazene os valores nas componentes x e y.

	Métodos:
		- Todos os inerentes à operações básicas e:

	    - float obter_distancia_para(Vector p):
	        Calcula a distância euclidiana até o vetor `p`.

	    - float CrossProduct(Vector p):
	        Retorna a componente vertical do produto vetorial 2D com o vetor `p`.

	    - float length() const:
	        Retorna o comprimento (módulo, ou raio polar) do vetor.

	    - float InnerProduct(const Vector &p) const:
	        Calcula o produto interno (dot product) com o vetor `p`.
	*/
public:
	float x, y;

	Vetor2D( float vx, float vy ) : x(vx),
									y(vy)
									{}

	Vetor2D 
	operator-() const { return Vetor2D( - x, - y ); }
	
	Vetor2D 
	operator-( const float &escalar ) const { return Vetor2D( x - escalar, y - escalar ); }

	Vetor2D
	operator-( const Vetor2D &outro_vetor ) const { return Vetor2D( x - outro_vetor.x, y - outro_vetor.y); }

	Vetor2D
	operator+( const float &escalar ) const { return Vetor2D( x + escalar, y + escalar ); }

	Vetor2D
	operator+( const Vetor2D &outro_vetor ) const { return Vetor2D( x + outro_vetor.x, y + outro_vetor.y ); }

	Vetor2D
	operator*( const Vetor2D &escalar ) const { return Vetor2D( x * escalar, y * escalar); }

	Vetor2D 
	operator/( const Vetor2D &escalar ) const { return Vetor2D( x / escalar, y / escalar ); }

	/*
	Há literalmente duas classes impossíveis:

	Produto Interno com produto e divisão.

	Como isso não faz nem sentido, não reproduzi-las aqui.
	*/

	Vetor2D
	operator=( const float &escalar ) { x = escalar; y = escalar; }

	Vetor2D
	operator+=( const float &escalar ) { x += escalar; y += escalar; }

	Vetor2D 
	operator+=( const Vetor2D &outro_vetor ) { x += outro_vetor.x; y += outro_vetor.y; }

	Vetor2D
	operator-=( const float &escalar ) { x -= escalar; y -= escalar; }
	
	Vetor2D
	operator-=( const Vetor2D &outro_vetor ) { x -= outro_vetor.x; y -= outro_vetor.y; }

	Vetor2D
	operator*=( const float &escalar ) { x *= escalar; y *= escalar; }

	Vetor2D
	operator/=( const float &escalar ) { x /= escalar; y /= escalar; }

	bool
	operator!=( const float &escalar ) { return ( x != escalar ) || ( y != escalar ); }

	bool 
	operator==( const float &escalar ) { return ( x == escalar ) && ( y == escalar ); }

	bool
	operator==( const Vetor2D &outro_vetor ) { return ( x == outro_vetor.x ) && ( y == outro_vetor.y ); }

	//////////////////////////////////////////////////////////////////////////////////////////////

	float obter_modulo() const { return sqrt( x * x + y * y ); }

	// Inglês fica bem melhor
	float CrossProduct( const Vetor2D &other ) { return this->x * other.y - this->y * other.x; }

	float InnerProduct( const Vetor2D &other ) { return this->x * other.x + this->y * other.y; }

	float obter_distancia_para( const Vetor2D &outro_vetor ) { return ( *this - outro_vetor ).obter_modulo(); }
};

class Vetor3D {
	/*
	Descrição:
	    Representa um vetor ou posição tridimensional no espaço 3D. Suporta operações tanto
	    em coordenadas cartesianas (x, y, z) quanto em coordenadas esfericas (r, θ, φ), com
	    funcionalidades para conversão entre essas representações.

	Construtores:
	    - Vetor3D(float x, float y, float z):
	        Inicializa o vetor com as coordenadas fornecidas.

	    - Vetor3D(const Vetor3D& other):
	        Construtor de cópia.

	    - Vetor3D(const Vetor& other):
	        Constrói a partir de um vetor 2D, assumindo z = 0.

	Principais Métodos:
	    - setX(), setY(), setZ(): 
	    	Definem os valores das coordenadas.

	    - operator[]: 
	    	Acesso às coordenadas por índice (0 = x, 1 = y, 2 = z).

	    - normalize(float len = 1): 
	    	Retorna o vetor normalizado com o comprimento desejado.

	    - to_cart(): 
	    	Converte de coordenadas esféricas para cartesianas.

	    - to_esfe(): 
	    	Converte de coordenadas cartesianas para esféricas.

	    - to2d(): 
	    	Extrai o vetor 2D correspondente, ignorando a componente z.

	    - obter_distancia_para(Vetor3D): 
	    	Calcula a distância euclidiana até outro vetor.
		
	    - static obter_ponto_medio(Vetor3D a, Vetor3D b): 
	    	Método estático para calcular o ponto médio entre dois vetores.

	Observações:
	    - As conversões polares assumem que os ângulos estão em graus.
	    - A classe permite acesso direto às coordenadas
	*/
public:
	float x, y, z;

	Vetor3D( float vx, float vy, float vz ) : x(vx),
											  y(vy),
											  z(vy) 
											  {}

	Vetor3D( const Vetor3D &outro_vetor ) : x()



























};























#endif // GEOMETRIA_H


































































