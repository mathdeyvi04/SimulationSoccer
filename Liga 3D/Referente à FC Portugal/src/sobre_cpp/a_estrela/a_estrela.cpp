/*
Matheus Deyvisson, 2025

Deixo registrado mais uma vez o nível de profissionalismo do código original
escrito por Miguel Abreu.

Sim, eu sei que esse arquivo é grande. Entretanto:
	
	Não é possível a criação de outros arquivos .cpp, pois obrigaria 
	a existência de um build aprimorado e não conseguiriamos criar o 
	.so trivialmente.
	
	Determinadas funções estão usando parâmetros definidos no escopo global,
	pois fazemos alterações nos ponteiros. Usando mais arquivos, a lógica ficaria mais complexa.

	Além disso, diversas linhas são apenas comentários para auxiliá-lo na compreensão.
*/

#include <iostream>  // Apenas para realizarmos testes.

#include "a_estrela.h"
#include "obtendo_possibilidades.h"

#include <cmath>
#include <algorithm>
#include <chrono>

using std::min;
using std::max;

/*
Apenas para melhorarmos legibilidade.
*/
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;


/*
Explicação do que significa 'inline'
	Usada para sugerir ao compilador que uma função deve ser expandida
	diretamente no local da chamada, em vez de realizar uma chamada de
	função tradicional. De semelhante à funções lambda do Python.
	
	Mais precisamente, é parecido com o #define só que para função.
*/
inline int 
x_para_linha(
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


inline int 
y_para_col(
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


inline float 
distancia_diagonal(
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


inline bool 
se_a_linha_ab_intercepta_qualquer_gol(
	float a_x,
	float a_y,
	float b_x,
	float b_y
){
	/*
	Descrição:
		Objetivo da função é autoexplicativo.
		
		Assume que a e b são dois pontos fora da área inalcançável do gol.
		Logo, linha ab, se for o caso, deve entrar e sair da área inalcançável.
		
		Para detectar isso, consideramos apenas a interseção da linha ab com as bordas
		externas do gol.
		
		As bordas internas terão outras verificações.
		
		Os números usados aqui são específicos para as dimensões do gol.
		Não razão para eles além de proporções geométricas.
	*/
	
	
	float delta_x = b_x - a_x;
	float delta_y = b_y - a_y;
	float coef_ang = 0;
	
	if(
		// Verificar se linha ab e 'goal back' são não colineares.
		delta_x != 0
	){
		/*
		a_x + delta_x * coef_ang = 15.75.
		*/
		coef_ang = (
			15.75 - a_x
		) / delta_x;
		
		if(
			(
				coef_ang >= 0
			) && (
				coef_ang <= 1 
			) && (
				fabsf(  // Apenas pega o valor absoluto de floats.
					a_y + delta_y * coef_ang 
				) < 1.25
			)
		){
			/*
			Então há colisão.
			*/
			return true;
		}
		
		coef_ang = (
			- 15.75 - a_x
		) / delta_x;
		
		if(
			(
				coef_ang >= 0
			) && (
				coef_ang <= 1 
			) && (
				fabsf(  // Apenas pega o valor absoluto de floats.
					a_y + delta_y * coef_ang 
				) < 1.25
			)
		){
			/*
			Então há colisão.
			*/
			return true;
		}
		
	}
	
	if(
		// Verificar se linha ab e 'goal sides' são não colineares.
		delta_y != 0
	){
		/*
		a_y + delta_y * coef_ang = 1.25
		*/
		
		coef_ang = (
			1.25 - a_y
		) / delta_y;
		if(
			coef_ang >= 0 && coef_ang <= 1
		){
			
			float intersecao_x = fabsf(
				a_x + delta_x * coef_ang
			);
			
			if(
				intersecao_x >= 15 && intersecao_x <= 15.75
			){
				/*
				Há colisão
				*/
				return true;
			}
		}
		
		coef_ang = (
			- 1.25 - a_y
		) / delta_y;
		if(
			coef_ang >= 0 && coef_ang <= 1
		){
			
			float intersecao_x = fabsf(
				a_x + delta_x * coef_ang
			);
			
			if(
				intersecao_x >= 15 && intersecao_x <= 15.75
			){
				/*
				Há colisão
				*/
				return true;
			}
			
		}
	

	}

	return false;
}


inline void 
adicionar_espaco_de_amortecimento(
	// A sutileza é que não estamos fornecendo um ponteiro para array,
	// mas o vetor. Note que apesar de serem trabalhados iguais no código,
	// possuem valores semânticos diferentes.
	float quadro_de_custo[]
){
	/*
	Descrição:
		Adiciona espaço de amortecimento perto das linhas centrais e finais
		a fim de evitar colisões com as bordas do campo. 
		
		É como se estivessemos criando um campo de potencial a fim de permitir
		comportamentos mais suaves.
		
		Objetivo: criar áreas de custo gradual ao redor das áreas citadas.
		
		TAMANHO_DO_AMORTECIMENTO trata-se da diminuição gradativa de 6 para 1,
		onde 6 é o mais perto e 1 é o mais distante.
	*/
	
	// linha do gol inimigo
	for(
		int i = 0;
		i < TAMANHO_DO_AMORTECIMENTO;
		i++
	){
		int ii = (i + 12) * QUANT_COLUNAS;
		
		for(
			int j = 12 + i;
			j < (209 - i);
			j++
		){
			quadro_de_custo[
				j + ii
			] = TAMANHO_DO_AMORTECIMENTO - i;
		}
	}
	
	// Nosso gol
	for(
		int i = 0;
		i < TAMANHO_DO_AMORTECIMENTO;
		i++
	){
		int ii = (308 - i) * QUANT_COLUNAS;
		
		for(
			int j = 12 + i;
			j < 99;
			j++
		){
			
			quadro_de_custo[
				j + ii
			] = TAMANHO_DO_AMORTECIMENTO - i;
			
			quadro_de_custo[
				220 + ii - j
			] = TAMANHO_DO_AMORTECIMENTO - i;
		}
	}
	
	// Linhas laterais
	for(
		int i = 0;
		i < TAMANHO_DO_AMORTECIMENTO;
		i++
	){
		for(
			int j = (13 + i) * QUANT_COLUNAS;
			j < (308 - i) * QUANT_COLUNAS;
			j += QUANT_COLUNAS
		){
			
			quadro_de_custo[
				i + j + 12	
			] = TAMANHO_DO_AMORTECIMENTO - i;
			
			quadro_de_custo[
				j - i + 208
			] = TAMANHO_DO_AMORTECIMENTO - i;
		}
	}
}


bool 
se_caminho_esta_obstruido(
	// Pontos
	float start_x, float start_y,
	float end_x, float end_y,
	float obstaculos_dados[],
	int   quantidade_de_obstaculos_dados,
	bool  ir_ao_gol,
	int   limite_para_qual_custo_eh_impossivel,
	float quadro_de_custos[]
){
	/*
	Descrição:
		Verifica se um caminho completo está obstruído.
		
		O caminho está obstruído se intercepta qualquer circunferência 
		HARD ou SOFT ou se começa dentro de circuferência HARD.
		
		Os números escolhidos não são aleatórios, tem haver com proporções
		geométricas.
	
	Retorno:
		False - Se não for necessário que A* resolva esse problema.
	*/
	
	/////////////////////////////////////////////////////////////////////////
	/// Definições Importantes
	/////////////////////////////////////////////////////////////////////////
	
	start_x = max(
		-16.f,
		min(
			start_x,
			16.f
		) // limita para que seja menor que 16.f
	); // Limita para que seja maior que -16.f
	
	start_y = max(
		-11.f,
		min(
			start_y,
			11.f
		)
	);
	
	int start_x_linha  = x_para_linha( start_x );
	int end_x_linha    = x_para_linha( end_x   );
	int start_y_coluna = y_para_col  ( start_y );
	int end_y_coluna   = y_para_col  ( end_y   );
	
	float start_custo = quadro_de_custos[
		start_x_linha * QUANT_COLUNAS + start_y_coluna
	];
	
	float end_custo   = quadro_de_custos[
		end_x_linha   * QUANT_COLUNAS + end_y_coluna
	];
	
	/////////////////////////////////////////////////////////////////////////
	/// Aplicação de Algoritmo
	/////////////////////////////////////////////////////////////////////////
	
	/*
	Caso o caminho tenha start e end iguais ou adjacentes, será trivial.
	*/
	bool se_esta_perto = abs(
		start_x_linha  - end_x_linha
	) <= 1 and        abs(
		start_y_coluna - end_y_coluna
	) <= 1;
	
	/*
	Permite que A* segure o abacaxi se a posição start é inacessível ou
	fora dos limites de borda não é permitido ou está bem perto do limite 
	de borda ou não
	*/
	if(
		(
			start_custo <= limite_para_qual_custo_eh_impossivel
		) || (
			!se_esta_perto and start_custo > 0
		)
	){
		return true;
	}
	
	/*
	Se desejarmos ir ao gol, devemos verificar se há colisão com gol, usamos A*.
	*/
	if(
		ir_ao_gol
	){
		
		end_x = 15.2;
		end_y = max(
			-0.8f,
			min(
				start_y, 0.8f
			)
		);
	}
	else{
		
		/*
		Restringimos para que o final esteja no mapa.
		*/
		end_x = max(
			-16.f,
			min(
				start_y,
				16.f
			)
		);
		
		end_y = max(
			-11.f,
			min(
				start_y,
				11.f
			)
		);
		
		// Mesmo motivo do if que é igual anteriormente.
		if(
			(
				end_custo <= limite_para_qual_custo_eh_impossivel
			) || (
				!se_esta_perto and end_custo > 0
			)
		){
			return true;
		}		
	}
	
	if(
		se_a_linha_ab_intercepta_qualquer_gol(
			start_x,
			start_y,
			end_x,
			end_y
		)
	){
		return true;
	}
		
	float obstaculos[
		quantidade_de_obstaculos_dados * 4 / 5 + 16
	] = {
		/*
		Lista de todos os obstáculos
			Traves do Gol + Obstáculos Dados
			
		Note que incluindo as traves do gol provemos nenhum espaço de 
		amortecimento, o qual poderia ser necessário.
		
		Parâmetros como as numerações de obstáculos serão apresentados:
			
			Localização da Trava do Gol (x, y)
			'''
			Miguel Abreu:
				Goal post location (tested in simulator, collision with robot,
				robot is sideways to the goal post and arms are close to body)
			'''
			
			raio_HARD:
			'''
			Miguel Abreu:
				Hard radius (tested in the same way, robot collides when 
				closer than 0.15m from goal post border)
				post radius 0.02 + agent radius 0.15 = 0.17
			'''
			
			maior_raio:
			'''
				largest radius (equivalent to hard radius, since there is no
				soft radius).
			'''
		*/
		 15.02,  1.07, 0.17, 0.17,
		
		 15.02, -1.07, 0.17, 0.17,
		
		-15.02,  1.07, 0.17, 0.17,
		
		-15.02, -1.07, 0.17, 0.17,
	};  // x, y, raio_HARD, maior_raio
	
	int quantidade_de_obstaculos_totais = 16;
	
	for(
		int index = 0;
		index < quantidade_de_obstaculos_dados;
		index += 5
	){
		obstaculos[
			quantidade_de_obstaculos_totais++
		] = obstaculos_dados[
			index
		]; // x
		
		obstaculos[
			quantidade_de_obstaculos_totais++
		] = obstaculos_dados[
			index + 1
		]; // y
		
		obstaculos[
			quantidade_de_obstaculos_totais++
		] = fmaxf(
			0,
			fminf(
				obstaculos_dados[
					index + 2
				],
				DIST_MAX
			)
		); // Raio Hard
		
		obstaculos[
			quantidade_de_obstaculos_totais++
		] = fmaxf(
			0, 
			fminf(
				max(
					obstaculos_dados[
						index + 2
					],
					obstaculos_dados[
						index + 3
					]
				),
				DIST_MAX
			)
		);  // Maior Raio
		
		
	}
	
	/////////////////////////////////////////////////////////////////////
	/// Verificações de Obstrução
	/////////////////////////////////////////////////////////////////////
	
	if(
		// Caso Especial: Caminho é obstruído se começa ou termina em área HARD.
		se_esta_perto
	){
		for(
			int ob = 0;
			ob < quantidade_de_obstaculos_totais;
			ob += 4
		){
			float center_x = obstaculos[ ob     ];  // Centro_x
			float center_y = obstaculos[ ob + 1 ];  // Centro_y
			
			float raio_hard = obstaculos[ ob +2 ]; 
			
			/*
			Começo para Centro
			*/
			float sc_x = center_x - start_x;
			float sc_y = center_y - start_y;
			
			/*
			Final para Centro
			*/
			float ec_x = center_x -  end_x;
			float ec_y = center_y -  end_y;
			
			if(
				/*
				Calculos se o inicio ou se o final está dentro da área.
				Observe que não utilizamos sqrt.
				*/
				(
					(sc_x * sc_x + sc_y * sc_y) <= raio_hard * raio_hard
				) || (
					(ec_x * ec_x + ec_y * ec_y) <= raio_hard * raio_hard
				)
			){
				return true;
			}
			
		}
	}
	else{
		// Caso Normal: Caminho é obstruído se intercepta obstáculo
		
		for(
			int ob = 0;
			ob < quantidade_de_obstaculos_totais;
			ob += 4
		){
			float center_x   = obstaculos[ ob    ];
			float center_y   = obstaculos[ ob + 1];
			float maior_raio = obstaculos[ ob + 3];
			
			/*
			Começo para Centro
			*/
			float sc_x = center_x - start_x;
			float sc_y = center_y - start_y;
			
			/*
			Começo para Final
			*/
			float se_x = end_x    - start_x;
			float se_y = end_y    - start_y;
			
			/*
			escala = comprimento da projeção / comprimento do vetor alvo
			*/
			float proj_sc_escala = (
				// produto escalar, vê?
				sc_x * se_x + sc_y * se_y
			) / (
				// Módulo do vetor de começo -> final.
				se_x * se_x + se_y * se_y
			);
			
			/*
			Projeção do vetor começo -> centro sob o vetor começo -> final.
			*/
			float proj_sc_x = se_x * proj_sc_escala;
			float proj_sc_y = se_y * proj_sc_escala;
			
			// Fazemos isso para impedir divisões por 0.
			float parametro = abs( se_x ) > abs( se_y ) ? proj_sc_x / se_x : proj_sc_y / se_y;
			
			if(
				parametro <= 0
			){
				if(
					// Verifica distância do centro do obstaculo e começo.
					sc_x * sc_x + sc_y * sc_y <= maior_raio * maior_raio 
				){
					return true;
				}
			}
			else{
				if(
					parametro >= 1
				){
					float ec_x = center_x - end_x;
					float ec_y = center_y - end_y;
					
					if(
						// Verifica distância do centro do obstáculo e final.
						ec_x * ec_x + ec_y * ec_y <= maior_raio * maior_raio
					){
						return true;
					}
				}
				else{
					// Verificamos distância do centro e da projeção.
					
					float proj_c_x = center_x - ( proj_sc_x + start_x );
					float proj_c_y = center_y - ( proj_sc_y + start_y );
					
					if(
						proj_c_x * proj_c_x + proj_c_y * proj_c_y <= maior_raio * maior_raio
					){
						return true;
					}
				}
			}
		}
	}
	
	float delta_x = end_x - start_x;
	float delta_y = end_y - start_y;
	
	tamanho_do_caminho_final = 6;
	caminho_final[ 0 ] = start_x;
	caminho_final[ 1 ] = start_y;
	caminho_final[ 2 ] =   end_x;
	caminho_final[ 3 ] =   end_y;
	caminho_final[ 4 ] =       3;  // Sem obstáculos.
	/*
	min. A* cost from start to end (e_cost is added even if start==end
	to help debug cell costs)
	*/
	caminho_final[ 5 ] = sqrtf( delta_x * delta_x + delta_y * delta_y ) + max( 0.f, end_custo / 10.f );
	
	return false;  // Sem obstruções.
}


void
a_estrela(
	float parametros[],
	int quantidade_de_parametros
){
	/*
	Descrição:
		Responsável por computar o melhor caminho possível.
		
	Parâmetros:
		parametros:
		
			[start x][start y]
			[allow out of bounds?][go to goal?]
			[optional target x][optional target y]
			[timeout]
			[x][y][hard radius][soft radius][force]
	*/
	
	//////////////////////////////////////////////////////////////////////
	/// Extração de Informações dos Argumentos
	//////////////////////////////////////////////////////////////////////
	
	// Auto permite que nós não nos importemos com o tipo fornecido.
	// Pq apesar de saber que é uma unidade de tempo, o ngc é louco.
	auto inicio = high_resolution_clock::now();
	
	const float start_x = parametros[ 0 ];
	const float start_y = parametros[ 1 ];
	
	const bool permissao_para_ir_para_alem_dos_limites = parametros[2];
	const int limite_para_qual_custo_eh_impossivel = permissao_para_ir_para_alem_dos_limites ? - 3 : - 2;
		
	const bool ir_ao_gol = parametros[ 3 ];
	
	const float alvo_opcional_x = parametros[ 4 ];
	const float alvo_opcional_y = parametros[ 5 ];
	
	const int tempo_limite = parametros[ 6 ];  // para limitarmos o tempo de busca.
	
	float *obstaculos = parametros + 7;  // Aritmética de Ponteiros
	int quantidade_de_obstaculos = quantidade_de_parametros - 7;

	float quadro_de_custo[
		QUANT_LINHAS * QUANT_COLUNAS
	] = {
		/*
		Definindo o campo de jogo.
		Não ousei tentar compreender isso ainda, entretanto, é importante
		pra caramba dado que explicita 
		*/
		L0_1,
		L2_5,
		L6_10,
		L11,
		LIN12_308,
		L309,
		L310_314,
		L2_5,
		L0_1
	};

	if(
		!permissao_para_ir_para_alem_dos_limites
	){
		adicionar_espaco_de_amortecimento(
			quadro_de_custo
		);
	}
	
	if(
		!se_caminho_esta_obstruido(
			start_x,
			start_y,
			alvo_opcional_x,
			alvo_opcional_y,
			obstaculos,
			quantidade_de_obstaculos,
			ir_ao_gol,
			limite_para_qual_custo_eh_impossivel,
			quadro_de_custo
		)
	){
		// Força retornar void
		return;
	}

	const int start_linha = x_para_linha(
		start_x
	);
	const int start_coluna = y_para_col(
		start_y
	);
	const int start_pos = start_linha * QUANT_COLUNAS + start_coluna;
	
	////////////////////////////////////////////////////////////////////
	/// Definimos objetivo.
	////////////////////////////////////////////////////////////////////
	
	int end_linha  = 0;
	int end_coluna = 0;
	if(
		!ir_ao_gol
	){
		end_linha  = x_para_linha( alvo_opcional_x );
		end_coluna = y_para_col  ( alvo_opcional_y );
	}
	else{
		end_linha = LINHA_DO_GOL;
	}
	
	////////////////////////////////////////////////////////////////////
	/// Limites de Borda
	////////////////////////////////////////////////////////////////////
	
	int linha_min = min( start_linha, end_linha );
	int linha_max = min( start_linha, end_linha );
	
	int coluna_min = 0;
	int coluna_max = 0;
	
	if(
		ir_ao_gol
	){
		coluna_min = min( start_coluna,        119 );
		coluna_max = max( start_coluna, 	   101 );
	}
	else{
		coluna_min = min( start_coluna, end_coluna );
		coluna_max = max( start_coluna, end_coluna );
	}
	
	if(
		/*
		Devemos limitar mais caso não seja permitir sair, para desincentivar
		ainda mais a saída.
		*/
		!permissao_para_ir_para_alem_dos_limites
	){
		linha_min  = min( linha_min,  306         );
		linha_max  = max( 14,         linha_max   );
		coluna_min = min( coluna_min, 206         );
		coluna_max = max( 14,  		  coluna_max  );
	}
	
	////////////////////////////////////////////////////////////////////
	/// Algoritmo A*
	////////////////////////////////////////////////////////////////////
	
	Node* raiz_da_estrutura = nullptr;
	Node quadro_de_possibilidades[
		QUANT_COLUNAS * QUANT_LINHAS
	];
	
	unsigned int estado_dos_nodes[
		QUANT_LINHAS * QUANT_COLUNAS
	] = {
		/*
		0 -> Desconhecido
		1 -> Aberto
		2 -> Fechado
		*/
		0	
	};  
	
	///////////////////////////////////////////////////////////////////
	/// Populamos com obstáculos
	///////////////////////////////////////////////////////////////////
	
	for(
		int ob = 0;
		ob < quantidade_de_obstaculos;
		ob += 5
	){
		int linha = x_para_linha( obstaculos[ ob    ] );
		int coluna = y_para_col ( obstaculos[ ob + 1] );
		
		float raio_hard = fmaxf(
			0,
			fminf(
				obstaculos[ ob + 2 ],
				DIST_MAX
			)
		);
		float raio_soft = fmaxf(
			0,
			fminf(
				obstaculos[ ob + 3 ],
				DIST_MAX
			)
		);
		
		float forca = obstaculos[ ob + 4 ];
		float forca_por_metro = forca / raio_soft;  // Rapaz, apenas para facilitar posteriores contas
		
		int raio_max = int(
			fmaxf(
				raio_hard,
				raio_soft
			) * 10.f + 1e-4
			/*
			A fim de aprimorar velocidade, não usaremos a função pow(10, -4).
			
			Oq esse valor significa?
				É um delta para evitar possíveis erros em contas posteriores.	
			*/
		);
		linha_min  = min( linha_min,  linha  - raio_max - 1 );
		linha_max  = max( linha_max,  linha  + raio_max + 1 );
		coluna_min = min( coluna_min, coluna - raio_max - 1 );
		coluna_max = max( coluna_max, coluna + raio_max + 1 );
		
		
		////////////////////////////////////////////////////////////////////
		/// Custo a partir dos raios de contato.
		////////////////////////////////////////////////////////////////////
		
		/*
		Fazemos atualizações do custo a partir dos valores que estão em
		obtendo_possibilidades.h
		
		Como acredito que você pode não ter visto o código referente à
		obtendo_possibilidades.py, lhe darei outra explicação.
		
		quantidade_de_pontos_disponiveis
			
			quantidade de pontos que foram contabilizados ao redor do agente.
		
		aneis_e_pontos_disponiveis
			
			raio dos aneis de cada ponto disponível. Por exemplo, um ponto
			(linha, coluna) possui o valor de sua distância registrada nesse 
			array.
			
		linhas_de_cada_ponto
		colunas_de_cada_ponto
		
			Respectivamente, as linhas e colunas de cada ponto em arrays 
			unidimensionais.
		*/
		int index = 0;
		while(
			(
				index < quantidade_de_pontos_disponiveis
			) && (
				aneis_e_pontos_disponiveis[index] <= raio_hard
			)
		){
			int coef_linha  = linha  + linhas_de_cada_ponto [ index ];
			int coef_coluna = coluna + colunas_de_cada_ponto[ index ];
			
			if(
				(
					coef_linha  >= 0
				) && (
					coef_coluna >= 0
				) && (
					coef_linha  <  QUANT_LINHAS
				) && (
					coef_coluna <  QUANT_COLUNAS
				)
			){
				quadro_de_custo[
					coef_linha * QUANT_COLUNAS + coef_coluna
				] = - 3;
			}
			
			index++;
		}
		
		while(
			(
				index < quantidade_de_pontos_disponiveis
			) && (
				aneis_e_pontos_disponiveis[index] <= raio_soft
			)
		){
			int coef_linha  = linha  + linhas_de_cada_ponto [ index ];
			int coef_coluna = coluna + colunas_de_cada_ponto[ index ];
			
			float custo_anteriormente_calculado = quadro_de_custo[
				coef_linha * QUANT_COLUNAS + coef_coluna
			];
			
			// Apenas um fator para ver se vale a pena tocar pelo raio_soft
			float fr = forca - (forca_por_metro * aneis_e_pontos_disponiveis[index]);
			
			if(
				(
					coef_linha  >= 0
				) && (
					coef_coluna >= 0
				) && (
					coef_linha  <  QUANT_LINHAS
				) && (
					coef_coluna <  QUANT_COLUNAS
				) && (
					custo_anteriormente_calculado > limite_para_qual_custo_eh_impossivel
				) && (
					custo_anteriormente_calculado < fr
				)
			){
				quadro_de_custo[
					coef_linha * QUANT_COLUNAS + coef_coluna
				] = fr;
			}
			
			index++;
		}
		
	}
	
	///////////////////////////////////////////////////////////////////
	/// Ajustes finos
	///////////////////////////////////////////////////////////////////
	
	/*
	Ajustamos limites de borda caso a área disponível esteja sobrepondo
	área dos gols.
	
	Os valores a seguir são específicos da geometria do campo.
	*/
	if(
		coluna_max > 96 and coluna_min < 124
	){
		
		if(
			// Caso nosso gol esteja sendo sobreposto
			linha_max > 1 and linha_min < 12
		){
			linha_max  = max( 12,  linha_max  );
			linha_min  = min( 1,   linha_min  );
			coluna_max = max( 124, coluna_max );
			coluna_min = min( 96,  coluna_min );
		}
		
		if(
			// Caso seja o gol deles.
			linha_max > 308 and linha_min < 319
		){
			linha_max  = max( 319, linha_max  );
			linha_min  = min( 308, linha_min  );
			coluna_max = max( 124, coluna_max );
			coluna_min = min( 96,  coluna_min );
		}
	}
	
	///////////////////////////////////////////////////////////////////
	/// Adicionamos o objetivo
	///////////////////////////////////////////////////////////////////
	
	/*
	Caso o objetivo não esteja acessível, não adicionamos ao mapa.
	Embora sua referência ainda exista.
	
	Portanto, nós saberemos o quão longe estamos da referência, apesar dela
	ser inacessível. Mesmo se estivermos bem próximos ao objetivo, a ideia 
	é se afastar para atingir o ponto válido mais próximo que permitirá a 
	acessibilidade.
	*/
	
	if(
		!ir_ao_gol
	){
		float *custo_do_fim = &quadro_de_custo[
			end_linha * QUANT_COLUNAS + end_coluna
		];
		
		if(
			*custo_do_fim > limite_para_qual_custo_eh_impossivel
		){
			
			*custo_do_fim = -1;
		}
	}
	else{
		for(
			int index = LINHA_DO_GOL * QUANT_COLUNAS + 101;
			    index < LINHA_DO_GOL * QUANT_COLUNAS + 119;
			    index++
		){
			if(
				quadro_de_custo[index] > limite_para_qual_custo_eh_impossivel
			){
				quadro_de_custo[index] = -1;
			}
		}
	}
	
	// Adicionamos novos limites de borda para restringir ainda mais espaço
	linha_max  = min( 320, linha_max);
	linha_min  = max( 0,   linha_min);
	coluna_max = min( 220, coluna_max);
	coluna_min = max( 0,   coluna_min);
	
	// Iniciamos o primeiro nó.
	// Fazemos um mínimo para nos basear a partir dele
	quadro_de_possibilidades[start_pos].custo_pontual = 0;
	quadro_de_possibilidades[start_pos].parente = nullptr;  
	raiz_da_estrutura = noding::inserir( &quadro_de_possibilidades[ start_pos ], raiz_da_estrutura );
	Node* melhor_node = &quadro_de_possibilidades[start_pos];
	float distancia_ao_melhor_node = std::numeric_limits<float>::max();  // Apenas dizemos que é a maior possível primeiro.
	
	/*
	Caso ultrapasse essa medida, deve parar imediatamente e ficar com oq 
	já tem.
	*/
	int medida_de_tempo_limite = 0;
	
	if(
		quadro_de_custo[start_pos] > limite_para_qual_custo_eh_impossivel
	){
		
		distancia_ao_melhor_node = distancia_diagonal(
			ir_ao_gol,
			start_linha,
			start_coluna,
			end_linha,
			end_coluna
		);
	}
	
	////////////////////////////////////////////////////////////////////
	/// Aplicação do Algoritmo
	////////////////////////////////////////////////////////////////////
	
	while(
		raiz_da_estrutura != nullptr
	){
		
		// Procuramos o próximo melhor nó.
		Node* node_atual = min_node;
		
		const int pos_atual = node_atual - quadro_de_possibilidades;
		const int linha_atual = pos_atual / QUANT_COLUNAS; 
		const int coluna_atual = pos_atual % QUANT_COLUNAS;
		const float custo_atual = quadro_de_custo[pos_atual];
		
		medida_de_tempo_limite = (medida_de_tempo_limite + 1) % 31; // Checar o tempo a cada 32 iterações
		if(
			custo_atual > limite_para_qual_custo_eh_impossivel
		){
			float dist_diagonal = distancia_diagonal(
				ir_ao_gol,
				linha_atual,
				coluna_atual,
				end_linha,
				end_coluna
			);
			
			if(
				// Achamos um novo melhor node
				distancia_ao_melhor_node > dist_diagonal
			){
				
				melhor_node = node_atual;
				distancia_ao_melhor_node = dist_diagonal;
			}
		}
		
		raiz_da_estrutura = noding::remover_min( raiz_da_estrutura );
		estado_dos_nodes[pos_atual] = 2;
		
		// Checamos se o objetivo foi atingido.
		if(
			custo_atual == -1
		){
			
			construir_caminho_final(
				melhor_node,
				quadro_de_possibilidades,
				0, // Significa sucesso
				!ir_ao_gol,
				alvo_opcional_x,
				alvo_opcional_y
			);
			
			return;
		}
		
		if(
			(
				medida_de_tempo_limite == 0
			) && (
				duration_cast<microseconds>( high_resolution_clock::now() - inicio).count() > tempo_limite
			)
		){
			
			construir_caminho_final(
				melhor_node,
				quadro_de_possibilidades,
				1 // Significa tempo excedido.
			);
			
			return;
		}
		
		/////////////////////////////////////////////////////////////////
		/// Expandimos nodes filhos
		/////////////////////////////////////////////////////////////////
		bool rcol_ok = coluna_atual < coluna_max;
		bool lcol_ok = coluna_atual > coluna_min;
		
		if(
			linha_atual > linha_min
		){
			int linha  = linha_atual  - 1;
			int coluna = coluna_atual - 1;
			
			int   posicao        = pos_atual - QUANT_COLUNAS - 1;
			float custo          = quadro_de_custo [  posicao  ];
			int   estado_do_node = estado_dos_nodes[  posicao  ];
			
			if(
				/*
				Checa se não é um obstáculo e se o node não está fechado.
				*/
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				) && (
					lcol_ok
				)
				
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					SQRT_2 // Extra
				);
			}
			
			coluna++;
			posicao++;
			custo          = quadro_de_custo [ posicao ];
			estado_do_node = estado_dos_nodes[ posicao ];
			
			if(
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				)
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					1 // Extra
				);
			}
			
			coluna++;
			posicao++;
			custo          = quadro_de_custo [ posicao ];
			estado_do_node = estado_dos_nodes[ posicao ];
			
			if(
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				) && (
					rcol_ok
				)
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					SQRT_2 // Extra
				);
			}
		}
		
		if(
			linha_atual < linha_max
		){
			int linha   = linha_atual  + 1;
			int coluna  = coluna_atual - 1;
			
			int   posicao        = pos_atual + QUANT_COLUNAS - 1;
			float custo          = quadro_de_custo [  posicao  ];
			int   estado_do_node = estado_dos_nodes[  posicao  ];
			
			if(
				/*
				Checa se não é um obstáculo e se o node não está fechado.
				*/
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				) && (
					lcol_ok
				)
				
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					SQRT_2 // Extra
				);
			}
			
			coluna++;
			posicao++;
			custo          = quadro_de_custo [ posicao ];
			estado_do_node = estado_dos_nodes[ posicao ];
			
			if(
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				)
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					1 // Extra
				);
			}
			
			coluna++;
			posicao++;
			custo          = quadro_de_custo [ posicao ];
			estado_do_node = estado_dos_nodes[ posicao ];
			
			if(
				(
					estado_do_node != 2
				) && !(
					custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
				) && (
					rcol_ok
				)
			){
				raiz_da_estrutura = noding::expandir_filho(
					raiz_da_estrutura,
					custo,
					limite_para_qual_custo_eh_impossivel,
					node_atual,
					quadro_de_possibilidades,
					posicao,
					estado_do_node,
					ir_ao_gol,
					linha,
					coluna,
					end_linha,
					end_coluna,
					estado_dos_nodes, 
					SQRT_2 // Extra
				);
			}
		}
		
		int coluna  = coluna_atual - 1;
		int posicao = pos_atual    - 1;
		float custo          = quadro_de_custo [ posicao ];
		int   estado_do_node = estado_dos_nodes[ posicao ];
		
		if(
			/*
			Checa se não é um obstáculo e se o node não está fechado.
			*/
			(
				estado_do_node != 2
			) && !(
				custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
			) && (
				lcol_ok
			)
			
		){
			raiz_da_estrutura = noding::expandir_filho(
				raiz_da_estrutura,
				custo,
				limite_para_qual_custo_eh_impossivel,
				node_atual,
				quadro_de_possibilidades,
				posicao,
				estado_do_node,
				ir_ao_gol,
				linha_atual,
				coluna,
				end_linha,
				end_coluna,
				estado_dos_nodes, 
				1 // Extra
			);
		}
			
		coluna  += 2;
		posicao += 2;
		custo          = quadro_de_custo [ posicao ];
		estado_do_node = estado_dos_nodes[ posicao ];
		
		if(
			/*
			Checa se não é um obstáculo e se o node não está fechado.
			*/
			(
				estado_do_node != 2
			) && !(
				custo <= limite_para_qual_custo_eh_impossivel and custo < custo_atual
			) && (
				rcol_ok
			)
			
		){
			raiz_da_estrutura = noding::expandir_filho(
				raiz_da_estrutura,
				custo,
				limite_para_qual_custo_eh_impossivel,
				node_atual,
				quadro_de_possibilidades,
				posicao,
				estado_do_node,
				ir_ao_gol,
				linha_atual,
				coluna,
				end_linha,
				end_coluna,
				estado_dos_nodes, 
				1 // Extra
			);
		}
	}
	
	construir_caminho_final(
		melhor_node,
		quadro_de_possibilidades,
		2
	);
	
	return;
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

