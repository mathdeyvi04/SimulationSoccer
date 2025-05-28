# Dissertação Sobre Funcionalidades Individuais e Hierarquia

A seguir, uma breve explicação organizada dos arquivos e pastas do código.

Se você deseja reconstruir ou aprender do zero a aplicação, sugiro seguir a ordem
de apresentação e explicação.

## [sobre_cpp](sobre_cpp)

Pasta responsável por gerenciar as funcionalidades mais pesadas e/ou mais frequentemente
usadas, todas construídas em C++ com portabilidade para serem utilizadas no Python.

Os módulos são independentes entre si e cada um é fundamental para o funcionamento da aplicação geral:

* [preditor_de_curva_da_bola](sobre_cpp/preditor_de_curva_da_bola.md)
* [a_estrela](sobre_cpp/a_estrela.md)
* [ambientacao](sobre_cpp/ambientacao.md)

Dentro de cada, há 3 arquivos essenciais para a portabilidade.

* debug.py ou debug.cc
  
Arquivo específico para realização de testes das funções implementadas no módulo.

Usando `make teste` é possível realizar as verificações. Lembre-se que há a necessidade
de realizar o `make` anteriormente.

* module_main.cpp

Arquivo específico **responsável pela portabilidade das aplicações para o Python**.

Para compreender como o pybind funciona de verdade, sugiro verificar o arquivo 
[module_main referente à estrela.cpp](../src/sobre_cpp/a_estrela/module_main.cpp).

Caso deseje alterar algo, saiba que o _pybind somente aceitará caracteres ASCII_, 
não tente colocar UTF-8 nas documentações ou nomes. Quando descobri isso, preferi reescrever inglês mesmo.

Utilizando `help(nome_do_modulo)` pode-se obter mais informações sobre o módulo e 
suas funcionalidades.

* Makefile 

Arquivo de build do módulo cpp, o qual gerará um arquivo _.so_.

Caso deseje compilar o módulo manualmente para realizar testes e comparações,
faça o que é descrito dentro do arquivo:

```
# Para executar a compilação manual, descomente a seguinte linha: 
# FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)
```

Em minha máquina, instalei o python e pybind com `sudo apt install ...`. 
Logo, meus caminhos são esses e essas são as respectivas versões. Caso você tente
fazer o make manualmente para testes e verificações, você deve colocar o caminho 
do interpretador e o caminho da biblioteca pybind11.


* Como é feito a automação?

Estes módulos são compilados e construídos a partir de um método específico da classe 
[Script](../src/sobre_scripts/comuns/Script.py).

... Descrição sucinta do que acontece.











