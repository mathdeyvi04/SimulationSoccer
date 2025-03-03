### Sobre Limitações

* Não há nenhuma documentação oficial disponível. 

O site que a hospedava foi desativado e agora só há resquícios. Por isso, fez-se necessário um estudo apurado sobre o software.

# Sumário

* [Sobre Connection](#sobre-connection)
* [Sobre Server](#sobre-server)
* [Sobre View](#sobre-view)
* [Sobre Camera](#sobre-camera)
* [Sobre Help](#sobre-help)


# Sobre Connection

Apenas configurações de conexão, nada tão importante dado que o RoboViz e o RCSSSimSpark.

# Sobre Server

### Connect

### Kill Server

Uma maneira mais direta e vísivel de extinguir o servidor.

### Kick Off

Time do lado definido inicia a partida.

### Free Kick 

Time do lado definido tem disponibilidade de total da bola. Acredito que apenas um jogador, o mais perto, tem o direito de ir chutá-la, enquanto os demais apenas observam.

Acredito que seja ideal para cobrança de faltas ou algo semelhante em que apenas um time deve ter disponibilidade da bola.

Assim como há uma circunferência com a cor do time que tem a posse, há um **temporizador** para que este time não tenha a posse infinita e possa permitir que o time adversário faça algo.

### Direct Free Kick

Acredito que tenha as mesmas propriedades do _Free Kick_, mas não há circunfrência e todos os jogadores podem alcançar a bola para chutá-la.

Não há circunferência.

### Reset Time

Reinicia apenas o tempo, não reinicia a partida ou os pontos de cada time.

### Request Full Stage Update

Acredito que seja um comando para o sistema se atualize imediatamente, sendo último para um bug talvez.

### Drop Ball

Cancela qualquer efeito que estiver havendo sobre a bola, como se voltasse um tempo atrás.

# Sobre View

### Screenshot

O próprio RoboViz tem uma ferramenta de captura de tela, pois a padrão faria confusão dentro da apresentação.

### Drawings

Este ponto é chave para **depuração** de como o time está pensando. A partir desta ferramente há inúmeras possibilidades.

Neste [artigo](https://github.com/user-attachments/files/19061754/um_pouco_sobre_desenhos_no_roboviz.pdf), há uma explicação completa e detalhada sobre como fazer esses desenhos.

### Toggle Full Screen

Entra ou sai do Modo Tela Cheia.

### Toggle Agent Overhead Type

Possibilita a visualização do número do jogador em cima da sua cabeça.

### Toggle Player Numbers

Possilibilita saber a quantidade de jogadores de cada time sem precisar contá-los. Os números surgem no canto inferior esquerdo e direito.

### Toggle Field Overlay

Possibilita a visualização 2D da partida.


### Toggle Drawings 

Possibilita a visualização dos desenhos colocados.

### Toggle Fouls

Possibilidade que faltas técnicas sejam habilitadas ou não.

### Toggle Penalty View

Em teoria, exibe um tempo de penalidade.

### Toggle Server Speed

Informa a velocidade do servidor.

### Log/Live Mode

* Log

Possibilita que o usuário insira arquivos específicos que guardam toda uma partida anteriormente realizada. Com isso, o usuário pode reassistir partidas.

* Live

Possibilita visualização da partida que está ocorrendo no servidor instantaneamente.

# Sobre Camera

### Track Ball

Foca na bola, óbvio.

### Track Player

Com jogador selecionado, foca nele.

### First Person

Com jogador selecionado, literalmente entramos visão de primeira pessoa do robô.

### Third Person

Com jogador selecionado, literalmente entramos na visão de terceira pessoa do robô.

### Select Ball

Deseja uma circunferência ao redor da bola.

### Remove Selection

Remove a circunferência ao redor da bola.

### Previous Player 

Seleciona o jogador de uma unidade menor.

### Next Player 

Seleciona o jogador de uma unidade maior.

# Sobre Help

### Help

Apenas relembrar atalhos de teclado.

### Configuration

Algumas configurações que também podiam ser alteráveis pelo config.txt
