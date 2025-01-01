#include "A_Simulando.h"

int
main(
	/*
	Quantos argumentos foram passados.
	Sendo que o nome do .exe já conta como 1 argumento.
	*/
	int argc,
	/*
	Contém os argumentos passados. 
	Cada elemento do vetor é uma string representando um argumento.
	*/
	char* argv[]
){
	Display display = inicializar_display();
	
	if (
		(display.janela == NULL) || (display.renderizador == NULL)
	){
		apresentar_erro(
			"Houve erro na inicialização da janela."
		);
		return 1;
	}
	
	destruir_display(
		display
	);
    
    printf("\n\n");
	system("pause");
    return 0;
}

