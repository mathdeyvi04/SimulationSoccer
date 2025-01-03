#include "B_FuncoesFront.h"

Display
inicializar_display(
	void
){
	/*
	Descrição:
		Função responsável por verificar e inicializar a janela e o renderizador.
		Como são estruturas que precisamos juntas, é mais interessante que usemos 
		uma struct da união delas.
	
	Retorno:
		Display em condições de ser usada, sendo este composto pela janela e renderizador;
		Display com renderizador NULL, caso apenas este tenha dado erro.
		Display NULO.
	*/
	
	Display resultado = {
		NULL,
		NULL
	};
	
	if(
		SDL_Init(
			SDL_INIT_EVERYTHING
		) != 0
	){
		apresentar_erro(
			"Houve erro ao inicializar ferramentas.\n"
		);
		
		return resultado;
	}
	
	SDL_Window *janela = SDL_CreateWindow(
		"Minha Janela",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		COMPRIMENTO_TELA,
		ALTURA_TELA,
		SDL_WINDOW_SHOWN
	);
	
	if(
		!janela
	){
		apresentar_erro(
			"Houve erro ao tentar criar janela.\n"
		);
		
		return resultado;
	}
	
	resultado.janela = janela;
	
	SDL_Renderer *renderizador = SDL_CreateRenderer(
		janela,  // À qual janela vamos anexá-lo.
		-1,  // Qual o código do display que vamos usar, -1 default.
		0  // Não desejamos características especiais.
	);
	
	if(
		!renderizador
	){
		apresentar_erro(
			"Houve erro ao tentar criar renderizador."
		);
		
		return resultado;
	}
	
	resultado.renderizador = renderizador;
	
	return resultado;
}


int
destruir_display(
	Display display
){
	/*
	Descrição:
		Função responsável por fechar corretamente o display.
	
	Parâmetros:
		Autoexplicativo.
	
	Retorno:
		Encerramento do display.
	*/
	
	
	if (
		display.renderizador != NULL
	){
		SDL_DestroyRenderer(
			display.renderizador
		);
	}
	
	if (
		display.janela != NULL
	){
		SDL_DestroyWindow(
			display.janela
		);
	}
	
	SDL_Quit();
	
	printf("\nEncerrado com sucesso!");
	
	return 0;
}


void 
entrada_de_usuario(){
	/*
	Descrição:
		Função responsável por gerenciar o input do
		usuário.
	
	Parâmetros:
		
	Retorno:
		Chamada das respectivas funções de entrada.
	*/
}


void
atualizacoes_de_estado(){
	/*
	Descrição:
		Função responsável por gerenciar as respectivas
		atualizações de cada um dos membros da simulação.
	
	Parâmetros:
		
	Retorno:
		Membros da simulação atualizados de forma multiprocessada.
	*/	
}


void
render(){
	/*
	Descrição:
		Função responsável por gerenciar as atualizações na tela.
	
	Parâmetros:
		
	Retorno:
		Tela atualizada conforme as necessidades de renderização.
	*/
}


