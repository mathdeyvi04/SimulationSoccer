# Necessidades para Desenvolvimento

* SDL para gráficos e renderização;
* Conhecimento em Multiprocessamento e Threads em C;
* Conhecimento em Inglês;

# Como fazer tudo funcionar

* Baixar a versão para desenvolvimento do SDL:
	* Algo como "SDL2-devel-2.30.10-mingw.tar.gz"
	* De posse dela, descompactá-la.

##### Dentro do Dev C++
* Configurar Compilador:
	* Em _opções de configurador_, adicionar, a partir da pasta x86 que está dentro da pasta descompactada, na aba _Diretórios_:
		* _Bibliotecas_: pasta 'lib';
		* _C includes_: pasta 'include'.
	
* Em _Opções do Projeto_, coloque dentro de _Linker_ da Aba _Parâmetros_: "**-lmingw32 -lSDL2main -lSDL2**"
		
* Copie o arquivo _SDL2.dll_ para a mesma pasta de saída do projeto. (Importante)
		
		
		
		
		
		
		
