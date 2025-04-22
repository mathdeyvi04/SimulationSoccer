/*
Por respeito à genialidade, os direitos autorais:
	Copyright (C) 2011 Justin Stoecker
	
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
	
	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
	
Objetivo do Código:
	Desenvolver ferramentas que permitirão o desenho de formas poligonais e
	retas dentro do RoboViz.
	
	É interessante que vendo o código, é como se estivessemos escrevemos C.
*/
#ifndef ROBOVIZDRAW_H
#define ROBOVIZDRAW_H

#include <string>  // Biblioteca padrão de strings em C++
#include <cstdio>  // Versão da biblioteca <stdio.h> escrita em C++
#include <cstring> // Versão da biblioteca <string.h> escrita em C++

using namespace std;

#define QUANT_DE_CARAC_PARA_PRECISAO_DE_FLOATS 6
#define FORMAT_PARA_SPRINTF "%6f"

/////////////////////////////////////////////////////////////////////////
/// Funções Inline
////////////////////////////////////////////////////////////////////////////

inline int 
escrever_char_no_buffer(
	unsigned char *buffer,
	unsigned char  valor
){
	/*
	Apenas trazemos a responsabilidade de atribuição para esta função.
	*/
	
	*buffer = valor;
	return 1;
}


inline int 
escrever_float_no_buffer(
	unsigned char *buffer,
	float          valor
){
	/*
	Descrição:
		Houve um discussão séria sobre como construir essa função. 
		Se confirmou que estamos trabalhando em um nível baixo demais
		e não há necessidade de tratar o terminador nulo aqui, o que 
		impede o uso inteligente da função snprintf.
		
		Sendo assim, somos forçados a usar sprintf e memcpy.
		
	Retorno:
		Quantidade de caracteres do número que foi colocada no buffer.
	*/
	
	/*
	Escrevemos o float que pode ser absolutamente qualquer coisa
	em um buffer temporário.
	
	** Tentei usar um buffer temporário maior, mas o maldito warning de 
	overflow estava acabando comigo.
	
	Essa função faz o trabalho de formatação e de limitação do float.
	*/
	char temp[16];
	sprintf(
		temp,
		FORMAT_PARA_SPRINTF,
		valor
	);
	
	/*
	Garante a cópia em exatamente a quantidade de  bytes.
	*/
	memcpy(
		buffer, 
		temp,
		QUANT_DE_CARAC_PARA_PRECISAO_DE_FLOATS 
	);
	
	return QUANT_DE_CARAC_PARA_PRECISAO_DE_FLOATS;
}


inline int 
escrever_cor_no_buffer(
	unsigned char *buffer,
	const float   *cor,		 // Cor em RGB, lembrando que os números são menores que 1.
	int canais               // R = 0, G = 1, B = 2
){
	
	int index = 0;
	while(
		index < canais
	){
		
		escrever_char_no_buffer(
			buffer + index,
			(unsigned char) (cor[index] * 255)
		);
		
		index++;
	}
	
	// Em teoria, apenas a quantidade de canais.
	return index;
}


inline int
escrever_string_no_buffer(
	unsigned char *buffer,
	const string  *texto
){
	
	long int index = 0;  // Estaremos mexendo com stack, números serão grandes mesmo.
	
	if(
		texto != NULL
	){
		index += (*texto).copy(
								/*
								Colocamos + index para deixar em condições 
								de qualquer alteração acima.
								*/
								(char*) buffer + index,
								(*texto).length(),
								0
							  );
	}
	
	index += escrever_char_no_buffer(
		buffer + index,
		0 // colocamos o terminador nulo
	);
	
	return index;
}


////////////////////////////////////////////////////////////////////////////
/// Funções Amplas
////////////////////////////////////////////////////////////////////////////

/*
Cabeçalho 1: 

	0 -> Buffer Vazio
	1 -> Buffer Preenchido Com Informações Geométricas
	2 -> Buffer Preenchido Com Informações Textuais

Cabeçalho 2:
	
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

Para mais informações, veja a função criar_novo_buffer.

Cada buffer criado possui uma formatação diferente, já que armazena informações
diferentes e precisa armazená-las de forma inteligente.
*/

unsigned char* 
criar_buffer(
	const string *string_buffer,
	int          *tamanho_do_buffer
){
	/*
	Descrição:
		Criamos um buffer que será usado pelas funções anteriores.
	
	Retorno:
		Buffer.
		
		+-----+-----+---------------------+-----+
		| 0x00| 0x00|    string_buffer    | 0x00|
		+-----+-----+---------------------+-----+
		
		Os dois espaços vazios correspondem aos indicadores de controle, cabeçalho.
		Indicam informações importantes, como quantidade total ou se pode ser preenchido 
		com mais informações.
	*/
	
	// 1 slot para Terminador nulo do final
	// 2 slots para Cabeçalhos
	// String buffer
	*tamanho_do_buffer = 1 + 2 + ((string_buffer != NULL) ? (*string_buffer).length() : 0);
	
	// Aloca dinamicamente, logo devemos fazer o delete por fora.
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];  
	
	long int index = 0;
	
	index += escrever_char_no_buffer  ( buffer + index,             0 );  // Zeramos os cabeçalhos
	index += escrever_char_no_buffer  ( buffer + index,             0 );  // Zeramos os cabeçalhos
	index += escrever_string_no_buffer( buffer + index, string_buffer );  // 
	
	return buffer;
} 


unsigned char*
criar_circulo(
	const float  *centro,
	      float   raio,
	      float   grossura,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criamos um buffer para transportar informações de círculo.
	*/
	
	*tamanho_do_buffer = 30 + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	 
	i += escrever_char_no_buffer  ( buffer + i,                 1 );  
	i += escrever_char_no_buffer  ( buffer + i,                 0 );
	
	i += escrever_float_no_buffer ( buffer + i,         centro[0] );
	i += escrever_float_no_buffer ( buffer + i,         centro[1] );
	i += escrever_float_no_buffer ( buffer + i, 	         raio );
	i += escrever_float_no_buffer ( buffer + i,          grossura );
	
	i += escrever_cor_no_buffer   ( buffer + i,            cor, 3 );
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado );
	
	return buffer;
}


unsigned char*
criar_linha(
	const float  *inicio,
	const float  *final,
	      float   grossura,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criamos um buffer para transportar informações de linha.
	*/	
	*tamanho_do_buffer = 48 + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer(buffer + i, 1);
	i += escrever_char_no_buffer(buffer + i, 1);
	
	i += escrever_float_no_buffer( buffer + i, inicio[0] );
	i += escrever_float_no_buffer( buffer + i, inicio[1] );
	i += escrever_float_no_buffer( buffer + i, inicio[2] );
	i += escrever_float_no_buffer( buffer + i,  final[0] );
	i += escrever_float_no_buffer( buffer + i,  final[1] );
	i += escrever_float_no_buffer( buffer + i,  final[2] );
	
	i += escrever_float_no_buffer ( buffer + i,         grossura );
	i += escrever_cor_no_buffer   ( buffer + i,            cor, 3);
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado);
	
	return buffer;
}


unsigned char*
criar_ponto(
	const float  *ponto,
	      float   tamanho,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criando buffer para transportar informações de ponto.
	*/
	
	*tamanho_do_buffer = 30 + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer  ( buffer + i,        1 );
	i += escrever_char_no_buffer  ( buffer + i,        2 );
	
	i += escrever_float_no_buffer ( buffer + i, ponto[0] );
	i += escrever_float_no_buffer ( buffer + i, ponto[1] );
	i += escrever_float_no_buffer ( buffer + i, ponto[2] );
	i += escrever_float_no_buffer ( buffer + i, tamanho  );

	i += escrever_cor_no_buffer   ( buffer + i,            cor, 3);
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado);
	
	return buffer;
}


unsigned char*
criar_esfera(
	const float  *centro,
	      float   raio,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criar buffer para transportar informações de esfera.
	*/
	
	*tamanho_do_buffer = 30 + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer( buffer + i, 1 );
	i += escrever_char_no_buffer( buffer + i, 3 );

	i += escrever_float_no_buffer( buffer + i, centro[0] );	
	i += escrever_float_no_buffer( buffer + i, centro[1] );	
	i += escrever_float_no_buffer( buffer + i, centro[2] );	
	i += escrever_float_no_buffer( buffer + i,      raio );	
	
	i += escrever_cor_no_buffer   ( buffer + i,            cor, 3 );
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado );
	
	return buffer;
}


unsigned char*
criar_poligono(
	const float  *vertices,
	        int  numero_de_vertices,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criamos uma formatação para transportar informações de polígono
	em um buffer.
	*/
	
	*tamanho_do_buffer = 18 * numero_de_vertices + 8 + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer( buffer + i, 1 );
	i += escrever_char_no_buffer( buffer + i, 4 );
	
	i += escrever_char_no_buffer( buffer + i, numero_de_vertices );
	i += escrever_cor_no_buffer ( buffer + i,              cor, 4);
	
	for(
		int index = 0;
		index < numero_de_vertices;
		index++
	){
		
		i += escrever_float_no_buffer( buffer + index, vertices[ index * 3 + 0] );
		i += escrever_float_no_buffer( buffer + index, vertices[ index * 3 + 1] );
		i += escrever_float_no_buffer( buffer + index, vertices[ index * 3 + 2] );
	}
	
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado );
	
	return buffer;
}


unsigned char*
criar_anotacao(
	const string *texto,
	const float  *ponto,
	const float  *cor,
	const string *nome_a_ser_setado,
	        int  *tamanho_do_buffer
){
	/*
	Criamos um buffer para transportar informações de anotação e de texto.
	*/
	
	*tamanho_do_buffer = 25 + (*texto).length() + ((nome_a_ser_setado != NULL) ? (*nome_a_ser_setado).length() : 0);
	unsigned char *buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer( buffer + i, 2 );
	i += escrever_char_no_buffer( buffer + i, 0 );
	
	i += escrever_float_no_buffer( buffer + i, ponto[0] );
	i += escrever_float_no_buffer( buffer + i, ponto[1] );
	i += escrever_float_no_buffer( buffer + i, ponto[2] );
	
	i += escrever_cor_no_buffer   ( buffer + i,            cor, 3 );
	i += escrever_string_no_buffer( buffer + i,             texto );
	i += escrever_string_no_buffer( buffer + i, nome_a_ser_setado );
	
	return buffer;
}


unsigned char*
criar_anotacao_de_agente(
	const string *texto,
	 	   bool   left_team,         // inglês é bem melhor
	 	    int   numero_do_agente,
	const float  *cor,
	        int  *tamanho_do_buffer
){
	/*
	Em teoria, criará buffer para transportar informação de anotação de agente,
	por exemplo aquele textinho que fica em cima do agente.
	*/
	
	*tamanho_do_buffer = (texto != NULL) ? (7 + (*texto).length()) : 3;
	
	unsigned char* buffer = new unsigned char[*tamanho_do_buffer];
	
	long int i = 0;
	
	i += escrever_char_no_buffer( buffer + i, 2);
	
	if(
		texto == NULL
	){
		/*
		Note que o segundo cabeçalho é indicado com 2 e não há texto.
		*/
		i += escrever_char_no_buffer( buffer + i, 2 );
		i += escrever_char_no_buffer( buffer + i, (left_team ? numero_do_agente - 1 : numero_do_agente + 127));
	}
	else{
		/*
		Note que o segundo cabeçalho é indicado com 1 e há texto a ser colocado.
		*/
		i += escrever_char_no_buffer  ( buffer + i,      1 );
		i += escrever_char_no_buffer  ( buffer + i, (left_team ? numero_do_agente - 1 : numero_do_agente + 127));
		i += escrever_cor_no_buffer   ( buffer + i, cor, 3 );
		i += escrever_string_no_buffer( buffer + i, texto  );
	}
	
	return buffer;
}

#endif //ROBOVIZDRAW_H
