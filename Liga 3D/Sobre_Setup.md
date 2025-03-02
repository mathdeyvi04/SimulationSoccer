###  Dificuldades e Limitações

* Sobre necessidades de hardware

Perceba que é necessário rodar 22 jogadores independentes e um main simulador, o qual calculará todas as colisões e visões possíveis. Por este motivo, é realmente complicado para um computador ser capaz de rodar sozinho, logo é extremamente sugerido que seja usado pelo menos 2 computadores, um para o servidor e outro para os times, no mínimo. No caso ideal, outro computador para outro time seria excelente.

* Tutoriais Disponíveis

Os tutoriais que existem são puramente inglês e possuem foco em instalação em Linux.
Aqui, faremos o possível para tornar o processo mais acessível possível.

* [Tutorial](https://github.com/david-simoes-93/RCSoccerSim3dSetup) de instalação pela equipe oficial da RoboCup

<br>

* [Tutorial](https://gitlab.com/robocup-sim/SimSpark/-/wikis/Installation-on-Linux) de instalação pela equipe que construiu os softwares usados pela competição.


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
  * [Para Futuros Procedimentos](#para-futuros-procedimentos)
* [Testando Time FC Portugal](#time-referente-ao-fc-portugal)

# Instalando SimSpark e RCSServer3D

A partir de muitos problemas com as instalações previamente desenvolvidas pelos tutoriais, busquei outros meios.

Criei um _Issue_ dentro do código-fonte da equipe oficial desenvolvedora do SimSpark e, um deles, me respondeu em algumas horas.

Dentro desse [link](https://software.opensuse.org//download.html?project=science%3ASimSpark&package=rcssserver3d), o qual foi me passado por meio do que respondeu o issue, **há a solução**.

Experimente testar usando o comando:

```
rcssserver3d
```

Caso surja diversas mensagens de erro, está funcionando, não se preocupe. (não ironicamente)

Para desligar o servidor, ou usaremos o RoboViz ou feche o terminal. 

# Instalando RoboViz

#### Verifique se há Java instalado na sua máquina.

```
java -version
```

Caso não exista, deve instalá-lo.

#### Procedimentos

* Entre no [link](https://github.com/magmaOffenburg/RoboViz/releases) e baixe o arquivo _.gz_.

* Descompacte-o, levando a pasta _bin_ para um local mais adequado e desejado.
  * Experimente renomeá-la para algo mais adequado.
* O arquivo _config.txt_ contém parâmetros essenciais para a simulação gráfica.
  * Experimente modificar termos gráficos para ampliar perfomance.
  * Em especial, as portas de conexão com o servidor. Caso esteja rodando tudo na mesma máquina, não se preocupe. Mas caso esteja em máquinas diferentes, experimente modificar os valores das portas.

* Para executar o RoboVIz:

```
./roboviz.sh
```

Para que algo seja exibido, ele pedirá por isso, ligue o servidor principal, vulgo rcssserver3d.

> [!TIP]
> * Como gravar partidas?
> 
> Há uma feature do roboviz capaz disso, experimente ver o vídeo da [BahiaRT](https://youtu.be/024bRKC5XnA).

# Conectando ServidorProxy

### Certificando o syncMode

Antes de poder usar o ServidorProxy, você precisa configurar
o RCSServer3D para rodar em modo de sincronização.
Altere o atributo agentSyncMode no arquivo de configuração
do servidor ```.../.simspark/spark.rb``` para _true_.

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

Tenha certeza que o syncMode está como _true_ no servidor.

* Host IP do Servidor RCSServer3D

Aqui, iremos rodar tudo no mesmo computador, logo usaremos o mesmo IP. Entretanto, caso usasse outra máquina, deve-se usar o IP correspondente à máquina que está rodando o servidor.

Para verificar qual é o localhost, você pode fazer:

```
ifconfig
```

Provavelmente surgirá duas possibilidades, _eno1_ e _lo_.
Como faremos tudo na mesma máquina, o que devemos focar é
_lo_ e usar o parâmetro do lado de _inet_. Em meu caso é:
_localhost_.

* Server Agent

Por definição da competição, deve ser 3100.

* Inicializando

Devemos informar outro parâmetro: a porta do servidor proxy, a partir da qual os respectivos times logarão.

Por exemplo:

```
./start.sh 127.0.0.1 3100 3500
```

Este último parâmetro é importante para um dos times que deverá entrar por este ServidorProxy. Mais à frente será mais detalhado o que deve ser feito.

Surgirá algo como:

```
Starting magmaProxy 3.0.0
Proxy server listening on port 3500
```

Em teoria, não faça nada, além disso. Este terminal servirá como acesso para controle do servidor. **Você deve abrir outro terminal agora.**

### Para futuros procedimentos.

Refaça os processos no terminal a partir da pasta magmaproxy,
a qual contém o start.sh referente ao ServidorProxy.

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
é extremente boa.

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
fiz:

```
python Run_Utils.py
```

Este comando deve construir algumas bibliotecas em C++ que
serão usadas no código python.

> [!IMPORTANT]
> 
> Caso surja uma mensagem de erro relacionada ao GBLICXX e
> sua versão, apenas tente fazer o que é citado neste
> [link](https://stackoverflow.com/questions/76974555/glibcxx-3-4-32-not-found-error-at-runtime-gcc-13-2-0).

> [!TIP]
> 
> É sugerido que seja rodado, por otimização:
> 
> ```export OMP_NUM_THREADS=1```

//// Alguns avisos a mais sobre como rodar o time

# Recapitulando

* Sequência de Comando Para fazer iniciar tudo 
