#include "F_FuncoesBasicas.h"

void
apresentar_erro(
	const char *texto_de_apresentacao_de_erro
){
	/*
	Descrição:
		Função responsável por apresentar o erro em vermelho.
	*/
	printf("\033[1;31m");
    printf("%s", texto_de_apresentacao_de_erro);   
    printf("\033[0m");   
}







