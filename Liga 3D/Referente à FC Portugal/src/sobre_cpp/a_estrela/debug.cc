#include "a_estrela.cpp"
#include <iostream>  // Apenas para realizarmos testes.
#include <cstdio>
/*
Arquivo de Testes Exclusivamente Para Testar Funcionalidades.

É aconselhável que use valgrind para informações de leaks.
*/

// namespace noding {

// 	int
// 	apresentar_recursivamente(
// 		Node* raiz,
// 		/*
// 		const std::string&
			
// 			É uma referência constante a um objeto do tipo std::string. Usá-la
// 			garante eficiência para o caso de darmos strings grandes demais,
// 			pois o endereço de memória original será usado.
			
// 			O const serve apenas para indicar que não será modificado dentro
// 			da função.
// 		*/
// 		const std::string& prefixo,
// 		bool se_eh_esquerdo
// 	){
// 		/*
// 			Apresentará propriamente a BST.
// 		*/
// 		if(
// 			raiz == nullptr
// 		){
// 			return 0;
// 		}
		
// 		std::cout << prefixo;
		
// 		std::cout << (se_eh_esquerdo ? "|---" : "L---");  // Sim, eu sei, operador terciário, me perdoe.
		
// 		std::cout << (*raiz).valor << std::endl;
		
// 		apresentar_recursivamente(
// 			(*raiz).right,
// 			prefixo + (se_eh_esquerdo ? "|   " : "    "),
// 			true
// 		);
// 		apresentar_recursivamente(
// 			(*raiz).left,
// 			prefixo + (se_eh_esquerdo ? "|   " : "    "),
// 			false
// 		);

// 		return 0;
// 	}
	
	
// 	int apresentar(
// 		Node* raiz
// 	){
// 		/*
// 		Descrição:
// 			Apenas preenche os parâmetros corretamente, exigir isso do usuário
// 			é demais.
// 		*/
		
// 		apresentar_recursivamente(
// 			raiz,
// 			"",
// 			false
// 		);

// 		if (min_node != nullptr){

// 			printf("\nmin_node.valor = %lf", (*min_node).valor);
// 		}

// 		return 0;

// 	}
// }

// void teste_de_criacao(){

// 	printf("Performing Creation Test.");

// 	// Não faremos nada com a raiz.
// 	return;
// }

// void teste_de_insercao(){

// 	Node *raiz = new Node(5);

// 	printf("The node is alone, so we make it the root.\n");
// 	noding::inserir(raiz, nullptr);
// 	noding::apresentar(raiz);

// 	printf("\n\nAddition of node smaller than minimum node.\n");
// 	noding::inserir(new Node((*raiz).valor - 3), raiz);
// 	noding::apresentar(raiz);

// 	printf("\n\nAddition of node smaller than the root.\n");
// 	noding::inserir(new Node((*raiz).valor - 1), raiz);
// 	noding::apresentar(raiz);

// 	printf("\n\nAddition of node greater than the root.\n");
// 	noding::inserir(new Node((*raiz).valor + 1), raiz);
// 	noding::apresentar(raiz);

// 	noding::liberar(raiz);
// 	return;
// }

// void teste_de_remocao_de_min(){

// 	Node *raiz = new Node(5);

// 	printf("Creating the valid structure\n");
// 	noding::inserir(raiz, nullptr);

// 	noding::inserir(new Node((*raiz).valor - 3), raiz);

// 	noding::inserir(new Node((*raiz).valor + 1), raiz);

// 	noding::inserir(new Node((*raiz).valor - 1), raiz);

// 	noding::apresentar(raiz);

// 	printf("\n\nRemove Min_Node\n");
// 	noding::remover_min(raiz);
// 	noding::apresentar(raiz);

// 	noding::liberar(raiz);
// 	return;
// }

// void teste_de_remocao_de_node(){

// 	Node *raiz = new Node(11);

// 	printf("\nTestando caso do node como min_node.");
// 	noding::inserir(raiz, nullptr);
// 	noding::inserir(new Node((*raiz).valor - 1), raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);
// 	printf("\n");
// 	noding::remover_min(raiz);
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node sem filhos.");
// 	Node *exemplo_a_ser_eliminado = new Node((*raiz).valor + 1);
// 	noding::inserir(exemplo_a_ser_eliminado, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);
// 	noding::remover_node(exemplo_a_ser_eliminado, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node somente com filho direito.");
// 	exemplo_a_ser_eliminado = new Node((*raiz).valor + 1);
// 	noding::inserir(exemplo_a_ser_eliminado, raiz);
// 	noding::inserir(new Node((*raiz).valor + 5), raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);
// 	noding::remover_node(exemplo_a_ser_eliminado, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node somente com filho esquerdo sem ser a raiz.");
// 	exemplo_a_ser_eliminado = new Node((*raiz).valor + 3);
// 	noding::inserir(exemplo_a_ser_eliminado, raiz);
// 	noding::inserir(new Node((*raiz).valor + 2, raiz), raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);
// 	noding::remover_node(exemplo_a_ser_eliminado, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node somente com filho esquerdo sendo a raiz.");
// 	noding::liberar(raiz);
// 	raiz = new Node(10);
// 	noding::inserir(raiz, nullptr);

// 	Node *novo_node = new Node(5);
// 	noding::inserir(novo_node, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	// Isso de retornar o elemento me deixou mt no sanha
// 	raiz = noding::remover_node(raiz, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node com ambos filhos, sendo o filho esquerdo único.");
// 	exemplo_a_ser_eliminado = new Node(12);
// 	noding::inserir(exemplo_a_ser_eliminado, raiz);
// 	noding::inserir(new Node(15), raiz);
// 	noding::inserir(new Node(10), raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	noding::remover_node(exemplo_a_ser_eliminado, raiz);
// 	printf("\n");
// 	noding::apresentar(raiz);

// 	printf("\nTestando caso do node com ambos filhos, não tendo filho esquerdo único.");
// 	noding::liberar(raiz);
// 	raiz = new Node(10);
// 	noding::inserir(raiz, nullptr);
// 	exemplo_a_ser_eliminado = new Node(8);
// 	noding::inserir(exemplo_a_ser_eliminado, raiz);
// 	noding::inserir(new Node(9), raiz);
// 	noding::inserir(new Node(5), raiz);
// 	noding::inserir(new Node(8.5), raiz);

// 	printf("\n");
// 	noding::apresentar(raiz);

// 	noding::remover_node(exemplo_a_ser_eliminado, raiz);

// 	noding::liberar(raiz);
// 	return;
// }

// void 
// realizar_teste(
// 	void (*funcao_de_teste_a_ser_realizada)()
// ){
// 	/*
// 	Vamos centralizar testes.
// 	*/

// 	printf("\n\n");
// 	printf("\033[7m----------------------------\n");
// 	printf("-> Actions to be taken:\033[0m\n\n\n");


// 	funcao_de_teste_a_ser_realizada();
	

// 	printf("\n\n\n\033[7m-> End of Test Actions\n");
// 	printf("----------------------------\033[0m\n\n");
// }

/*
Testando a função principal!
*/
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

std::chrono::_V2::system_clock::time_point inicio, fim;

float parametros[] = {
    /*
    Não me atrevo a escrever outra coisa.
    */
    15.78,-0.07, //start
    1,1, //out of bounds? go to goal?
    0,0, //target (if not go to goal)
    500000, // timeout
    -10,0,1,5,5,
    -10,1,1,7,10,
    -10,-7,0,5,1
};

int quantidade_de_parametros = sizeof( parametros ) / sizeof(float);

void teste_de_funcao_principal_a_estrela(){

	inicio = high_resolution_clock::now();

    a_estrela(parametros, quantidade_de_parametros);

    fim = high_resolution_clock::now();

    printf("\033[7m%d\033[0mus -> Com Valores de Inicializacao.\n", (int)duration_cast<microseconds>(fim - inicio).count());

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    inicio = high_resolution_clock::now();

    a_estrela(parametros, quantidade_de_parametros);

    fim = high_resolution_clock::now();

    printf("\033[7m%d\033[0mus -> Sem inicializacao.\n", (int)duration_cast<microseconds>(fim - inicio).count());
}

int main()
{
	teste_de_funcao_principal_a_estrela();
		
	
   return 0;
}
