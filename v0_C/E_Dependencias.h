#ifndef H_Dependencias_H
#define H_Dependencias_H

//////////////////////////////////////////////////////////////////////////////////
//// Importações 
//////////////////////////////////////////////////////////////////////////////////

// Ferramentas de Teste
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <time.h> // For the srand()

// Importações de Interface
#include <SDL2/SDL.h>


// Importação de Multiprocessamento.
#include <pthread.h>

//////////////////////////////////////////////////////////////////////////////////
//// Variáveis De Tela
//////////////////////////////////////////////////////////////////////////////////

#define IMAGE_NAME "campo.bmp"
#define WIDTH_SCREEN 1200
#define HEIGHT_SCREEN 800
#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)  // in miliseconds.

// Borders Field Soccer
#define TOPLEFT_X 0.053 * WIDTH_SCREEN + 0.7 
#define TOPLEFT_Y 0.085 * HEIGHT_SCREEN + 0.8
#define BOTTOMRIGHT_X 0.94 * WIDTH_SCREEN + 0.2
#define BOTTOMRIGHT_Y 0.926 * HEIGHT_SCREEN + 0.8
#define MEDIUM_X WIDTH_SCREEN / 2 - 5
#define MEDIUM_Y HEIGHT_SCREEN / 2 + 5

#define NUMBER_OF_PLAYERS 7
#define N_DIMENSIONS 2

//////////////////////////////////////////////////////////////////////////////////
//// Structs
//////////////////////////////////////////////////////////////////////////////////
// To save screen information.
typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *image_field;
} Display;

#endif  // H_Dependencias_H
