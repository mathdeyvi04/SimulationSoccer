###  Dificuldades e Limitações

* Sobre necessidades de hardware

Perceba que é necessário rodar 22 jogadores independentes e um main simulador, o qual calculará todas as colisões e visões possíveis. Por este motivo, é realmente complicado para um computador ser capaz de rodar sozinho, logo é extremamente sugerido que seja usado pelo menos 2 computadores, um para o servidor e outro para os times, no mínimo. No caso ideal, outro computador para outro time seria excelente.

* Por que apenas não copiar o diretório que tem tudo  instalado?

Pois a instalação não é apenas os arquivos, mas variáveis de caminho por exemplo dentre outros aspectos que não seriam cobertos apenas com os arquivos.

* Tutoriais Disponíveis

Os tutoriais que existem são puramente inglês e possuem foco em instalação em Linux.
Aqui, faremos o possível para tornar o processo mais acessível possível.

* [Tutorial](https://github.com/david-simoes-93/RCSoccerSim3dSetup) de instalação pela equipe oficial da RoboCup

<br>

* [Tutorial](https://gitlab.com/robocup-sim/SimSpark/-/wikis/Installation-on-Linux) de instalação pela equipe que construiu os softwares usados pela competição.

<br>

* [Tutorial](https://www.youtube.com/watch?v=024bRKC5XnA) de instalação pela equipe BahiaRT.

* Segue a arquitetura do que iremos instalar.

<div align="center">
<img src="https://github.com/user-attachments/assets/286d84cc-d0e3-4846-a80c-17485a32eb7f" width="400"/>
</div>

> [!IMPORTANT]
> 
> O tutorial a seguir está focado no Sistema Operacional Ubuntu 24.04

# Sumário

* [Instalando SimSpark e RCSServer3D](#instalando-simspark-e-rcsserver3d)
* [Instalando RoboViz](#instalando-roboviz)
* [Conectando ServidorProxy](#conectando-servidorproxy)
  * [Certificando SyncMode](#certificando-o-syncmode)
  * [Instalando MagmaProxy](#instalando-magmaproxy)
  * [Ligando ServidorProxy](#ligando-o-servidorproxy)
* [Testando Time FC Portugal](#time-referente-ao-fc-portugal)
* [Futuros Procedimentos](#recapitulando)


# Instalando SimSpark e RCSServer3D

A partir de muitos problemas com as instalações previamente desenvolvidas pelos tutoriais, busquei outros meios.

Criei um _Issue_ dentro do código-fonte da equipe oficial desenvolvedora do SimSpark e, um deles, me respondeu em algumas horas.

Dentro desse [link](https://software.opensuse.org//download.html?project=science%3ASimSpark&package=rcssserver3d), o qual foi me passado por meio do que respondeu o issue, **há a solução**.

Experimente testar usando o comando:

```
rcssserver3d
```

Caso surja diversas mensagens de erro, está funcionando, não se preocupe. (não ironicamente)

Para desligar o servidor, ou usaremos o RoboViz ou feche o terminal executando
`pkill rcssserver3d -e -9; pkill simspark -e -9` em outro terminal.

# Instalando RoboViz

#### Verifique se há Java instalado na sua máquina.

```
java -version
```

Caso não exista, deve instalá-lo.

#### Procedimentos para instalação de Roboviz

* Entre no [link](https://github.com/magmaOffenburg/RoboViz/releases) e copie o endereço de link para o arquivo `.gz` mais recente.

* Execute no terminal:

```
sudo wget https://github.com/magmaOffenburg/RoboViz/releases/download/2.0.1/RoboViz.tar.gz
```

* Descompacte o arquivo baixado da seguinte forma e renomeie a pasta gerada:

```
sudo mkir RoboViz && sudo tar -vzvf RoboViz.tar.gz -C RoboViz
```

Os arquivos referentes ao software estarão em `~./RoboViz/bin`.

* O arquivo _config.txt_ contém parâmetros essenciais para a simulação gráfica.
  * Experimente modificar termos gráficos para ampliar perfomance.
  * Em especial, as portas de conexão com o servidor. Caso esteja rodando tudo na mesma máquina, não se preocupe. Mas caso esteja em máquinas diferentes, experimente modificar os valores das portas.

* Dentro da pasta, experimente rodar:

```
./roboviz.sh
```

Para que algo seja exibido, ligue o servidor principal, vulgo rcssserver3d.

> [!TIP]
> * Como gravar partidas?
> 
> Há uma feature do roboviz capaz disso, experimente ver o vídeo da [BahiaRT](https://youtu.be/024bRKC5XnA).

# Conectando ServidorProxy

### Certificando o syncMode

Antes de poder usar o ServidorProxy, você precisa configurar o RCSServer3D para rodar em modo de sincronização. Altere o atributo agentSyncMode no arquivo de configuração do servidor ```.../.simspark/spark.rb``` para _true_.

### Instalando MagmaProxy

* Entre neste [link](https://github.com/magmaOffenburg/magmaProxy/releases) e baixe o arquivo .zip referente ao magmaproxy mais recente.
  * Este _software_ foi uma solução criada por este time para aprimorar a eficiência entre as conexões dos times e servidor.

Pelo terminal, você deve fazer:

```
cd Downloads
unzip magmaproxy-(código-da-versao)
cd magmaproxy-(codigo-da-versao)/
sudo chmod 777 start.sh
```

Nada vai acontecer, à vista nu, e estará tudo bem. (não ironicamente)

* Experimente modificar o nome das pastas, visto que a única importante é o arquivo ```./start.sh```

### Ligando o ServidorProxy

* Host IP do Servidor RCSServer3D

Aqui, iremos rodar tudo no mesmo computador, logo usaremos o mesmo IP. Entretanto, caso usasse outra máquina, deve-se usar o IP correspondente à máquina que está rodando o servidor.

Para verificar qual é o localhost, você pode fazer:

```
ifconfig
```

Provavelmente surgirá duas possibilidades, _eno1_ e _lo_.
Como faremos tudo na mesma máquina, o que devemos focar é
_lo_ e usar o parâmetro do lado de _inet_. Em meu caso é:
_127.0.0.1_.

* Server Agent

Por definição da competição, deve ser 3100.

* Inicializando

Devemos informar outro parâmetro: a porta do servidor proxy, a partir da qual os respectivos times logarão em seus respectivos servidores proxy.

Por exemplo:

```
./start.sh 127.0.0.1 3100 3500
```

O processo de login do time ao servidor será detalhado mais adiante.

Surgirá algo como:

```
Starting magmaProxy 3.0.0
Proxy server listening on port 3500
```

Em teoria, não faça nada, além disso. Este terminal servirá como acesso para controle do servidor. **Você deve abrir outro terminal agora.**

> [!WARNING]
> 
> O ServidorProxy necessita que o Servidor RCSServer3D esteja ativo.
>
> Caso não, ligue o servidor principal antes com: 
>
> ```rcssserver3d```

# Time Referente Ao FC Portugal

Usaremos para **teste** o código de uma equipe chamada FC Portugal.
Escolhi-a, pois maior parte de seu código-fonte é em Python além 
de que sua [documentação](https://docs.google.com/document/d/1aJhwK2iJtU-ri_2JOB8iYvxzbPskJ8kbk_4rb3IK3yc/edit?tab=t.0)
é extremamente boa.

Experimente:

```
git clone https://github.com/m-abr/FCPCodebase.git
```

De posse disso, devemos conseguir gerenciar algumas dependências:

```
sudo apt install libgsl-dev
pip3 install numpy pybind11 psutil
```

Seguinte: Eu tive um trabalho **ABSURDO**, pois ler a documentação
dos desenvolvedores infelizmente não foi a primeira coisa que
fiz. Na mesma pasta do repositório, execute

```
python Run_Utils.py
```

Este comando constrói algumas bibliotecas em C++ que
serão usadas no código python.

> [!IMPORTANT]
> 
> Caso surja uma mensagem de erro relacionada ao GBLICXX e
> sua versão, apenas tente fazer o que é citado neste
> [link](https://stackoverflow.com/questions/76974555/glibcxx-3-4-32-not-found-error-at-runtime-gcc-13-2-0).
> 
> Em meu caso que estou usando Anaconda, o que funcionou especificamente foi a última sugestão.


> [!TIP]
> 
> É sugerido que seja rodado, por otimização:
> 
> ```export OMP_NUM_THREADS=1```

Agora para executar o time:

```
./start.sh 127.0.0.1 3500
```

Perceba como especificamos o localhost e a porta do ServidorProxy.

# Recapitulando

### Ligando Servidor

```
rcssserver3d
```

Conforme a partida for ocorrendo, no terminal do servidor RCSSServer3D surgirão mensagens específicas de colisões que estão ocorrendo dentro da simulação.

> [!TIP]
> 
> Caso não saiba se há servidores ligados ou ocorra um [erro especifico](https://github.com/m-abr/FCPCodebase/issues/37), execute:
>
> ```pkill rcssserver3d -e -9; pkill simspark -e -9```


### Ligando ServidorProxy

* Para cada time, um servidor proxy com porta diferente, não esqueça. 

* Deve estar dentro da pasta que contém os arquivos do magmaproxy.

```
./start.sh 127.0.0.1 3100 3500
```

### Ligando RoboViz

* Dentro da pasta que contém os arquivos do RoboViz.

```
./roboviz.sh
```

### Conectando Time

* Dentro da pasta que contém os arquivos de código.

```
./start.sh 127.0.0.1 3500
```

Com isso, tudo deve funcionar. 

* Experimente orar por mim como agradecimento, pois tudo isso foi realmente complicado de fazer.
