# Necessidades para Desenvolvimento
* OpenGL e SDL para graficos e renderizacao;
* Conhecimento em Multiprocessamento e Threads;


# Como fazer tudo funcionar

* Baixar a versao para desenvolvimento do SDL:
	* De posse dela, descompacta-la.
	
* Configurar Compilador:
	* Em opcoes de configurador, adicionar a partir da pasta x86 na aba de Diretórios:
		* Bibliotecas: pasta 'lib';
		* C includes: pasta 'include'.
			
* Em opcoes do Projeto, coloque dentro de Linker da Aba Parametros: '-lmingw32 -lSDL2main -lSDL2'
		
* Copie o arquivo SDL(algumacoisa).dll para a mesma pasta de saida do projeto.
		
		
		
		
		
		
		
