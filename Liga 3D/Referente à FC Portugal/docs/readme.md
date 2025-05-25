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

* debug.py
  
Arquivo específico para realização de testes das funções implementadas no módulo.
Para informações mais completas, pode-se utilizar o help(nome_do_modulo)

* module_main.cpp

Arquivo específico **responsável pela portabilidade das aplicações para o Python**.

Caso deseje alterar algo, saiba que o _pybind somente aceitará caracteres ASCII_, 
não tente colocar UTF-8 nas documentações ou nomes.

* Makefile 

Arquivo de build do módulo cpp, o qual gerará um arquivo _.so_.

Caso deseje compilar o módulo manualmente para realizar testes e comparações,
faça o que é descrito dentro do arquivo:

```
# Para executar a compilação manual, descomente a seguinte linha: 
# FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)
```

Atente-se ao que está sendo feito, colocamos o diretório da versão de python e 
da pasta pybind11.

* Como é feito a automação?

Estes módulos são compilados e construídos a partir de um método específico da classe 
[Script](../src/sobre_scripts/comuns/Script.py).

... Descrição sucinta do que acontece.











