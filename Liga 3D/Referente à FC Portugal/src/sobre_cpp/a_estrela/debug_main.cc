/*
Matheus Deyvisson, 2025

Os arquivos com extensão .cc (ou .cpp, .cxx, etc.) são
arquivos de código-fonte em C++. Eles contêm a implementação
de funções, classes e lógica do programa, que depois são
compilados para gerar executáveis ou bibliotecas (como .so
no Linux ou .dll/.pyd no Windows).

Oq eu entendi: Apenas Código C++ para linux.
*/

#include "a_estrela.h"
#include <chrono>
#include <iostream>

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

int main(){

    inicio = high_resolution_clock::now();

    a_estrela(parametros, quantidade_de_parametros);

    fim = high_resolution_clock::now();

    std::cout << duration_cast<microseconds>(fim - inicio).count() << "us (inclui valores de inicialização.\n";

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    inicio = high_resolution_clock::now();

    a_estrela(parametros, quantidade_de_parametros);

    fim = high_resolution_clock::now();

    std::cout << duration_cast<microseconds>(fim - inicio).count() << "us.\n";

}

















