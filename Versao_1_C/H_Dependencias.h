#ifndef H_Dependencias_H
#define H_Dependencias_H

///////////////////////////////// Importações ///////////////////////

// Ferramentas de Teste
#include <stdlib.h>
#include <stdio.h>

// Importações de Interface
#include <SDL2/SDL.h>


//////////////////////////////// Variáveis //////////////////////////

#define COMPRIMENTO_TELA 800
#define ALTURA_TELA 600

#define N_DIMENSIONS 2


#define QUANTIDADE_DE_PLAYERS 1



/////////////////////////////// Structs /////////////////////////////

// Para guardarmos as informações de tela.
typedef struct {
	SDL_Window *janela;
	SDL_Renderer *renderizador;
} Display;

#endif  // H_Dependencias_H
