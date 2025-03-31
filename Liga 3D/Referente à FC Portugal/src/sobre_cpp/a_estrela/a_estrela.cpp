/*
Matheus Deyvisson, 2025
*/

#include <iostream>  // Apenas para realizarmos testes.

#include "a_estrela.h"
#include "obtendo_possibilidades.h"
#include <cmath>
#include <algorithm>
#include <chrono>

/*
Apenas para melhorarmos legibilidade.
*/
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

using std::min;
using std::max;

// Assim como definimos um Terra em circuitos, definiremos um nó não expandido
// que representará o menor custo total previsto.
Node* min_node = nullptr;  

namespace noding{
	/*
	Descrição:
		Define um espaço de nomes (namespace) chamado open, que serve para 
		organizar e agrupar funcionalidades (como classes, funções, variáveis)
		em um escopo nomeado, evitando conflitos de nomes com outras partes do
		programa ou bibliotecas externas.
		
		Acredito que foi feito assim pois Node não é exatamente um objeto,
		mas uma estrutura de dados e, portanto, struct. E, por legibilidade,
		não associamos classes<->structs, cada um no seu quadrado.
		
		Coloquei 'noding' porque queria relacionar com Node.
		
		A seguir, um exemplo de criação BST que você pode usar para testas
		as funções criadas aqui.
		
			Node *node = new Node(1);
			
			Node *node_l = new Node(2);
			(*node_l).up = node;
			(*node).left = node_l;
			
			Node *node_r = new Node(3);
			(*node_r).up = node;
			(*node).right = node_r;
			
			Node *node_s = new Node(99);
			(*node_s).up = node_r;
			(*node_r).right = node_s;
			
			noding::apresentar(
				node
			);
			
			delete node_s;
			delete node_l;
			delete node_r;
			delete node;
				
	Forma de Uso:
		open::imprimir();
		std::cout << open::valor;
	*/
	
	int
	apresentar_recursivamente(
		Node* raiz,
		/*
		const std::string&
			
			É uma referência constante a um objeto do tipo std::string. Usá-la
			garante eficiência para o caso de darmos strings grandes demais,
			pois o endereço de memória original será usado.
			
			O const serve apenas para indicar que não será modificado dentro
			da função.
		*/
		const std::string& prefixo,
		int se_eh_esquerdo
	){
		/*
			Apresentará propriamente a BST.
		*/
		if(
			raiz == nullptr
		){
			return 0;
		}
		
		std::cout << prefixo;
		
		std::cout << (se_eh_esquerdo ? "|---" : "L---");  // Sim, eu sei, operador terciário, me perdoe.
		
		std::cout << (*raiz).valor << std::endl;
		
		apresentar_recursivamente(
			(*raiz).right,
			prefixo + (se_eh_esquerdo ? "|   " : "    "),
			true
		);
		apresentar_recursivamente(
			(*raiz).left,
			prefixo + (se_eh_esquerdo ? "|   " : "    "),
			false
		);
	}
	
	int apresentar(
		Node* raiz
	){
		/*
		Descrição:
			Apenas preenche os parâmetros corretamente, exigir isso do usuário
			é demais.
		*/
		return apresentar_recursivamente(
			raiz,
			"",
			false
		);
	}
		
	int liberar(
		Node* node
	){
		/*
		Descrição:
			Libera a memória usada pela estrutura.
		*/
		if(
			node == nullptr
		){
			return 0;
		}
		
		liberar(
			(*node).left
		);
		
		liberar(
			(*node).right
		);
		
		std::cout << "\nLiberando noh de valor: " << (*node).valor;
		
		delete node;
	}	
	
	Node*
	inserir(
		Node* novo_node,
		Node* raiz
	){
		/*
		Descrição:
			Obviamente pega um nó e o insere dentro de outra estrutura.
		*/
		
		(*novo_node).left = nullptr;
		(*novo_node).right = nullptr;
		
		// Caso estrutura vazia
		if(
			raiz == nullptr
		){
			(*novo_node).up = nullptr;
			
			min_node = novo_node;  // Salvamos o nó minimo.
			
			return novo_node;
		}
		
		///////////////////////////////////////////////////////////////////
		
		// Caso o novo nó seja o menor que o mínimo.
		if(
			(*novo_node).valor < (*min_node).valor 
		){
			/*
			O nó minimo passa a ter este novo nó como seu filho esquerdo.
			*/
			(*min_node).left = novo_node;
			
			/*
			O novo nó passa a ter o nó mínimo como pai.
			*/
			(*novo_node).up = min_node;
			
			/*
			Atualiza o nó mínimo para ser o novo nó.
			Retornando a raiz da árvore pois a inserção foi concluída.
			*/
			min_node = novo_node;  
			
			return raiz;
		}
		/*
		Mostremos um exemplo para caso a verificação anterior seja bem
		sucedida.
		
		* Estrutura no Início:
		
			  [5]
		     /   \
	    	[2]   [10]
	   	   /   \
		[1]    [4]
       	^
		|(min_node)
		
		* Ordem de Ações:
		
		Filho esquerdo do mínimo torna-se o novo nó.
		O pai do novo nó torna-se o mínimo.
		O novo nó torna-se o mínimo.
		
		* Estrutura na Saída:
		
		   		[5]
		       /   \
		     [2]   [10]
		    /   \
		  [1]    [4]
		  /
		[0] <- MIN
		*/
		
		///////////////////////////////////////////////////////////////////
		
		/*
		Definimos um ponto de partida para iterarmos sobre a estrutura.
		Desse jeito, estamos peganodo o mesmo endereço de memória.
		*/
		Node* node = raiz; 
		
		// Partimos do fato que é uma BST.
		while(
			1
		){
			if(
				// Se for menor, devemos colocá-lo na esqueda.
				(*novo_node).valor < (*node).valor
			){
				
				if(
					/*
					Caso não exista filho esquerdo, devemos tornar o novo nó
					este filho esquerdo. Enfim, teremos acabado.
					*/
					(*node).left == nullptr
				){
					
					(*node).left = novo_node;
					
					break;
				}
				else{
					/*
					Caso exista um filho esquerdo, vamos iterar sobre ele
					para novas verificações.
					*/
					
					node = (*node).left;
				}
				
			}
			else{
				// Caso seja maior ou igual, devemos colocá-lo na direita.
				
				// Basta ler o primeiro caso para maiores explicações.
				if(
					(*node).right == nullptr
				){
					
					(*node).right = novo_node;
					
					break;
				}
				else{
					
					node = (*node).right;
				}
			}
		}
		
		// Por fim, atualizamos o Pai do novo nó.
		(*novo_node).up = node;
		
		return raiz;
	}
	
	Node*
	pop_min(
		Node* raiz
	){
		/*
		Descrição:
			Desejamos remover o menor elemento, que apesar de deveria ser
			o último elemento, não necessariamente será.
			
			Note que o mínimo pode ter um filho, mas este necessariamente
			será direito.
		*/
		
		if(
			// Caso não tenha filho.
			(*min_node).right == nullptr
		){
			
			if(
				// Caso não tenha pai.
				min_node == raiz
			){
				// Então, após removê-lo não haverá nada dentro.
				delete min_node;
				
				return nullptr;
			}
			
			/*
			Caso ele tenha um pai.
			
			Tornar o elemento esquerdo do pai nulo.
			
			E atualizar o novo mínimo.
			*/
			
			(*((*min_node).up)).left = nullptr;
			
			Node* temp_para_eliminacao = min_node;
			
			min_node = (*temp_para_eliminacao).up;
			
			delete temp_para_eliminacao;
		}
		else{
			// Há filho direito
			
			if(
				// Caso seja a raiz.
				min_node == raiz
			){
				/*
				Essa situação é bem mais rara, pois teremos:
				
				  [5](min_node)
			        \
		   			[10]
				
				Note que ele possui filho direito e é a raiz.
				*/
				
				Node* temp_para_eliminacao = min_node;
				
				min_node = (*min_node).right;
				
				delete temp_para_eliminacao;
				
				raiz = min_node;
				
				(*raiz).up = nullptr;
				/*
				No exemplo que eu dei anteriormente, movemos o nó mínimo 
				para que seja o 10 e a raiz também, eliminamos o up.
				*/
				
				while(
					/*
					Já estamos com um determinado nó.
					Enquanto houver um filho esquerdo, haverá necessariamente
					um nó menor, portanto, devemos torná-lo mínimo.
					*/
					(*min_node).left != nullptr
				){

					min_node = (*min_node).left;
				}
				
				return raiz;				
			}
			
			/*
			Considere este exemplo para este caso.
			
			     [7]
			  	/
			  [5](min_node)
	            \
   			   [10]
   			   
			*/
			
			/*
			Evidenciamos que o pai do filho direito deve ser o pai do mínimo.
			*/
			(*((*min_node).right)).up = (*min_node).up;
			
			/*
			O filho esquerdo do pai do mínimo deverá ser o filho direito do mínimo.
			*/
			(*((*min_node).up)).left = (*min_node).right;
			
			// Atualizamos o novo mínimo
			
			Node* temp_para_eliminacao = min_node;
			
			min_node = (*min_node).right;
			
			delete min_node;
			
			while(
				/*
				Enquanto houver um elemento à esquerda, haverá um mínimo.
				*/
				(*min_node).left != nullptr
			){
				
				min_node = (*min_node).left;
			}
			
			return raiz;
		}	
		
		return raiz;
	}

}

int main()
{
	
//	Node *node = new Node(5);
//			
//	Node *node_l = new Node(2);
//	(*node_l).up = node;
//	(*node).left = node_l;
//	
//	Node *node_r = new Node(8);
//	(*node_r).up = node;
//	(*node).right = node_r;
//	
//	Node *node_s = new Node(7);
//	(*node_s).up = node_r;
//	(*node_r).left = node_s;
//	
//	min_node = node_s;
//	
//	noding::apresentar(
//		node
//	);
//	
//	noding::liberar(
//		node
//	);
	
    return 0;
}
