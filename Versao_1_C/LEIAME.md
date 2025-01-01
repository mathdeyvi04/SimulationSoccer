# Necessidades para Desenvolvimento
<<<<<<< Updated upstream
* OpenGL e SDL para graficos e renderizacao;
* Conhecimento em Multiprocessamento e Threads;
=======
	* SDL para graficos e renderizacao;
	* Multiprocessamento e Threads;
>>>>>>> Stashed changes


# Como fazer tudo funcionar

<<<<<<< Updated upstream
* Baixar a versao para desenvolvimento do SDL:
	* De posse dela, descompacta-la.
	
* Configurar Compilador:
	* Em opcoes de configurador, adicionar a partir da pasta x86 na aba de Diretórios:
		* Bibliotecas: pasta 'lib';
		* C includes: pasta 'include'.
=======
	* Baixar a versao para desenvolvimento do SDL:
		* Algo como "SDL2-devel-2.30.10-mingw.tar.gz"
		* De posse dela, descompacta-la.
	
	* Configurar Compilador:
		* Em opcoes de configurador, a partir da pasta x86:
			* Dentro da Aba Bibliotecas: adicionar a pasta 'lib';
			* Dentro da Aba C includes: adicionar a pasta 'include';
>>>>>>> Stashed changes
			
* Em opcoes do Projeto, coloque dentro de Linker da Aba Parametros: '-lmingw32 -lSDL2main -lSDL2'
		
* Copie o arquivo SDL(algumacoisa).dll para a mesma pasta de saida do projeto.
		
		
		
		
		
		
		
