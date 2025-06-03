/*
 *  Copyright (C) 2011 Justin Stoecker
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef ROBOVIZDRAW_H
#define ROBOVIZDRAW_H

/*
Importa-se <cstring> para manipular strings no estilo C, ou seja,
arrays de caracteres (char*) e funções como strcpy, snprintf e strcmp.

Importa-se <string> para usar a classe std::string do C++, que facilita
e torna mais segura a manipulação de textos, permitindo operações como
concatenação, comparação e busca de forma mais simples e moderna.
*/
#include <string>
#include <cstring>

using namespace std;

inline int 
escrever_caractere_no_buffer( unsigned char* buf, unsigned char valor ) {
    /*
    Descrição:
    	Escreve um caractere no buffer fornecido.

    Parâmetros:
    	- buf:Ponteiro para o buffer de destino.
    	- valor: Valor do caractere a ser escrito.

    Retorno:
    	Quantidade de bytes escritos (sempre 1).
    */

    *buf = valor;
    return 1;
}

inline int 
escrever_float_no_buffer( unsigned char* buf, float valor ) {
    /*
    Descrição:
    	Escreve um valor float formatado como texto (6 caracteres) no buffer.

    Parâmetros:
    	- buf: Ponteiro para o buffer de destino.
    	- valor: - Valor float a ser convertido e escrito.

    Retorno:
    	Quantidade de bytes escritos (sempre 6).
    */

	// Nem vou mexer aqui filho.
    char temp[20];
    sprintf(temp, "%6f", valor);
    memcpy(buf, temp, 6);
    return 6;
}

inline int 
escrever_cor_no_buffer( unsigned char* buf, const float* cor, int canais ) {
    /*
    Descrição:
    	Escreve uma cor no buffer, convertendo cada canal (float) para unsigned char no intervalo 0-255.

    Parâmetros:
    	- buf: Ponteiro para o buffer de destino.
    	- cor: Ponteiro para o array com os valores dos canais da cor (float de 0 a 1).
    	- canais: Quantidade de canais (exemplo: 3 para RGB, 4 para RGBA).

    Retorno:
    	Quantidade de bytes escritos (igual ao número de canais).
    */

    int i = 0;
    for(
    	i = 0; 
    	i < canais; 
    	i++
    )
        escrever_caractere_no_buffer(buf + i, (unsigned char) (cor[i]*255));

    return i;
}

inline int 
escrever_string_no_buffer( unsigned char* buf, const string* texto ) {
    /*
    Descrição:
    	Escreve uma string no buffer e adiciona o caractere nulo ('\0') ao final.

    Parâmetros:
    	- buf: Ponteiro para o buffer de destino.
    	- texto: - Ponteiro para o objeto string a ser copiado.

    Retorno:
    	Quantidade de bytes escritos (tamanho da string + 1 para o caractere nulo).
    */

    long int i = 0;

    if (texto != NULL){

        i += texto->copy((char*) buf + i, texto->length(), 0);
    }

    // Escrevemos o terminador nulo.
    i += escrever_caractere_no_buffer(buf + i, 0);  
    
    return i;
}

////////////////////////////////////////////////////////////////////////////////
/*
Buffer.
		
+--------+----------+-----------------------+------------+---------+
| Tipo   | Subtipo  | Dados do objeto       | Nome       |		   |
+--------+----------+-----------------------+------------+---------+
|  0x01  |  0x02    | ......                | "meu_nome" |  0x00   |
+--------+----------+-----------------------+------------+---------+

Os dois espaços vazios correspondem aos indicadores de controle, cabeçalho.
Indicam informações importantes, como quantidade total ou se pode ser preenchido 
com mais informações.

Tipo 1: 

	0 -> Buffer Vazio
	1 -> Buffer Preenchido Com Informações Geométricas
	2 -> Buffer Preenchido Com Informações Textuais

SubTipo 2:
	
	Para Informações Geométricas
	
		0 -> Círculo
		1 -> Linha
		2 -> Ponto
		3 -> Esfera
		4 -> Poligono
		
	Para Informações Textuais
		
		0 -> Anotação Simples
		1 -> Anotação de Agente com Texto
		2 -> Anotação de Agente sem Texto

*/

unsigned char* 
novo_buffer_swap( const string* nome, int* tamanho_buffer ) {
    /*
    Descrição:
        Cria um novo buffer para operação de swap, armazenando informações básicas como nome.

    Parâmetros:
        - nome:
            Ponteiro para a string que representa o nome (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações do swap.
    */

    *tamanho_buffer = 3 + ((nome != NULL) ? nome->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long int indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 0
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 0
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome
              );

    return buffer;
}

unsigned char* 
novo_circulo(
    const float*       centro, 
    float              raio, 
    float              espessura,
    const float*       cor,
    const std::string* nome_conjunto,
    int*               tamanho_buffer
) {
    /*
    Descrição:
        Cria um buffer representando um círculo, contendo informações como centro, raio, espessura, cor e nome do conjunto.

    Parâmetros:
        - centro:
            Ponteiro para array de floats representando as coordenadas do centro (x, y).
        - raio:
            Valor do raio do círculo.
        - espessura:
            Valor da espessura da borda do círculo.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual o círculo pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações do círculo.
    */
    *tamanho_buffer = 30 + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long int indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 0
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, centro[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, centro[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, raio
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, espessura
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 3
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome_conjunto
              );

    return buffer;
}

unsigned char* 
nova_linha(
    const float*       ponto_a, 
    const float*       ponto_b, 
    float              espessura,
    const float*       cor, 
    const std::string* nome_conjunto, 
    int*               tamanho_buffer
) {
    /*
    Descrição:
        Cria um buffer representando uma linha no espaço 3D, contendo vértices inicial e final, espessura, cor e nome do conjunto.

    Parâmetros:
        - ponto_a:
            Ponteiro para array de floats representando as coordenadas do ponto inicial (x, y, z).
        - ponto_b:
            Ponteiro para array de floats representando as coordenadas do ponto final (x, y, z).
        - espessura:
            Valor da espessura da linha.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual a linha pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações da linha.
    */
    *tamanho_buffer = 48 + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long int indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_a[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_a[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_a[2]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_b[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_b[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto_b[2]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, espessura
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 3
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome_conjunto
              );

    return buffer;
}

unsigned char*
novo_ponto(
    const float*       ponto, 
    float              tamanho, 
    const float*       cor,
    const std::string* nome_conjunto, 
    int*               tamanho_buffer
) {
    /*
    Descrição:
        Cria um buffer representando um ponto no espaço 3D, contendo suas coordenadas, tamanho, cor e nome do conjunto.

    Parâmetros:
        - ponto:
            Ponteiro para array de floats representando as coordenadas do ponto (x, y, z).
        - tamanho:
            Valor do tamanho do ponto.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual o ponto pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações do ponto.
    */
    *tamanho_buffer = 30 + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long int indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 2
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[2]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, tamanho
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 3
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome_conjunto
              );

    return buffer;
}

unsigned char* 
nova_esfera(
    const float*         ponto,
    float                raio,
    const float*         cor,
    const string*        nome_conjunto,
    int*                 tamanho_buffer
) {
    /**
    Descrição:
        Cria um buffer representando uma esfera no espaço 3D, contendo suas coordenadas, raio, cor e nome do conjunto.

    Parâmetros:
        - ponto:
            Ponteiro para array de floats representando as coordenadas do centro da esfera (x, y, z).
        - raio:
            Valor do raio da esfera.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual a esfera pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações da esfera.
    */
    *tamanho_buffer = 30 + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 3
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[2]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, raio
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 3
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome_conjunto
              );

    return buffer;
}

unsigned char* 
novo_poligono(
    const float*         vertices,
    int                  num_vertices,
    const float*         cor,
    const string*   	 nome_conjunto,
    int*                 tamanho_buffer
) {
    /**
    Descrição:
        Cria um buffer representando um polígono 3D, contendo seus vértices, cor e nome do conjunto.

    Parâmetros:
        - vertices:
            Ponteiro para array de floats representando os vértices do polígono (cada vértice possui 3 coordenadas x, y, z).
        - num_vertices:
            Número de vértices do polígono.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGBA).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual o polígono pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações do polígono.
    */
    *tamanho_buffer = 18 * num_vertices + 8 + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 1
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 4
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, num_vertices
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 4
              );

    for (int j = 0; j < num_vertices; j++) {
        indice += escrever_float_no_buffer(
                      buffer + indice, vertices[j * 3 + 0]
                  );
        indice += escrever_float_no_buffer(
                      buffer + indice, vertices[j * 3 + 1]
                  );
        indice += escrever_float_no_buffer(
                      buffer + indice, vertices[j * 3 + 2]
                  );
    }

    indice += escrever_string_no_buffer(
                 buffer + indice, nome_conjunto
             );

    return buffer;
}

unsigned char* 
nova_anotacao(
    const string*        texto,
    const float*         ponto,
    const float*         cor,
    const std::string*   nome_conjunto,
    int*                 tamanho_buffer
) {
    /**
    Descrição:
        Cria um buffer representando uma anotação em um ponto 3D, contendo texto, coordenadas, cor e nome do conjunto.

    Parâmetros:
        - texto:
            Ponteiro para string contendo o texto da anotação.
        - ponto:
            Ponteiro para array de floats representando as coordenadas do ponto (x, y, z).
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - nome_conjunto:
            Ponteiro para string com o nome do conjunto ao qual a anotação pertence (pode ser nulo).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações da anotação.
    */
    *tamanho_buffer = 25 + texto->length() + ((nome_conjunto != NULL) ? nome_conjunto->length() : 0);
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 2
              );
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 0
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[0]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[1]
              );
    indice += escrever_float_no_buffer(
                  buffer + indice, ponto[2]
              );
    indice += escrever_cor_no_buffer(
                  buffer + indice, cor, 3
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, texto
              );
    indice += escrever_string_no_buffer(
                  buffer + indice, nome_conjunto
              );

    return buffer;
}

unsigned char* 
nova_anotacao_agente(
    const string*   	 texto,
    bool                 time_esquerdo,
    int                  num_agente,
    const float*         cor,
    int*                 tamanho_buffer
) {
    /*
    Descrição:
        Cria um buffer representando uma anotação para um agente, podendo incluir ou não texto, o time, número do agente e cor.

    Parâmetros:
        - texto:
            Ponteiro para string contendo o texto da anotação (pode ser nulo).
        - time_esquerdo:
            Booleano indicando se o agente pertence ao time esquerdo.
        - num_agente:
            Número do agente.
        - cor:
            Ponteiro para array de floats representando os canais de cor (RGB).
        - tamanho_buffer:
            Ponteiro para inteiro onde será armazenado o tamanho do buffer criado.

    Retorno:
        Ponteiro para o buffer alocado com as informações da anotação do agente.
    */
    *tamanho_buffer = (texto == NULL) ? 3 : 7 + texto->length();
    unsigned char* buffer = new unsigned char[*tamanho_buffer];

    long indice = 0;
    indice += escrever_caractere_no_buffer(
                  buffer + indice, 2
              );

    if (texto == NULL) {
        indice += escrever_caractere_no_buffer(
                      buffer + indice, 2
                  );
        indice += escrever_caractere_no_buffer(
                      buffer + indice, (time_esquerdo ? num_agente - 1 : num_agente + 127)
                  );
    } else {
        indice += escrever_caractere_no_buffer(
                      buffer + indice, 1
                  );
        indice += escrever_caractere_no_buffer(
                      buffer + indice, (time_esquerdo ? num_agente - 1 : num_agente + 127)
                  );
        indice += escrever_cor_no_buffer(
                      buffer + indice, cor, 3
                  );
        indice += escrever_string_no_buffer(
                      buffer + indice, texto
                  );
    }

    return buffer;
}

#endif // ROBOVIZDRAW_H
