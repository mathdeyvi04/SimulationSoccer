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
#ifndef ALGLIN_H
#define ALGLIN_H

#include <cmath>

using namespace std;

/*
Usaremos o macro unicamente para o método to_esfe()
presente em Vetor3D.
*/
#define EPSILON 1e-10

/*
Para nossa manipulação de matrizes.
*/
#define M_LINHAS  4
#define M_COLUNAS 4
#define M_TAMANHO M_LINHAS * M_COLUNAS

// Usamos para distância entre retas e segmentos de reta
#define MIN_PARA_QUE_SEJAM_PARALELAS 1e-5

class Vetor2D  {
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
	operator-( const float& escalar ) const { return Vetor2D( x - escalar, y - escalar ); }

	Vetor2D
	operator-( const Vetor2D& outro_vetor ) const { return Vetor2D( x - outro_vetor.x, y - outro_vetor.y); }

	Vetor2D
	operator+( const float& escalar ) const { return Vetor2D( x + escalar, y + escalar ); }

	Vetor2D
	operator+( const Vetor2D& outro_vetor ) const { return Vetor2D( x + outro_vetor.x, y + outro_vetor.y ); }

	Vetor2D
	operator*( const float& escalar ) const { return Vetor2D( x * escalar, y * escalar); }

	Vetor2D 
	operator/( const float& escalar ) const { return Vetor2D( x / escalar, y / escalar ); }

	/*
	Há literalmente duas classes impossíveis:

	Produto Interno com produto e divisão.

	Como isso não faz nem sentido, não reproduzi-las aqui.
	*/

	Vetor2D&
	operator=( const float& escalar ) { x = escalar; y = escalar; return *this; }

	Vetor2D&
	operator+=( const float& escalar ) { x += escalar; y += escalar; return *this; }

	Vetor2D&
	operator+=( const Vetor2D& outro_vetor ) { x += outro_vetor.x; y += outro_vetor.y; return *this;}

	Vetor2D&
	operator-=( const float& escalar ) { x -= escalar; y -= escalar; return *this; }
	
	Vetor2D&
	operator-=( const Vetor2D& outro_vetor ) { x -= outro_vetor.x; y -= outro_vetor.y; return *this; }

	Vetor2D&
	operator*=( const float& escalar ) { x *= escalar; y *= escalar; return *this; }

	Vetor2D&
	operator/=( const float& escalar ) { x /= escalar; y /= escalar; return *this; }

	bool
	operator!=( const float& escalar ) { return ( x != escalar ) || ( y != escalar ); }

	bool 
	operator==( const float& escalar ) { return ( x == escalar ) && ( y == escalar ); }

	bool
	operator==( const Vetor2D& outro_vetor ) { return ( x == outro_vetor.x ) && ( y == outro_vetor.y ); }

	//////////////////////////////////////////////////////////////////////////////////////////////

	float 
	obter_modulo() const { return sqrt( x * x + y * y ); }

	// Inglês fica bem melhor
	float 
	CrossProduct( const Vetor2D& other ) { return this->x * other.y - this->y * other.x; }

	float 
	InnerProduct( const Vetor2D& other ) { return this->x * other.x + this->y * other.y; }

	float 
	obter_distancia_para( const Vetor2D& outro_vetor ) { return ( *this - outro_vetor ).obter_modulo(); }
};

class Vetor3D  {
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
	    - set(int, float): 
	    	Seta um valor específico para uma coordenada, baseando-se
	    	no index de entrada.

	    - operator[]: 
	    	Acesso às coordenadas por índice (0 = x, 1 = y, 2 = z).

	    - normalize(float len = 1): 
	    	Retorna o vetor normalizado com o comprimento desejado.

	    - to_cart(): 
	    	Converte de coordenadas esféricas para cartesianas.

	    - to_esfe(): 
	    	Converte de coordenadas cartesianas para esféricas.

	    - to_2d(): 
	    	Extrai o vetor 2D correspondente, ignorando a componente z.

	    - obter_distancia_para(Vetor3D): 
	    	Calcula a distância euclidiana até outro vetor.
		
	    - static obter_ponto_medio(Vetor3D a, Vetor3D b): 
	    	Método estático para calcular o ponto médio entre dois vetores.

	Observações:
	    - As conversões polares assumem que os ângulos estão em graus! ATENÇÃO
	    - A classe permite acesso direto às coordenadas
	*/
public:
	float x, y, z;

	Vetor3D( const float& vx, const float& vy, const float& vz ) : x(vx),
																   y(vy),
																   z(vy) 
																   {}

	Vetor3D( const Vetor3D& outro_vetor ) : x(outro_vetor.x),
											y(outro_vetor.y),
											z(outro_vetor.z)
											{}

	Vetor3D( const Vetor2D& outro_vetor ) : x(outro_vetor.x),
											y(outro_vetor.y),
											z(0)
											{}

	// Acredito que o destrutor possa inútil aqui, dado
	// que somente há alocações estáticas, normalmente.	
	
	Vetor3D 
	operator+( const float& escalar) const { return Vetor3D( x + escalar, y + escalar, z + escalar ); }

	Vetor3D
	operator+( const Vetor3D& outro_vetor ) const { return Vetor3D( x + outro_vetor.x, y + outro_vetor.y, z + outro_vetor.z ); }

	Vetor3D&
	operator+=(const float& escalar ) { x += escalar; y += escalar; z += escalar; return *this; }

	Vetor3D&
	operator+=(const Vetor3D& outro_vetor ) { x += outro_vetor.x; y += outro_vetor.y; z += outro_vetor.z; return *this; }

	Vetor3D
	operator-( const Vetor3D& outro_vetor ) const { return Vetor3D( x - outro_vetor.x, y - outro_vetor.y, z - outro_vetor.z ); }

	Vetor3D 
	operator-( const float& escalar ) const { return Vetor3D( x - escalar, y - escalar, z - escalar ); }

	Vetor3D&
	operator-=(const float& escalar ) { x -= escalar; y -= escalar; z -= escalar; return *this; }

	Vetor3D&
	operator-=(const Vetor3D& outro_vetor ) { x -= outro_vetor.x; y -= outro_vetor.y; z -= outro_vetor.z; return *this; }

	Vetor3D 
	operator*( const float& escalar ) const { return Vetor3D( x * escalar, y * escalar, z * escalar ); }

	Vetor3D
	operator/( const float& escalar ) const { return Vetor3D( x / escalar, y / escalar, z / escalar ); }

	bool
	operator==( const Vetor3D& outro_vetor ) const { return (x == outro_vetor.x) && (y == outro_vetor.y) && (z == outro_vetor.z); }

	// Omitive o operator %, pois achei completamente desnecessário

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float 
	operator[]( const int& index ) const {

		float valor = 0;
		switch (index) {

			case 0:
				valor = x;
				break;
			case 1:
				valor = y;
				break;

			case 2:
				valor = z;
				break;
		}

		return valor;
	}

	void
	setar( const int& index, const float& valor ){

		switch (index) {
			case 0:
				x = valor;
				return;
			case 1:
				y = valor;
				return;
			case 2:
				z = valor;
				return;
		}
	}

	float
	InnerProduct( const Vetor3D& outro_vetor ) const { return this->x * outro_vetor.x + this->y * outro_vetor.y + this->z * outro_vetor.z; }

	Vetor3D
	CrossProduct( const Vetor3D& other ) const {

		return Vetor3D(
						this->y * other.z - this->z * other.y,
						this->z * other.x - this->x * other.z,
						this->x * other.y - this->y * other.x
					  );
	}

	float
	obter_modulo() const { return sqrt(x * x + y * y + z * z); }

	Vetor3D	
	normalize( float& novo_modulo ) const { return (*this) * (novo_modulo / this->obter_modulo()); }

	Vetor3D
	to_cart() const { 
		/*
		Assumimos que:

		x -> distância
		y -> theta(°)
		z -> phi(°)
		*/

		return Vetor3D(
                       x * cos( z * M_PI / 180 ) * cos( y * M_PI / 180 ),
                       x * cos( z * M_PI / 180 ) * sin( y * M_PI / 180 ),
                       x * sin( z * M_PI / 180 )
					  );
	}

	Vetor3D
	to_esfe() const {
		/*
		Assumimos que será:

		x -> distância
		y -> theta(°)
		z -> phi(°)
		*/

		float modulo_do_vetor_em_2d = sqrt(x * x + y * y);

		return Vetor3D(
						this->obter_modulo(),
						(fabs(x) < EPSILON && fabs(y) < EPSILON) ? 0.0 : atan2( y, x ) * 180 / M_PI,
						(fabs(z) < EPSILON && fabs(modulo_do_vetor_em_2d) < EPSILON) ? 0.0 : atan2( z, modulo_do_vetor_em_2d ) * 180 / M_PI
					  );
	}

	float 
	obter_distancia_para( const Vetor3D& outro_vetor ) const { return (*this - outro_vetor).obter_modulo(); }

	Vetor2D
	to_2d() const { return Vetor2D(x, y); }

	static Vetor3D
	obter_ponto_medio( Vetor3D& v1, Vetor3D& v2 ) {

		return ( v1 + v2 ) / 2;
	}
};

class Matriz4D {
	/*
	Descrição:
	    Representa uma matriz 4x4 de ponto flutuante.
	   	Por simplicidade, estamos apenas representando um vetor de 16 
	   	elementos como uma matriz abstrata.

	   	Apesar de possuir um tamanho definido, por prazer, vamos construir
	   	a maioria das finalidades de tal forma que é geral para MxN.

	    Por que fazemos em 4x4?
	    	- Algumas transformações são não lineares, como translação,
	    	e matrizes 3x3 não são capazes de realizar tal fato.

	Construtores:
	    - Matriz4D():
	        Construtor padrão. Inicializa a matriz identidade (sem transformação).

	    - Matriz4D(const float[]):
	        Inicializa a matriz a partir um vetor linear de 16 valores (forma vetorial).

	    - Matriz4D(const Matrix4D& other):
	        Construtor de cópia.

	    - Matriz4D(const Vetor3D& v):
	        Constrói uma matriz de translação a partir de um vetor tridimensional.

	Métodos principais:
	    - setar(i, value): 
	    	Define o valor da posição i (vetorial).
	    - setar(i, j, value): 
	    	Define o valor na posição (i,j) da matriz.
	    - obter(i): 
	    	Retorna o valor da posição i (vetorial).
	    - obter(i, j): 
	    	Retorna o valor na posição (i,j).

	    - operator[]: 
	    	Acesso direto ao valor na posição i do vetor interno.

	    - operator*(Vetor3D): 
	    	Aplica a transformação representada pela matriz em um vetor tridimensional.

	    - obter_transposta(): 
	    	Retorna a matriz transposta.

	    - obter_transformacao_inversa():
	    	Calcula a matriz que gera a transformação inversa.

	    - obter_vetor_de_translacao():
	    	Extrai o vetor de translação contido na matriz.

	Todos os seguintes métodos foram excluídos devido à falta de necessidade.
	    - rotationX(angle)
	    - rotationY(angle)
	    - rotationZ(angle)
	    - rotation(axis, angle)
	    - translation(x, y, z) 
	    - translation(Vector3f v)
	    - operator==
	    - operator+=
	    - operator-=
	    - inverse


	Observações:
	    - Presume-se que os ângulos fornecidos estejam em graus (não radianos).
	*/

	float conteudo[M_TAMANHO];

	Matriz4D() {
		/*
		Imagine um loop percorrendo pelos indexs:

		    0        1        2      ...    M_C - 1  
		  M_C    M_C+1    M_C+2      ...  2*M_C - 1  
		2*M_C  2*M_C+1  2*M_C+2      ...

		Observe que o termo que recebe 1 necessariamente é múltiplo
		da quantidade M_COLUNAS + 1.
		*/

		for(
			int i = 0;
				i < M_TAMANHO;
				i++
		){
			
			this->conteudo[ i ] = i % (M_COLUNAS + 1) == 0;
		}
	}

	Matriz4D(
		const float entradas[M_TAMANHO]
	) {
		// Necessariamente o vetor tem a mesma quantidade de elementos.
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			this->conteudo[i] = entradas[i];
		}
	}

	Matriz4D(
		const Matriz4D& outra_matriz
	) {
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			this->conteudo[i] = outra_matriz.conteudo[i];
		}
	}

	// Criamos a matriz de translação
	Matriz4D(
		const Vetor3D& vetor_de_translacao
	){
		/*
		Eu havia feito um código complicado demais
		Desejo corrigir meu erro aqui!
		*/

		// Construimos identidade
		for(
			int i = 0;
				i < M_TAMANHO;
				i++
		){
			
			this->conteudo[ i ] = i % (M_COLUNAS + 1) == 0;
		}

		// Atribuimos os valores no local correto
		this->conteudo[4 ] = vetor_de_translacao.x;
		this->conteudo[8 ] = vetor_de_translacao.y;
		this->conteudo[12] = vetor_de_translacao.z;
	}

	~Matriz4D() {}

	void 
	setar( const unsigned& index, const float& valor ) { this->conteudo[ index ] = valor; }

	void
	setar( const unsigned& linha, const unsigned& coluna, const float& valor ) { this->conteudo[ M_COLUNAS * linha + coluna ] = valor; }

	float
	obter( const unsigned& index ) const { return this->conteudo[ index ]; }

	float 
	obter( const unsigned& linha, const unsigned& coluna ) const { return this->conteudo[ M_COLUNAS * linha + coluna ]; }

	Matriz4D
	operator+( 
		const Matriz4D& outra_matriz
	) const {

		float temp[M_TAMANHO];
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			temp[i] = this->conteudo[i] + outra_matriz.conteudo[i];
		}
		
		return Matriz4D(temp);
	}

	Matriz4D
	operator-(
		const Matriz4D& outra_matriz
	) const {

		float temp[M_TAMANHO];
		
		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			temp[i] = this->conteudo[i] - outra_matriz.conteudo[i];
		}
		
		return Matriz4D(temp);
	}

	Matriz4D
	operator*(
		const Matriz4D& outra_matriz
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
					int k = 0;
					k < M_COLUNAS;
					k++
				){
					
					temp[
						M_COLUNAS * linha + coluna
					] += this->conteudo[
						M_COLUNAS * linha + k
					] * outra_matriz.conteudo[
						M_COLUNAS * k + coluna
					];
				}
			}
		}

		return Matriz4D(temp);
	}

	// Aplica a transformação em um vetor, gerando um novo.
	Vetor3D 
	operator*(
		const Vetor3D &vetor 
	) const {
		/*
	    Aplicação do algoritmo da matriz de translação.
	    Verifique:
	    https://www.brainvoyager.com/bv/doc/UsersGuide/CoordsAndTransforms/SpatialTransformationMatrices.html

	    Foi um aprimoramento gigante no algoritmo, experimente ver como 
	    está no original.

	    Não é exatamente garantido que a submatriz do lado esquerdo
	    seja a identidade 3x3.
		*/

		float temp[3] = {0, 0, 0};

		int index = 0;
		for(
			int i =  0;
				i < 12;  // Faz parte do algoritmo mesmo.
				i++
		){

			if(
				i == 4 * (1 + index)
			){

				index++;
			}

			// O operador terciário serve apenas para garantirmos que acessaremos
			// apenas elementos válidos do vetor, forçando o elemento 1 em seguida.
			temp[index] += this->conteudo[i] * ( ((i % 4) != 3) ? vetor[ i % 4 ] : 1 );
		}
		
		return Vetor3D(
						temp[0], 
						temp[1], 
						temp[2]
					  );
	}

	Matriz4D&
	operator=(
		const Matriz4D& outra_matriz
	){

		for(
			int i = 0;
			i < M_TAMANHO;
			i++
		){
			
			this->conteudo[i] = this->conteudo[i] + outra_matriz.conteudo[i];
		}

		return *this;
	}

	float& 
	operator[]( const unsigned index ){ return this->conteudo[ index ]; }

	Vetor3D
	obter_vetor_de_translacao() const { return Vetor3D( this->conteudo[ 3 ], this->conteudo[ 7 ], this->conteudo[ 11 ]); }

	Matriz4D
	obter_transposta() const {

		Matriz4D transp;
		
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
							this->obter(
										 linha,
										 coluna
									  )
							);
			}
		}
		
		return transp;
	}

	Matriz4D
	obter_transformacao_inversa() const {
		/*
	    Suponha uma matriz que representa uma Transformação não necessariamente
	    Linear, basta verificar novamente o estilo que estamos manipulando:
	    https://www.brainvoyager.com/bv/doc/UsersGuide/CoordsAndTransforms/SpatialTransformationMatrices.html

	    Eu desejo obter a matriz que representa a Transformação Inversa da mesma.
	    Para quem olhou no site, desejamos a matriz inversa da apresentada
	    com as entradas em a_ij.
		*/

		Matriz4D inv;  
		
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
			] = this->conteudo[
				index
			];
			
			index++;
		}

		index     = 0;
		while(
			index < 3
		){
			
			inv[
				4 * (index + 1) - 1
			] = (
				- this->conteudo[     index ] * this->conteudo[ 3  ]
			) + (
				- this->conteudo[ 4 + index ] * this->conteudo[ 7  ]
			) + (
				- this->conteudo[ 8 + index ] * this->conteudo[ 11 ]
			);
			
			index++;
		}

		return inv;
	}
};

class Linha6D  {
	/*
	Descrição:
	    Representa uma linha 3D imutável composta por dois pontos do tipo Vetor3D.
	    A classe armazena tanto as coordenadas cartesianas quanto polares dos pontos inicial e final,
	    além do comprimento do segmento de reta.

	Construtores:
	    - Linha6D(Vetor3D, Vetor3D):
	        Constrói uma linha com pontos fornecidos em coordenadas polares.

	    - Linha6D(Vetor3D, Vetor3D, float):
	        Constrói uma linha com pontos cartesianos e comprimento especificado.

	    - Linha6D(const Linha6D&):
	        Construtor de cópia.

	Métodos principais:
	
		ponto_mais_proximo_na_reta_para_ponto_cartesiano(Vetor3D)
		- Retorna o ponto mais próximo, em uma linha infinita, de um ponto cartesiano.

		&-> Há semelhante para ponto esférico.

		distancia_da_reta_para_ponto_cartesiano(Vetor3D)
		- Retorna a distância de um ponto cartesiano até a linha infinita.

		&-> Há semelhante para ponto esférico.

		distancia_entre_retas(Linha6D)
		- Retorna a distância entre duas linhas.

		ponto_mais_proximo_no_segmento_para_ponto_cartesiano(Vetor3D)
		- Retorna o ponto mais próximo dentro do segmento até o ponto fornecido.

		&-> Há semelhante para ponto esférico.

		distancia_do_segmento_para_ponto_cartesiano(Vetor3D)
		- Retorna a distância de um ponto ao segmento de reta.

		&-> Há semelhante para ponto esférico.

		distancia_entre_segmentos(Linha6D)
		- Calcula a menor distância entre dois segmentos de reta.

		ponto_medio_cartesiano() e ponto_medio_esferico()
		- Retorna o ponto médio do segmento em coordenadas cartesianas ou esféricas.

	Observações:
	    - A linha é tratada como imutável: todos os atributos são constantes.
	    - Os métodos de ponto mais próximo e distância consideram tanto linhas infinitas quanto segmentos finitos.
	*/

	const Vetor3D ponto_inicial_esferica;
	const Vetor3D ponto_final_esferica;
	const Vetor3D ponto_inicial_cartesiano;
	const Vetor3D ponto_final_cartesiano;
	const float comprimento;

	//////////////////////////////////////////////////////////////////////////////////////

	Linha6D(
		const Vetor3D& ponto_inicial_esferica_,
		const Vetor3D& ponto_final_esferica_
	) : ponto_inicial_esferica(ponto_inicial_esferica_),
		ponto_final_esferica(ponto_final_esferica_    ),

		ponto_inicial_cartesiano(ponto_inicial_esferica_.to_cart()),
		ponto_final_cartesiano(ponto_final_esferica_.to_cart()),

		comprimento(ponto_inicial_cartesiano.obter_distancia_para(ponto_final_cartesiano))
		{}

	Linha6D(
		const Vetor3D& ponto_inicial_cartesiano_,
		const Vetor3D& ponto_final_cartesiano_,
		float&         comprimento
	) : ponto_inicial_esferica(ponto_inicial_cartesiano_.to_esfe()),
		ponto_final_esferica(ponto_final_cartesiano_.to_esfe()),

		ponto_inicial_cartesiano(ponto_inicial_cartesiano_),
		ponto_final_cartesiano(ponto_final_cartesiano_),
		
		comprimento(comprimento)
		{}

	// Omitimos o construtor de cópia.

	bool
	operator==( const Linha6D& outra_linha ) const { 
		/*
		Observe que vamos comparar apenas as componentes esféricas, assim evitaremos 
		a propagação de erros.
		*/
		return ( ponto_inicial_esferica == outra_linha.ponto_inicial_esferica ) && ( ponto_final_esferica == outra_linha.ponto_final_esferica ); 
	}

	//////////////////////////////////////////////////////////////////////////////////////

	Vetor3D 
	ponto_mais_proximo_na_reta_para_ponto_cartesiano(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
		A partir de um ponto dado qualquer, achará o ponto na RETA INFINITA
		definida pela linha que é mais próximo do ponto dado.
		
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

		float param = (
			ponto_qualquer_cart - ponto_inicial_cartesiano
		).InnerProduct(
			ponto_final_cartesiano - ponto_inicial_cartesiano
		) / (comprimento * comprimento);	

		return ponto_inicial_cartesiano + (ponto_final_cartesiano - ponto_inicial_cartesiano) * param;
	}

	Vetor3D 
	ponto_mais_proximo_na_reta_para_ponto_esferica  (
		const Vetor3D& ponto_qualquer_esfe
	) const {
		/*
		Note que retornaremos o ponto em coordenadas cartesianas.
		*/
		return ponto_mais_proximo_na_reta_para_ponto_cartesiano( ponto_qualquer_esfe.to_cart() );
	}

	float 
	distancia_da_reta_para_ponto_cartesiano(
		const Vetor3D& ponto_qualquer_cart
	) const {
		/*
        Distância de um ponto a uma reta infinita represetada pela linha. 
        
        Suponha B - A o segmento de linha pertencente à reta infinita e 
        Q o ponto qualquer.
        
        Note que:
        
        2 * Área = |(Q - inicio_c) X (Q - final_c)| = Base * Altura
        
        Altura = |(Q - inicio_c) X (Q - final_c)| / |final_c - inicio_c|
		*/

		return (
			(
				ponto_qualquer_cart - ponto_inicial_cartesiano
			).CrossProduct(
				ponto_qualquer_cart - ponto_final_cartesiano
			)
		).obter_modulo() / comprimento;
	}

	float 
	distancia_da_reta_para_ponto_esferica  (
		const Vetor3D& ponto_qualquer_esfe
	) const {
		/*
		Note que retornamos o valor em cartesiano.
		*/
		return distancia_da_reta_para_ponto_cartesiano( ponto_qualquer_esfe.to_cart() );
	}

	float 
	distancia_entre_retas(
		const Linha6D& linha_qualquer
	){
		/*
	    Suponha duas retas definidas pelas linhas, (*this) e a dada.
	    De tal forma que:

	    P = A + t(B - A)         Q = N + r(M - N)

	    São pontos quaisquer respectivamente nas mesmas. Se desejamos a
	    distância entre essas linhas, devemos buscar P - Q de tal forma
	    que essa linha seja perpendicular a B - A e a M - N simultaneamente.

	    < P - Q, B - A > = 0
	    < P - Q, M - N > = 0

	    Em seguida, manipular algebricamente:

	    t< B - A, B - A > - r< M - N, B - A > = < N - A, B - A >
	    t< B - A, M - N > - r< M - N, M - N > = < N - A, M - N >

	    Observe que temos um sistema linear. Há três opções.

	    i) Não há solução.

	        Como estamos em dimensões no máximo tridimensionais, sempre haverá
	        pelo menos uma solução (P, Q) que permitirá a mínima distância.

	    ii) Infinitas soluções.

	        Caso em que as retas estão paralelas. Há infinitos pontos que
	        satisfazem a perpendicularidade.

	        Basta que o determinante da matriz dos coeficientes seja zero.

	        Então basta escolher um ponto da linha dada como fixo, N por exemplo,
	        e obtermos a distância até esse ponto. Neste caso, a solução é trivial.

	    iii) Uma única solução.

	        Para facilitar, podemos denotar os coeficientes como parâmetros e
	        assim resolver o sistema 2x2.
		*/

		float prod_esc_entre_vet_diretores = (
			linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
		).InnerProduct(
			this->ponto_final_cartesiano - this->ponto_inicial_cartesiano
		);
		
		float Det = prod_esc_entre_vet_diretores * prod_esc_entre_vet_diretores - this->comprimento * this->comprimento * linha_qualquer.comprimento * linha_qualquer.comprimento; 
		
		if(
			// Caso sejam suficientemente paralelas
			fabs(Det) < MIN_PARA_QUE_SEJAM_PARALELAS
		){

			return this->distancia_da_reta_para_ponto_cartesiano(linha_qualquer.ponto_inicial_cartesiano);
		}
		
		/*
		Produto escalar entre posição relativa do segmento, N - A, e
		vetor diretor da nossa linha, B - A.
		*/
		float g_this = (
			linha_qualquer.ponto_inicial_cartesiano - this->ponto_inicial_cartesiano
		).InnerProduct(
			this->ponto_final_cartesiano  -  this->ponto_inicial_cartesiano
		);
		/*
		Produto escalar entre posição relativa do segmento, N - A, e
		vetor diretor da linha dada, M - N.
		*/
		float g_other = (
			linha_qualquer.ponto_inicial_cartesiano - this->ponto_inicial_cartesiano
		).InnerProduct(
			linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
		);
	
		// Respectivos Parâmetros:
		float param_t = (
			g_other * prod_esc_entre_vet_diretores - g_this * linha_qualquer.comprimento * linha_qualquer.comprimento
		) / Det;
		
		float param_r = (fabs(prod_esc_entre_vet_diretores) < 1e-5) ? - g_other / (linha_qualquer.comprimento * linha_qualquer.comprimento) : (
			param_t * this->comprimento * this->comprimento - g_this
		) / prod_esc_entre_vet_diretores;
		
		// Respectivos pontos de distância mínima.
		Vetor3D P = this->ponto_inicial_cartesiano + (this->ponto_final_cartesiano - this->ponto_inicial_cartesiano) * param_t;
		Vetor3D Q = linha_qualquer.ponto_inicial_cartesiano + (linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano) * param_r;
		
		return P.obter_distancia_para(Q);
	}	

	//////////////////////////////////////////////////////////////////////////////////////

	Vetor3D
	ponto_mais_proximo_no_segmento_para_ponto_cartesiano(
		const Vetor3D& ponto_qualquer_cart
	){
		/*
		Basicamente, mesma função que a anterior, entretanto, agora limitamos
		os pontos possíveis para o segmento de reta definido por ponto_inicial_cartesiano e 
		ponto_final_cartesiano.
		*/

		float param = (
			ponto_qualquer_cart - ponto_inicial_cartesiano
		).InnerProduct(
			ponto_final_cartesiano - ponto_inicial_cartesiano
		) / (comprimento * comprimento);	
		
		// Ambas condições nunca serão verdadeiras ao mesmo tempo.
		param += (param < 0) * ( - param ) + (param > 1) * (1 - param); // Simplesmente goat
		
		return ponto_inicial_cartesiano + (ponto_final_cartesiano - ponto_inicial_cartesiano) * param;
	}

	Vetor3D
	ponto_mais_proximo_no_segmento_para_ponto_esferica (
		const Vetor3D& ponto_qualquer_esfe
	){
		/*
		Retornamos um ponto cartesiano.
		*/
		return ponto_mais_proximo_no_segmento_para_ponto_cartesiano( ponto_qualquer_esfe.to_cart() );
	}

	float 
	distancia_do_segmento_para_ponto_cartesiano(
		const Vetor3D& ponto_qualquer_cart
	){

		/*
        Mesma função que a de nome semelhante, entretanto, não pensaremos mais
        na reta infinita, e sim no segmento de reta.
        
        Imagine um ponto qualquer Q e um SEGMENTO DE RETA definido por B - A.
        
                  .B
        Q.       /
               /
            A.
        
        Caso Q esteja para "trás" de A, a distância |Q - A| representa a distância
        mínima de Q ao segmento.
        
        Caso Q esteja para "frente" de B, a distância |Q - B| representa a distância
        mínima de Q ao segmento.
        
        Caso Q esteja na região entre A e B, teremos o triângulo.
		*/

		if(
			(
				ponto_qualquer_cart - this->ponto_inicial_cartesiano
			).InnerProduct(
				this->ponto_final_cartesiano - this->ponto_inicial_cartesiano
			) <= 0
		){

			return this->ponto_inicial_cartesiano.obter_distancia_para( ponto_qualquer_cart );
		}

		if(
			(
				ponto_qualquer_cart - this->ponto_final_cartesiano
			).InnerProduct(
				this->ponto_final_cartesiano - this->ponto_inicial_cartesiano
			) >= 0
		){

			return this->ponto_final_cartesiano.obter_distancia_para( ponto_qualquer_cart );
		}

		return (
			ponto_qualquer_cart - this->ponto_inicial_cartesiano
		).CrossProduct(
			ponto_qualquer_cart - this->ponto_final_cartesiano
		).obter_modulo() / this->comprimento;
	}

	float 
	distancia_do_segmento_para_ponto_esferica  (
		const Vetor3D& ponto_qualquer_esfe
	){

		return distancia_do_segmento_para_ponto_cartesiano( ponto_qualquer_esfe.to_cart() );
	}

	float 
	distancia_entre_segmentos(
		const Linha6D& linha_qualquer
	){
		/*
        Obter distância entre dois segmentos de reta definidos respectivamente
        por B - A e M - N.
        
        Podemos reutilizar o algoritmo das retas infinitas e verificar os valores
       	dos parâmetros.
        
        Caso não sejam paralelos:

        Trivialmente reutilizaremos o algoritmo da função distancia_entre_linhas
        e analisaremos se os parâmetros são menores que 0 ou maiores
        que 1.

        Caso sejam paralelos:

        M.
        
        
        N.
        
                .B
                
                .A
        
        Observe que, dependendo se M - N está "à frente" ou "atrás",
        temos configurações diferentes de distâncias.

        Caso B não esteja para trás de N e A não esteja para 
        frente de M, teremos:

        M.
            .B

            .A
        N.

        Neste caso, teremos um trapézio.
		*/
		float prod_esc_entre_vet_diretores = (
			linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
		).InnerProduct(
			this->ponto_final_cartesiano - this->ponto_inicial_cartesiano
		);
		
		float Det = prod_esc_entre_vet_diretores * prod_esc_entre_vet_diretores - this->comprimento * this->comprimento * linha_qualquer.comprimento * linha_qualquer.comprimento; 
		
		if(
			// Caso sejam paralelas ou pelo menos quase paralelas.
			fabs(Det) < MIN_PARA_QUE_SEJAM_PARALELAS
		){
			
			if(
				/*
				Se B estiver para trás de N, a distância será |B - N|.
				*/
				(
					this->ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
				).InnerProduct(
					linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
				) <= 0
			){
				
				return this->ponto_final_cartesiano.obter_distancia_para(linha_qualquer.ponto_inicial_cartesiano);
			}
			
			if(
				/*
				Se A estiver para frente de M, a distância será |A - M|.
				*/
				(
					this->ponto_inicial_cartesiano - linha_qualquer.ponto_final_cartesiano
				).InnerProduct(
					linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
				) >= 0
			){
				
				return this->ponto_inicial_cartesiano.obter_distancia_para(linha_qualquer.ponto_final_cartesiano);
			}
			
			/*
			Obter altura a partir da área do trapézio formado pelas linhas.
			*/
			
			float semi_triang_1 = (linha_qualquer.ponto_inicial_cartesiano - this->ponto_inicial_cartesiano).CrossProduct(this->ponto_final_cartesiano - this->ponto_inicial_cartesiano).obter_modulo();
			float semi_triang_2 = (linha_qualquer.ponto_inicial_cartesiano -    linha_qualquer.ponto_final_cartesiano).CrossProduct(this->ponto_final_cartesiano -   linha_qualquer.ponto_inicial_cartesiano).obter_modulo();
			
			return (semi_triang_1 + semi_triang_2) / (linha_qualquer.comprimento + this->comprimento);
		}
		
		/*
		Produto escalar entre posição relativa do segmento, N - A, e
		vetor diretor da nossa linha, B - A.
		*/
		float g_this = (
			linha_qualquer.ponto_inicial_cartesiano - this->ponto_inicial_cartesiano
		).InnerProduct(
			this->ponto_final_cartesiano - this->ponto_inicial_cartesiano
		);
		/*
		Produto escalar entre posição relativa do segmento, N - A, e
		vetor diretor da linha dada, M - N.
		*/
		float g_other = (
			linha_qualquer.ponto_inicial_cartesiano - this->ponto_inicial_cartesiano
		).InnerProduct(
			linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano
		);
	
		// Respectivos Parâmetros:
		float param_t = (
			g_other * prod_esc_entre_vet_diretores - g_this * linha_qualquer.comprimento * linha_qualquer.comprimento
		) / Det;
		
		float param_r = (fabs(prod_esc_entre_vet_diretores) < 1e-5) ? - g_other / (linha_qualquer.comprimento * linha_qualquer.comprimento) : (
			param_t * this->comprimento * this->comprimento - g_this
		) / prod_esc_entre_vet_diretores;
		
		// Correção para Segmentos de Reta.
		param_t += (param_t < 0) * (- param_t) + (param_t > 1) * (1 - param_t);
		
		param_r += (param_r < 0) * (- param_r) + (param_r > 1) * (1 - param_r);
		
		// Respectivos pontos de distância mínima.
		Vetor3D P = ponto_inicial_cartesiano + (ponto_final_cartesiano - ponto_inicial_cartesiano) * param_t;
		Vetor3D Q = linha_qualquer.ponto_inicial_cartesiano + (linha_qualquer.ponto_final_cartesiano - linha_qualquer.ponto_inicial_cartesiano) * param_r;
		
		return P.obter_distancia_para(Q);
	}

	Vetor3D
	obter_ponto_medio_da_linha_cartesiano() const { return (this->ponto_inicial_cartesiano + this->ponto_final_cartesiano) / 2; }

	Vetor3D
	obter_ponto_medio_da_linha_esferica	 () const { return this->obter_ponto_medio_da_linha_cartesiano().to_esfe(); }
};

#endif // ALGLIN_H
