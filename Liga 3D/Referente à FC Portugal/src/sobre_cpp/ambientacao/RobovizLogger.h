/*
Manteremos os créditos por respeito.

 * Created on: 2013/06/24
 * Author: Rui Ferreira
 
//////////////////////////////////////////////////////////////////////////
 
Alterações foram feitas para aprimorar algoritmos e aumentar legibilidade.

POSSÍVEL ERRO QUE VOCÊ OBTER:

Bibliotecas usadas aqui estão unicamente disponíveis em sistemas UNIX.
logo, se você estiver baixado o GNU GCC Compiler MinGW para windows,
pode ter problema, já que não vai encontrar as respectivas bibliotecas.

Observe minha solução para o problema:

https://github.com/mathdeyvi04/RoboCup_RoboIME/issues/12

*/
#ifndef ROBOVIZLOGGER_H
#define ROBOVIZLOGGER_H

/*
As bibliotecas usada no original são as padrões de C.
Achei melhor aprimorá-las para C++.
*/
#include <cstdlib>
#include <cmath>
#include <cerrno>
#include <string>

/*///////////////////////////////////////////////////////////////////////////

Bibliotecas a seguir são bem mais específicas e não possuem 
suas respectivas versões em C++.

*///////////////////////////////////////////////////////////////////////////

/*
Interface com Sistema Operacional LINUX, conseguindo lidar com:

- chamadas diretas do sistema operacional;
- operações de entrada e de saída;
- controle de processos, sleep por exemplo;
- acesso a diretórios.
*/
#include <unistd.h>  

/*
Para tiparmos variáveis e ponteiros da melhor forma possível.
São struct amplamente desenvolvidas como:

- pid_t: Para ID de processos da CPU;
- size_t: Para dimensionar precisamente objetos;
- off_t: Para deslocamentos em arquivos.
*/
#include <sys/types.h>

/*//////////////////////////////////////////////////////////////////////////

As bibliotecas a seguir são focadas em programação de REDES em 
Sistemas UNIX/LINUX.

Juntas elas formam a base para comunicação TCP/UDP, permitindo desde simples
clientes até servidores complexos.

*///////////////////////////////////////////////////////////////////////////

/*
Criação e gerenciamento de sockets, os quais são canais de comunicação
entre processos locais e/ou remotos.

Focada em suportar protocolos:

- TCP: 
	Transmisson Control Protocol, comunicação ideal para aplicações que exigem
	confiabilidade acima de velocidade.

- UDP:
	User Datagram Protocol, comunicação ideal para aplicações
	que exigem velocidade acima de confiabilidade e integridade de dados.
*/
#include <sys/socket.h>

/*
Header fundamental para programação de redes via protocolo TCP.
Sendo focado em sistemas UNIX, possui definições essenciais para 
criação de sockets e manipulação de endereços de rede.

Contém MUITAS definições essenciais para manipulações de endereços IP
e portas, structs poderosas e constantes essenciais.

Acredito que, por questão de curiosidade e de brio, você também 
deva pelo menos passar os olhos neste arquivo de alta complexidade.

Caso esteja em Windows e não possa ver:
	https://github.com/leostratus/netinet/blob/master/in.h

*/
#include <netinet/in.h>

/*
Um pouco mais simples que a anterior, focada na conversão de 
endereços IP entre formatos legíveis para humanos, como 192.172.0.1,
e formatos binários utilizados nas funções que envolvem sockets.


Caso esteja em Windows e não possa ver, experimente:
	https://github.com/openbsd/src/blob/master/include/arpa/inet.h
*/
#include <arpa/inet.h>

/*
API de sockets em C, usada para resolver nomes de Host, serviços e
endereços de rede, funcionando como uma interface entre nomes simbólicos
e seus endereços númericos, como "educa.ime.eb.br" -> "192.120.1.2".

Caso esteja em Windows e não possa ver, experimente:
	https://github.com/openbsd/src/blob/master/include/netdb.h
*/
#include <netdb.h>


#include "RobovizDraw.h"
#include "Singular.h"


/*//////////////////////////////////////////////////////////////////////////////

As definições a seguir são bem restritas aos técnicos, mas farei o possível.

*/////////////////////////////////////////////////////////////////////////////

#define ROBOVIZ_HOST "localhost"
#define ROBOVIZ_PORT "32769"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class RobovizLogger {
	
private:
	// Desejamos impedir o uso indiscriminado.
	
	// Construtores
	RobovizLogger() {}
	RobovizLogger( const RobovizLogger& ) {}
	RobovizLogger& operator=( const RobovizLogger& ) {}
	
	// Destrutores
	/*
	Qual a diferença entre usarmos:
	
	-> ~ RobovizLogger();
	
	-> virtual ~ RobovizLogger();
	
	Está no comportamento da destruição de objetos em hierarquias de herança.
	
	~classe(); é um destrutor comum, chamado automaticamente quando
	um objeto da classe é destruído, mas não garante a destruição correta
	de objetos derivados se for chamado através de um ponteiro para a
	classe base.
	
	Exemplo:
	
		delete ponteiroBase;
		
		+------------------+      
		|   ClasseBase     |       < só essa parte é deletada
		+------------------+
		
		+------------------+
		| ClasseDerivada   |     < essa parte continua na memória!
		|  (camada extra)  |
		+------------------+
	
	virtual ~classe(); é um destrutor virtual, que garante que,	ao deletar
	um objeto derivado por meio de um ponteiro para a 	classe base, o 
	estrutor do objeto derivado seja chamado corretamente, 	evitando 
	azamento de memória ou destruição incompleta.
	
	Exemplo:
	
		delete ponteiroBase;

		+------------------+
		| ClasseDerivada   |     ? primeiro essa parte é destruída
		+------------------+
		
		+------------------+
		|   ClasseBase     |     ? depois essa aqui
		+------------------+
	*/
	virtual ~RobovizLogger() {}
	
	// Atributos
	bool se_ja_esta_inicializada = false;
	
	/*
	Depois que entendermos melhor o código, devemos alterar oq são esses nomes.
	*/
	int socket_number;
	struct addrinfo *p;
	struct addrinfo *servinfo;
	
	
	// Amizades
	friend class Singular<RobovizLogger>;
	
public:	
	
	static RobovizLogger* 
	obter_instancia(){
		/*
		Criar uma instância única da classe.
		*/	
		return Singular<RobovizLogger>::obter_instancia();
	}
	
	int init(){
		
		if(
		// garantir que haja apenas uma instância.
			se_ja_esta_inicializada
		){
			return 0;
		}
		
		struct addrinfo criterios_de_busca;
		int rv;
		int numbytes;
		
		criterios_de_busca = {0};
		
		
		
		
		
	}
	
	
	
	
};












#endif // ROBOVIZLOGGER_H
