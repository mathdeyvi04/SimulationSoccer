# Necessidades para Desenvolvimento

* SDL para gráficos e renderização;
* Conhecimento em Multiprocessamento e Threads em C;
* Conhecimento em Inglês;

# Como fazer funcionar Aplicação SDL

* Baixar a versão para desenvolvimento do SDL:
	* Algo como "SDL2-devel-2.30.10-mingw.tar.gz"
	* Descompacte-a.

##### Dentro do Dev C++
* Configurar Compilador:
	* Em _opções de configurador_, na aba _Diretórios_, adicionar a partir da pasta x86 que está dentro da pasta descompactada:
		* Em _Bibliotecas_: pasta 'lib';
		* Em _C includes_: pasta 'include'.
	
* Em _Opções do Projeto_, coloque dentro de _Linker_ da Aba _Parâmetros_: "**-lmingw32 -lSDL2main -lSDL2**"
	
>[!IMPORTANT]
>* Copie o arquivo _SDL2.dll_ para a mesma pasta de saída do projeto.
	
# Como fazer funcionar Aplicação SDL_ttf

* Obter versão para desenvolvimento do SDL_ttf:
	* Algo como "SDL2_ttf-devel-2.24.0-mingw.zip";
	* Descompacte-a.
	
* Mover arquivo _SDL_ttf.h_ da pasta _include_ para _include_ do "SDL2-devel-2.30.10-mingw".

* Mover arquivo _SDL_tff.dll_ para _bin_ da mesma pasta anteriormente citada.
  
>[!IMPORTANT]
>* Mova-o também para o mesmo diretório de desenvolvimento.

* Mover arquivos .a e .la que estão em _lib_ da pasta referente ao ttf para pasta _lib_ da pasta de desenvolvimento do SDL.

##### Dentro DevC++

* Dentro de Linker, colocar "-lSDL2_ttf".
