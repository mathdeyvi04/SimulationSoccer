/*
 * RobovizLogger.h
 *
 * Created on: 2013/06/24
 * Author: Rui Ferreira

 Adapted: Deyvisson.
 */
#ifndef ROBOVIZLOGGER_H
#define ROBOVIZLOGGER_H

#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "Singular.h"
#include "RobovizDraw.h"

/*
Descrição:
Esta biblioteca oferece comandos que permitem ao programa interagir diretamente com o sistema operacional. 
Com ela, podemos realizar tarefas como ler e escrever arquivos, fazer o programa "dormir" por alguns segundos, 
criar novos processos, ou até mesmo encerrar o programa.

Importância:
É uma das bibliotecas mais fundamentais no desenvolvimento de programas em sistemas baseados em Unix (como Linux). 
Ela fornece acesso a funções essenciais do sistema operacional.
*/
#include <unistd.h>

/*
Descrição:
Essa biblioteca define tipos de dados especiais usados em várias outras bibliotecas do sistema, 
como números para identificar processos, tamanhos de arquivos, etc.

Importância:
Ela ajuda a garantir que os dados usados nas funções do sistema tenham o formato correto, 
independente do tipo de computador ou sistema operacional.
*/
#include <sys/types.h>

/*
Descrição:
Esta biblioteca é usada quando queremos fazer um programa "conversar" com outros programas, 
seja no mesmo computador ou em outro pela rede. Ela fornece as ferramentas para criar conexões 
de rede usando "sockets", que são como canais de comunicação.

Importância:
É essencial para programar qualquer tipo de comunicação de rede, como servidores de jogos, 
chat online ou servidores web.
*/
#include <sys/socket.h>

/*
Descrição:
Ela contém definições específicas para trabalhar com a internet, como endereços IP e portas. 
Com essa biblioteca, conseguimos dizer ao programa com qual computador ele deve se conectar, 
ou em qual porta ele deve ficar "ouvindo".

Importância:
Sem ela, não seria possível configurar os dados básicos para comunicação pela internet.
*/
#include <netinet/in.h>

/*
Descrição:
Essa biblioteca ajuda a converter endereços IP entre o formato "letra e número" 
(ex: "192.168.0.1") e o formato usado internamente pelo computador (binário).

Importância:
É muito útil para que o programa entenda e manipule endereços da internet 
que nós, humanos, escrevemos em texto.
*/
#include <arpa/inet.h>

/*
Descrição:
Essa biblioteca permite que o programa procure informações sobre nomes de domínio, 
como "google.com", e os converta para endereços IP que podem ser usados para conexões.

Importância:
Ela é essencial para permitir que o programa se conecte a servidores usando nomes, 
sem depender diretamente de IPs numéricos.
*/
#include <netdb.h>

#define ROBOVIZ_HOST "localhost"
#define ROBOVIZ_PORT "32769"

class RobovizLogger {
	/*
	Descrição:
		Classe responsável por enviar comandos de visualização ao simulador RoboViz,
		utilizado no contexto do RoboCup 3D Soccer Simulation. Esta classe segue o
		padrão Singleton, garantindo que apenas uma instância do logger exista em
		tempo de execução.

	Responsabilidades:
		- Inicializar e encerrar conexões com o visualizador RoboViz.
		- Enviar primitivas gráficas (linhas, esferas, pontos, etc.) para exibição.
		- Gerenciar buffers de desenho nomeados para controle de visualizações.

	Principais Métodos:

	- static RobovizLogger* Instance():
	    Acessa a instância única do logger.

	- int init():
	    Inicializa a conexão com o RoboViz.

	- void destroy():
	    Encerra e libera recursos da conexão.

	- void swapBuffers(const std::string* setName):
	    Troca o buffer de desenho atual, útil para animações.

	- void drawLine(...):
	    Desenha uma linha 3D entre dois pontos com cor e espessura definidas.

	- void drawCircle(...):
	    Desenha um círculo no plano XY com cor e espessura especificadas.

	- void drawSphere(...):
	    Desenha uma esfera em 3D com cor e raio definidos.

	- void drawPoint(...):
	    Desenha um ponto no espaço 3D.

	- void drawPolygon(...):
	    Desenha um polígono preenchido com transparência.

	- void drawAnnotation(...):
	    Adiciona uma anotação textual em uma coordenada 3D.

	Observações:
		- A comunicação é feita por socket UDP com o RoboViz:
			- UDP:
				User Datagram Protocol, comunicação ideal para aplicações
				que exigem velocidade acima de confiabilidade e integridade de dados.
	*/

private:
	RobovizLogger() {};
	RobovizLogger( const RobovizLogger& );
	RobovizLogger& operator=( const RobovizLogger& );

	friend class Singular<RobovizLogger>;

	//////////////////////////////////////////////////////////////////////////////

	bool conexao_inicializada = false;
	// Indica se a conexão com o RoboViz foi estabelecida corretamente.

	int descritor_socket;
	// Identificador do socket usado para comunicação com o RoboViz.

	struct addrinfo* endereco_ativo;
	// Ponteiro para a estrutura de endereço de destino (RoboViz) atualmente utilizada.

	struct addrinfo* lista_de_enderecos;
	// Lista de endereços retornada por getaddrinfo() que contém possíveis destinos válidos.

public:

	static RobovizLogger*
	obter_instancia(){
		/*
		Mesma ideia do Singular<...>, entretanto, aqui desejamos
		retornar um ponteiro para a instância, enquanto lá retornamos 
		uma referência.
		*/
		static RobovizLogger instancia;

		return &instancia;
	}

	int 
	init(){
		/*
	    Descrição:
	    	Inicializa a conexão com o visualizador RoboViz via socket UDP.

		    Esta função configura os parâmetros de conexão de rede utilizando
		    a função `getaddrinfo`, cria um socket UDP e define o endereço
		    de destino para envio de dados de visualização.

	    Parâmetros:
	    	None

	    Retorno:
	        - 0: sucesso (conexão inicializada)
	        - 1: erro ao resolver endereço com getaddrinfo
	        - 2: erro ao criar socket
		*/

		if ( conexao_inicializada ) { return 0; }  // Garantia de que executará apenas uma vez.

	    struct addrinfo dicas_de_endereco = {0};
	    int    resultado_resolucao = 0;

	    /*
		Permitirá que o Sistema Operacional escolha automaticamente entre
		IPv4 e/ou IPv6, isto é, o domínio.
		*/
	    dicas_de_endereco.ai_family = AF_UNSPEC;

	    /*
		Indica que o socket estará no padrão UDP, no qual 
		haverá prioridade na velocidade e sem garantia de
		integridade.
		*/  
	    dicas_de_endereco.ai_socktype = SOCK_DGRAM;

	    if (
	    	/*
			Buscamos por resultados a partir desses critérios e 
			colocamos eles dentro do ponteiro de informações de servidor.
			
			Para mais informações desta princesa complexa:

				https://pubs.opengroup.org/onlinepubs/009619199/getad.htm
				(não tem o L no final mesmo.)
			*/
	    	(resultado_resolucao = getaddrinfo(ROBOVIZ_HOST, ROBOVIZ_PORT, &dicas_de_endereco, &lista_de_enderecos)) != 0
	   	) {

	   		fprintf(
				/*
				Apenas providenciamos melhor depuração.
				*/
				stderr,
				"Erro na inicialização do logger->getaddrinfo: %s",
				gai_strerror(resultado_resolucao)
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
	    	endereco_ativo = lista_de_enderecos; 
	    	endereco_ativo != NULL; 
	    	endereco_ativo = endereco_ativo->ai_next
	    ){

	        if(
	        	/*
				Para mais informações:
					https://pubs.opengroup.org/onlinepubs/009695099/functions/socket.html
				*/
	        	(descritor_socket = socket(endereco_ativo->ai_family, endereco_ativo->ai_socktype,
	                endereco_ativo->ai_protocol)) == -1
	        ){

	            perror("Erro na Construção do Socket");
	            continue;
	        }

	        break;
	    }

	    // Não há socket disponível.
	    if (endereco_ativo == NULL) {
	        return 2;
	    }

	    conexao_inicializada = true;
	    return 0;
	}

	void 
	destroy(){

		freeaddrinfo(lista_de_enderecos);
		lista_de_enderecos = NULL;
		close(descritor_socket);
	}

	void 
	enviar_e_deletar( 
		unsigned char* buffer,
		int tamanho_buffer
	){
		/*
		Para mais informações sobre a função sendto:
			https://pubs.opengroup.org/onlinepubs/009695099/functions/sendto.html
		*/

	    sendto(
	        descritor_socket,
	        buffer,
	        tamanho_buffer,
	        0,
	        endereco_ativo->ai_addr,
	        endereco_ativo->ai_addrlen
	    );
	    delete[] buffer;
	}

	void 
	criar_buffer_limpo(
		const string* nome_conjunto
	) {
    
	    int tamanho_buffer = 0;
	    unsigned char* buffer = novo_buffer_swap(nome_conjunto, &tamanho_buffer);

	    enviar_e_deletar(
	    	buffer,
	    	tamanho_buffer
	    );
	}

	void 
	desenhar_ponto(
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
		unsigned char* buffer = novo_ponto(
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

	void 
	desenhar_linha(
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
		unsigned char* buffer = nova_linha(
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

	void 
	desenhar_circulo(
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
		unsigned char* buffer = novo_circulo(
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
		unsigned char* buffer = nova_esfera(
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

	void 
	desenhar_poligono(
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
		unsigned char* buffer = novo_poligono(
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

	void 
	desenhar_anotacao(
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
		unsigned char *buffer = nova_anotacao(
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

	void 
	desenhar_anotacao_de_agente(
		const string *texto,
		bool left_team,
		int numero_de_agente,
		float cor_r,
		float cor_g,
		float cor_b
	){

		float cor[3] = {cor_r, cor_g, cor_b};

		int tamanho_do_buffer = 0;
		unsigned char *buffer = nova_anotacao_agente    (
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
	virtual ~RobovizLogger() { destroy(); } // Pesquise oq significa isso, é genial demais.
};

#endif // ROBOVIZLOGGER_H
