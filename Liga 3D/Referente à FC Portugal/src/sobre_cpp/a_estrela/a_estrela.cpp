/*
Matheus Deyvisson, 2025

Deixo registrado mais uma vez o nível de profissionalismo do código original
escrito por Miguel Abreu.
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

/*
Explicação do que significa 'inline'
	Usada para sugerir ao compilador que uma função deve ser expandida
	diretamente no local da chamada, em vez de realizar uma chamada de
	função tradicional. De semelhante à funções lambda do Python.
	
	Mais precisamente, é parecido com o #define só que para função.
*/
inline int x_para_linha(
	float x
){
	/*
	Descrição:
		Transformar de coordenada matemática x para coordenada de linha do campo.
	*/
	return int(
		// Linhas e Colunas são inteiras.
		fmaxf(
			/*
			Garantimos que a saída será 0 ou algo maior.
			*/
			0.f,
			fminf(
				/*
				Garantimos que estará sob as 320 linhas totais.
				*/
				10 * x + 160,
				320.f
			)
		) + 0.5f
	);
}

inline int y_para_col(
	float y
){
	return int(
		// Linhas e Colunas são inteiras.
		fmaxf(
			/*
			Garantimos que a saída será 0 ou algo maior.
			*/
			0.f,
			fminf(
				/*
				Garantimos que estará sob as 220 colunas totais.
				*/
				10 * y + 110,
				220.f
			)
		) + 0.5f
	);
}

inline float distancia_diagonal(
	bool ir_ao_gol,
	int linha,
	int coluna,
	int linha_final,
	int coluna_final
){
	/*
	Descrição:
		Oq está prestes é algo muito foda.
		Sugiro que verifique o seguinte site explicatório: http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html
		
		Basicamente, estamos escolhendo velocidade à precisão! Buscando
		um caminho que seja bom o suficiente, mas o mais rápido possível.	
	*/
	
	int delta_linha = 0;
	int delta_coluna = 0;
	
	if(
		ir_ao_gol
	){
		delta_linha = abs(
			LINHA_DO_GOL - linha
		);
		
		// OK, me rendi à forma que foi escrito no original.
		if(coluna > 119)       { delta_coluna = coluna - 119; }
		else if(coluna < 101)  { delta_coluna = 101 - coluna; }
		else                   { delta_coluna = 0;			  }
	}
	else{
		delta_linha  = abs( linha  - linha_final  );
		delta_coluna = abs( coluna - coluna_final );
	}
	
	/*
	Aqui está o cerne da brincadeira. Sugiro que de verdade que verifique
	o site apresentado na descrição. Alto Nível demais.
	Esse número é um valor específico para o campo. Tome cuidado alterando-o.
	*/
	
	return (delta_linha + delta_coluna) - 0.585786437626905f * min(delta_linha, delta_coluna);
}

// Assim como definimos um Terra em circuitos, definiremos um nó não expandido
// que representará o menor custo total previsto.
Node* min_node = nullptr;  

namespace noding{
	/*
	Descrição:
		Define um espaço de nomes (namespace), que serve para organizar e 
		agrupar funcionalidades (como classes, funções, variáveis) em um escopo
		nomeado, evitando conflitos de nomes com outras partes do programa ou 
		bibliotecas externas.
		
		Acredito que foi feito assim pois Node não é exatamente um objeto,
		mas uma estrutura de dados e, portanto, struct. E, por legibilidade,
		não associamos classes<->structs, cada um no seu quadrado.
		
		Coloquei 'noding' porque queria relacionar com Node.
			
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
		bool se_eh_esquerdo
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
			Insere um nó de maneira correta dentro da estrutura
			
		Retorno:
			Raiz da estrutura.
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
		sucedida, isto é, o novo node tem um valor menor que o nó mínimo.
		
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
		O pai do novo nó torna-se o mínimo atual, [1].
		O novo nó torna-se o mínimo.
		
		* Estrutura na Saída:
		
		   		[5]
		       /   \
		     [2]   [10]
		    /   \
		  [1]    [4]
		  /
		[0] <- Novo MIN
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
	remover_min(
		Node* raiz
	){
		/*
		Descrição:
			Desejamos remover o menor elemento, que apesar de que deveria ser
			o último elemento, não necessariamente será.
			
			Note que o mínimo pode ter um filho, mas este necessariamente
			será direito.
			
		Retorno:
			Raiz da estrutura.
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
	
	
	Node*
	remover_node(
		Node* node_a_ser_removido,
		Node* raiz
	){
		/*
		Descrição:
			Separa em casos de existência de filhos e remove nós desejados.
			
		Retorno:
			Raiz da estrutura.
		*/
		
		if(
			node_a_ser_removido == min_node
		){
			return remover_min(
				raiz
			); // removerá o mínimo da estrutura.
		}
		
		if(
			// Caso o nó a ser removido não tenha filhos, logo também não é raiz.
			(*node_a_ser_removido).left == nullptr && (*node_a_ser_removido).right == nullptr
		){
			/*
			A partir do nó a ser removido, devemos retirar a conexão de descida do pai
			para ele.
			*/
			if(
				(*((*node_a_ser_removido).up)).left == node_a_ser_removido
			){
				(*((*node_a_ser_removido).up)).left = nullptr;
			}
			else{
				(*((*node_a_ser_removido).up)).right = nullptr;
			}
			
			delete node_a_ser_removido;
			
			return raiz;
		}
		
		if(
			// Caso tenha somente filho direito.
			(*node_a_ser_removido).left == nullptr
		){
			// Redirecionamos conexões de subida do filho direito do nó a ser 
			// removido para o pai do nó a ser removido.
			(*((*node_a_ser_removido).right)).up = (*node_a_ser_removido).up;
			
			if(
				(*((*node_a_ser_removido).up)).left == node_a_ser_removido		
			){
				/*
				Já que estamos removendo um nó que tem filho direito, fazemos com que 
				este seja o novo filho do pai do nó a ser removido, conservando o lado
				deste.
				*/
				(*((*node_a_ser_removido).up)).left = (*node_a_ser_removido).right;
			}
			else{
				(*((*node_a_ser_removido).up)).right = (*node_a_ser_removido).right;
			}
			
			delete node_a_ser_removido;
			
			return raiz;
			
		}
		
		if(
			// Caso somente tenha filho esquerdo
			(*node_a_ser_removido).right == nullptr
		){
			
			if(
				/*
				Além de ter somente filho esquerdo, ele é a raiz. Logo,
				devemos desfazer a conexão de subida do filho esquerdo e 
				torná-lo a nova raiz.
				*/
				node_a_ser_removido == raiz
			){
				
				(*((*node_a_ser_removido).left)).up = nullptr;
				
				Node* nova_raiz = (*node_a_ser_removido).left;
				
				delete node_a_ser_removido;
				
				return nova_raiz; 
			}
			
			// Como não é a raiz.
			(*((*node_a_ser_removido).left)).up = (*node_a_ser_removido).up;
			
			if(
				(*((*node_a_ser_removido).up)).left == node_a_ser_removido
			){
				(*((*node_a_ser_removido).up)).left = (*node_a_ser_removido).left;
			}
			else{
				(*((*node_a_ser_removido).up)).right = (*node_a_ser_removido).left;
			}
			
			delete node_a_ser_removido;
			
			return raiz;
		}
		
		/*
		Logo, tem filhos e não é o mínimo.
		Vejamos o exemplo:
		
			   [5]
		      /   \
 (Removê-lo)[2]   [10]
	   	   /   \
		[1]    [4]
		...		...
		
		*/
		
		/*
		Como é BST, temos a garantia que o filho direito é maior que o 
		filho esquerdo. 
		
		Caso o filho esquerdo do nó a ser removido substitua-o, este
		será pai do filho direito do nó a ser removido, o que contradiz a BST.
		
		Portanto, o filho direito do nó a ser removido, [4], torna-se o
		sucessor.
		*/
		Node* sucessor = (*node_a_ser_removido).right;
		
		if(
			// Sucessor não tem filho esquerdo.
			(*sucessor).left == nullptr
		){
			// Caso o sucessor não tenha filho esquerdo.
			
			/*
			Filho esquerdo do nó a ser removido torna-se o filho esquerdo
			do sucessor.
			*/
			(*sucessor).left = (*node_a_ser_removido).left; 
			
			/*
			Pai do sucessor torna-se o pai do nó a ser removido.
			*/
			(*sucessor).up = (*node_a_ser_removido).up;
			
			/*
			Pai do filho esquerdo torna-se o sucessor.
			*/
			(*((*node_a_ser_removido).left)).up = sucessor;
			
			if(
				// Se for a raiz
				node_a_ser_removido == raiz
			){
				delete node_a_ser_removido;
				
				return sucessor; // Este tornou-se a nova raiz
			}
			
			// Se não for a raiz, alteramos o filho do pai do nó a ser removido.
			if(
				(*((*node_a_ser_removido).up)).left == node_a_ser_removido
			){
				
				(*((*node_a_ser_removido).up)).left = sucessor;
			}
			else{
				
				(*((*node_a_ser_removido).up)).right = sucessor;
			}
		}
		else{
			// Sucessor tem filho esquerdo.
			
			while(
				/*
				Enquanto o sucessor tiver filho esquerdo, vamos
				tornar este o novo sucessor.
				*/
				(*sucessor).left != nullptr
			){
				sucessor = (*sucessor).left;
			}
			
			if(
				/*
				Pelo loop anterior, não há filho esquerdo.
				Verificamos se não há filho direito.
				*/
				(*sucessor).right == nullptr
			){
				// Não há filhos direito e esquerdo no sucessor.
				
				/*
				O pai desse sucessor não terá mais filho esquerdo. Por isso, nullptr.
				Eliminamos a conexão de descida.
				*/
				(*((*sucessor).up)).left = nullptr;
			}
			else{
				/*
				O filho esquerdo do pai do sucessor, deve se tornar o único filho do sucessor.
				*/
				(*((*sucessor).up)).left = (*sucessor).right;
				
				/*
				Apenas a conexão de subida referente à conexão de descida definida anteriormente.
				*/
				(*((*sucessor).right)).up = (*sucessor).up;
			}
			
			/*
			Alteramos todas as conexões do sucessor para substituir exatamente
			o nó que desejamos remover.
			*/
			(*sucessor).left = (*node_a_ser_removido).left;
			(*sucessor).right = (*node_a_ser_removido).right;
			(*sucessor).up = (*node_a_ser_removido).up;
			
			/*
			Atualizamos as conexões de subida dos filhos esquerdo e direito para
			o substituto do nó a ser removido, vulgo sucessor.
			*/
			(*((*node_a_ser_removido).left)).up = sucessor;
			(*((*node_a_ser_removido).right)).up = sucessor;
			
			if(
				node_a_ser_removido == raiz
			){
				
				delete node_a_ser_removido;
				return sucessor;
			}
			
			if(
				/*
				Apenas atualizamos a conexão de descida do pai do node a ser removido.
				*/
				(*((*node_a_ser_removido).up)).left == node_a_ser_removido
			){
				
				(*((*node_a_ser_removido).up)).left = sucessor;
			}
			else{
				
				(*((*node_a_ser_removido).up)).right = sucessor;
			}
			
			return raiz;
		}
		/*
		Um exemplo completo do algoritmo referente ao último 
		
		Questão:
		
			L---5
			    |---10
			    L---2
			        |---4
			        |   |---5
			        |   L---3
			        L---1
			
			Desejamos remover o nó de valor 2.
			
		Solução:
			
			Sucessor torna-se nó 4.
			
			Este possui filho esquerdo, logo enquanto houver um filho esquerdo,
			teremos garantia que este será menor que seu pai( Definição BST).
			Iteramos atualizando o sucessor para cada filho esquerdo.
			Note que mesmo que haja filhos direitos, temos a garantia de que
			estes serão maiores que os respectivos filhos esquerdos(Definição BST).
			Nó 3 torna-se o sucessor.
			
			
			Neste caso, não há filho direito no sucessor. Como o sucessor vai sair, 
			o pai dele ficará sem filhos.
			Caso tivesse filho direito no sucessor, ele se tornaria o novo filho do 
			pai do então sucessor.
			
			Atualizamos as conexões que partem do sucessor para que ele substitua
			o nó que será removido.
			Em seguida, atualizamos as atualizações dos filhos do nó que será removido
			para apontarem para o substituto, vulgo sucessor.
			Finalmente, atualizamos a conexão de descida do pai do nó que será removido
			para apontar para o sucessor.
		*/
		return raiz;
	}
	
	Node*
	expandir_filho(
		Node* raiz_da_estrutura,  
		float custo_para_chegar_ao_no_desejado,
		float limite_para_qual_custo_eh_impossivel,
		Node* node_atual_a_ser_expandido,
		/*
		Representação da grid do campo
		*/
		Node* quadro_de_possibilidades,
		int posicao_do_node_no_quadro,
		/*
		0 - Explorado
		1 - Lista Aberta
		*/
		int estado_atual_do_node, 
		bool ir_ao_gol,
		// Coordenadas do nó
		int linha, int coluna,
		// Coordenadas do nó final
		int linha_final, int coluna_final,
		// Array que armazenará estados dos nós
		unsigned int* estado_dos_nodes,
		// Custo Adicional para o movimento
		float extra
	){
		/*
		Descrição:
			Parte do algoritmo para busca de melhor caminho possível, 
			expandindo e avaliando filhos durante busca.
			
			Fiz o possível para tentar decifrar o que acontece aqui.
		*/
		
		if(
			/*
			Filho pode ser inacessível a partir do nó atual, para isso haverá um 
			custo de penalidade para desencorajar caminhos impossíveis.
			*/
			custo_para_chegar_ao_no_desejado <= limite_para_qual_custo_eh_impossivel
		){
			
			custo_para_chegar_ao_no_desejado = 100.f;
		}
		
		float min_custo = (*node_atual_a_ser_expandido).custo_pontual + extra + std::fmaxf(
			0.f,
			custo_para_chegar_ao_no_desejado
		);  // Max para garantimos que seja positivo.
		
		Node* filho = quadro_de_possibilidades + posicao_do_node_no_quadro;
		
		if(
			// Caso o nó atual já esteja na lista aberta.
			estado_atual_do_node
		){
			if(
				/*
				E o novo custo calculo não for melhor que o do nó atual,
				o descartamos.
				*/
				min_custo >= (*node_atual_a_ser_expandido).custo_pontual
			){
				
				return raiz_da_estrutura;
			}
			else{
				/*
				Houve uma melhora, devemos remover a referência do nó filho,
				atualizá-lo e adicioná-lo na estrutura posteriormente.
				*/
				raiz_da_estrutura = noding::remover_node(
					filho,
					raiz_da_estrutura
				);
			}
		}
		else{
			// Colocamos ele na lista.
			estado_dos_nodes[
				posicao_do_node_no_quadro
			] = 1;
		}
		
		/*
		Preenchemos características do nó filho que está sendo analisado.
		*/
		float predicao_de_custo_para_atravessar_filho = min_custo + distancia_diagonal(
			ir_ao_gol,
			linha,
			coluna,
			linha_final,
			coluna_final
		);
		
		(*filho).custo_pontual = min_custo;
		(*filho).valor = predicao_de_custo_para_atravessar_filho;
		(*filho).parente = node_atual_a_ser_expandido;
		
		// Finalmente, o inserimos.
		return noding::inserir(
			filho,
			raiz_da_estrutura
		);
	}

}

float caminho_final[2050] = {0};
int tamanho_do_caminho_final = 0;

inline void
construir_caminho_final(
	/*
	Observe essa diferença sutil, simplesmente genial Miguel Abreu.
	
	Node* const
		Ponteiro para Node constante.
		Objeto Node não pode ser modificado através do ponteiro.
		Mas o ponteiro pode apontar para outro Node.
	
	Node* const
		Ponteiro constante para Node
		Ponteiro em si é constante, não pode apontar para outro Node.
		Mas o objeto Node pode ser modificado.
	*/
	Node* const melhor_node,
	const Node* quadro_de_possibilidades,
	
	float status,
	/*
	Ativa ou desativa a substituição das coordenadas finais discretas
	por valores exatos.
	
	False - Coordenadas Discretas do Grid
	True  - Substitui o último ponto do caminho por valores exatos em end_x e em end_y.
	*/
	const bool override_end=false,
	/*
	Especificam as coordenadas exatas do ponto final quando override está ativo.
	*/
	const float end_x=0, const float end_y=0
){
	/*
	Descrição:
		Função responsável por construir o melhor a partir da estrutura de 
		nós fornecida.
	*/
	
	Node* ptr = melhor_node;
	
	int quantidade_de_parentes = 0;
	while(
		ptr != nullptr
	){
		
		ptr = (*ptr).parente;
		quantidade_de_parentes++;
	}
	
	tamanho_do_caminho_final = min(
		/*
		Limitamos o caminho final à quantidade de slots disponíveis.
		*/
		2 * quantidade_de_parentes,
		2048
	);
	
	/*
	Voltamos ao melhor nó. 
	Note que foi por isso que escrevemos o const, para preservar essa
	propriedade. 
	Completamente insano.
	*/
	ptr = melhor_node; 
	int indice = tamanho_do_caminho_final - 1;
	
	if(
		// Se ativado, atualiza o ponto final para coordenadas corretas
		// ao invés de versão discreta.
		override_end
	){
		/*
		indice   - acessamos no indice.
		indice-- - acessamos no indice e o decrementamos
		*/
		caminho_final[indice--] = end_y;
		caminho_final[indice--] = end_x;
		
		ptr = (*ptr).parente;
	}
	
	while(
		indice > 0
	){
		// Não é um delta embaixo parece.
		caminho_final[indice--] = (
			(ptr - quadro_de_possibilidades) % QUANT_COLUNAS
		) / 10.f - 11.f;  // y
		
		caminho_final[indice--] = (
			(ptr - quadro_de_possibilidades) / QUANT_COLUNAS
		) / 10.f - 16.f;  // x
		
		ptr = (*ptr).parente;
	}
	
	/*
	Atualizamos o status do caminho e acrescentamos a unidade do tamanho
	
	0 - sucesso
	1 - tempo excedido
	2 - impossível
	*/
	caminho_final[
		tamanho_do_caminho_final++
	] = status;  
		
	/*
	Informações sobre o custo completo do caminho.
	*/
	caminho_final[
		tamanho_do_caminho_final++
	] = (*melhor_node).custo_pontual / 10.f; 
}	


int main()
{
 	// Exemplo de BST para testarmos código de noding.
 	// Fiz usando // para poder usar o atalho.
	{
//		Node *node = new Node(5);			
//		Node *node_l = new Node(2);
//		(*node_l).up = node;
//		(*node).left = node_l;
//		
//		Node *node_r = new Node(10);
//		(*node_r).up = node;
//		(*node).right = node_r;
//		
//		Node *node_ll = new Node(1);
//		(*node_ll).up = node_l;
//		(*node_l).left = node_ll;
//		
//		Node *node_lr = new Node(4);
//		(*node_lr).up = node_l;
//		(*node_l).right = node_lr;
//		
//		Node* node_lrl = new Node(3);
//		(*node_lrl).up = node_lr;
//		(*node_lr).left = node_lrl;
//		
//		Node* node_lrr = new Node(5);
//		(*node_lrr).up = node_lr;
//		(*node_lr).right = node_lrr;
//		
//		
//		noding::apresentar(
//			node
//		);
//		
//		noding::liberar(
//			node
//		);

	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
    return 0;
}
