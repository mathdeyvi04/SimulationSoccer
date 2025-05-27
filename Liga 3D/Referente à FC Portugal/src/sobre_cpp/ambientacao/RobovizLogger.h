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

#include "Singular.h"
#include "RobovizDraw.h"

/*//////////////////////////////////////////////////////////////////////////////

As definições a seguir são bem restritas aos técnicos, mas farei o possível.

*/////////////////////////////////////////////////////////////////////////////

#define ROBOVIZ_HOST "localhost"
#define ROBOVIZ_PORT "32769"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class RobovizLogger {
	/*
	Classe responsável por gerenciar logins de entrada no RoboViz
	e ser a interface intermediária que possibilitará o uso de desenhos
	no mesmo.
	*/
	
private:
	// Desejamos impedir o uso indiscriminado.
	
	// Construtores
	RobovizLogger() {}
	RobovizLogger( const RobovizLogger& ) {}
	RobovizLogger& operator=( const RobovizLogger& );
	
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
	// virtual ~RobovizLogger(); Vamos precisar defini-la após o a construção
	// de outra função.
	
	// Atributos
	bool se_ja_esta_inicializada = false;
	
	int socket_descriptor;   // inglês é bem mais bonito
	struct addrinfo *elemento_do_servidor;
	struct addrinfo *servidor;
	
	
	// Amizades
	friend class Singular<RobovizLogger>;
	
public:	

	static RobovizLogger* obter_instancia(){
		/*
		Mesma ideia do Singular<...>, entretanto, enquanto
		aquele retorna referência, este retorna um ponteiro.
		*/

		static RobovizLogger instancia;
		return &instancia;
	}

	int init(){
		/*
		Descrição:
			Função que inicializa o gerenciador de login.

		Retorno:

			0 -> Inicialização bem sucedida.
			1 -> Função addrinfo não foi bem sucedida. Ela indircará o próprio erro.
			2 -> Sem informações de socket disponíveis.
		*/

		if(
			/*
			Para impedir múltiplas inicializações.
			*/
			se_ja_esta_inicializada
		){
			return 0;
		}

		int retorno = 0;

		struct addrinfo criterios_de_busca = {0};  // Apenas zeramos toda a informação.

		/*
		Permitirá que o Sistema Operacional escolha automaticamente entre
		IPv4 e/ou IPv6, isto é, o domínio.
		*/
		criterios_de_busca.ai_family   =    AF_UNSPEC;

		/*
		Indica que o socket estará no padrão UDP, no qual 
		haverá prioridade na velocidade e sem garantia de
		integridade.
		*/  
		criterios_de_busca.ai_socktype = SOCK_DGRAM;  // C

		/*
		Buscamos por resultados a partir desses critérios e 
		colocamos eles dentro do ponteiro de informações de servidor.
		
		Para mais informações desta princesa complexa:

			https://pubs.opengroup.org/onlinepubs/009619199/getad.htm
			(não tem o L no final mesmo.)
		*/
		retorno = getaddrinfo(
			ROBOVIZ_HOST,
			ROBOVIZ_PORT,
			&criterios_de_busca,
			&servidor
		);

		if(
			retorno != 0
		){
			fprintf(
				/*
				Apenas providenciamos melhor depuração.
				*/
				stderr,
				"Erro na inicialização do logger, getaddrinfo: %s",
				gai_strerror(retorno)
			);

			return 1;
		}

		/*
		Caso a função anterior, getaddrinfo, tenha sido bem sucedida,
		a localização de servidor indicará uma lista linkada de addrinfo
		structs.

		Cada uma delas específica um endereço de socket e informações de como
		criar um socket neste endereço.

		O funcionamento da função garante que haverá pelo menos um addrinfo struct.
		*/
		for(

			elemento_do_servidor = servidor;

			elemento_do_servidor != NULL;

			elemento_do_servidor = (*elemento_do_servidor).ai_next

		){	

			/*
			Para mais informações:
				https://pubs.opengroup.org/onlinepubs/009695099/functions/socket.html
			*/
			socket_descriptor = socket(
				(*elemento_do_servidor).ai_family,
				(*elemento_do_servidor).ai_socktype,
				(*elemento_do_servidor).ai_protocol
			);

 			if(
 				/*
				Caso algum resulte em erro, vamos apenas pulá-lo e
				procurar outro.
 				*/
				socket_descriptor == -1
			){

				perror("socket");
				continue;
			}

			/*
			Vamos parar no primeiro que estiver em condições.
			*/

			break;
		}

		if(
			/*
			Não há socket disponível.
			*/
			elemento_do_servidor == NULL
		){

			return 2;
		}

		se_ja_esta_inicializada = true;

		return 0;
	}


	void destroy(){
		/*
		Devemos eliminar a memória da maneira correta.
		*/
		freeaddrinfo(
			servidor
		);

		servidor = NULL;

		close(socket_descriptor);
	}


	void enviar_e_deletar(
		unsigned char *buffer,
		int tamanho_do_buffer
	){
		/*	
		Descrição:
			Apenas focamos a responsabilidade de enviar e deletar para uma única função.
		*/


		/*
		Para mais informações sobre a função sendto:
			https://pubs.opengroup.org/onlinepubs/009695099/functions/sendto.html
		*/
		sendto(
			socket_descriptor,
			buffer,
			tamanho_do_buffer,
			0,
			(*elemento_do_servidor).ai_addr,
			(*elemento_do_servidor).ai_addrlen
		);

		// Depois de enviarmos, não precisamos mais nos preocupar.
		delete[] buffer;
	}


	void enviar_buffer_limpo(
		const string *nome_a_ser_setado
	){
		/*
		Descrição:
			Envia o buffer por meio do socket que é representado
			por seu socket_descriptor.
		*/

		int tamanho_do_buffer = 0;
		unsigned char *buffer = criar_buffer( 
											  nome_a_ser_setado, 
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_ponto(
		float x,
		float y,
		float z,
		float tam,
		float cor_r,
		float cor_g,
		float cor_b,
		const string *nome_a_ser_setado
	){

		float centro[3] =             {x, y, z};
		float    cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char* buffer = criar_ponto(
											  centro,
											  tam,
											  cor,
											  nome_a_ser_setado,
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_linha(
		float x1,
		float y1,
		float z1,
		float x2,
		float y2,
		float z2,
		float grossura,
		float cor_r,
		float cor_g,
		float cor_b,
		const string *nome_a_ser_setado
	){
		float inicio[3] =          {x1, y1, z1};
		float  final[3] =          {x2, y2, z2};
		float    cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char* buffer = criar_linha(
											  inicio,
											  final,
											  grossura,
											  cor,
											  nome_a_ser_setado,
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_circulo(
		float x,
		float y,
		float raio,
		float grossura,
		float cor_r,
		float cor_g,
		float cor_b,
		const string *nome_a_ser_setado
	){

		float centro[2] =                {x, y};
		float    cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char* buffer = criar_circulo(
											  centro,
											  raio,
											  grossura,
											  cor,
											  nome_a_ser_setado,
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_esfera(
		float x,
		float y,
		float z,
		float raio,
		float cor_r,
		float cor_g,
		float cor_b,
		const string *nome_a_ser_setado
	){

		float centro[3] =             {x, y, z};
		float    cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char* buffer = criar_esfera(
											  centro,
											  raio,
											  cor,
											  nome_a_ser_setado,
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_poligono(
		const float *vertices,
		int numero_de_vertices,
		float cor_r,
		float cor_g,
		float cor_b,
		float cor_a,  // transparencia
		const string *nome_a_ser_setado
	){

		float cor[4] = {cor_r, cor_g, cor_b, cor_a};

		int tamanho_do_buffer = 0;
		unsigned char* buffer = criar_poligono(
											  vertices,
											  numero_de_vertices,
											  cor,
											  nome_a_ser_setado,
											  &tamanho_do_buffer
											);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_anotacao(
		const string *texto,
		float x, 
		float y,
		float z,
		float cor_r,
		float cor_g,
		float cor_b,
		const string *nome_a_ser_setado
	){

		float cor[3] = {cor_r, cor_g, cor_b};
		float pos[3] =             {x, y, z};

		int tamanho_do_buffer = 0;
		unsigned char *buffer = criar_anotacao(
												texto,
												pos,
												cor,
												nome_a_ser_setado,
												&tamanho_do_buffer
											  );

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}


	void desenhar_anotacao_de_agente(
		const string *texto,
		bool left_team,
		int numero_de_agente,
		float cor_r,
		float cor_g,
		float cor_b
	){

		float cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char *buffer = criar_anotacao_de_agente(
														  texto,
														  left_team,
														  numero_de_agente,
														  cor,
														  &tamanho_do_buffer
														);

		enviar_e_deletar(
			buffer,
			tamanho_do_buffer
		);
	}
	
private:
	// Finalmente.
	virtual ~RobovizLogger(){ destroy(); }
	
};

#endif // ROBOVIZLOGGER_H
