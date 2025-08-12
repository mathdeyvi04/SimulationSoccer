# Sumário 

* [Anatomia](#anatomia)

* [Arquitetura dos Agentes](#arquitetura-dos-agentes)

* [Iniciando Agente](#iniciando-agente)

* [Comandos de Mensagem](#comandos)
    * [Comandos Effector](#alguns-_effectors_)
    * [Comandos Perceptors](#alguns-_perceptors_)

# Anatomia

Os robôs apresentados são baseados no NAO

<div align="center">
<img src="https://github.com/user-attachments/assets/b3943863-4b2c-4942-bfc3-fcb2d27ae0a9" width="400"/>
</div>

Na prática, não existe comando como _correr_ ou _pular_, os comandos referenciam
todos esses eixos e juntas e, com isso, controlamos o robô na simulação. Cada robô é
representado por um agente e é neste que os usuários trabalharão.

Informações Importantes:

* Agentes **NÃO** podem se comunicar entre si de forma direta usando TCP Sockets (será explicado).
	* A ideia é simular um jogo real e literalmente consta no regulamento.
* Agentes são representados pelo simulador/servidor e se comunicam com ele. 
	* A partir de mensagens trocadas com servidor, os agentes realizam suas ações. 

## Tipos de NAO

* Tipo 0
  * NAO Padrão, 22 juntas.

* Tipo 1
  * Pernas e braços mais longos, 22 juntas.

* Tipo 2
  * Velocidade linear mais rápida e velocidade 
  de rotação mais lenta no tornozelo.

* Tipo 3
  * Pernas e braços mais longos possíveis e quadril mais largo.

* Tipo 4
  * NAO Padrão com pés, de 24 juntas.

# Arquitetura dos Agentes

<div align="center">
<img src="https://github.com/user-attachments/assets/90cf9ded-873d-4f18-8bc8-f2028d53a8fc" width="400"/>
</div>

* Message Parser
	* Decodificar a mensagem do servidor.
* World State Update
	* Como o agente interpreta o mundo, no caso, as informações que chegaram.
* Think
	* Algoritmo de Decisão.
* Control
	* Como o agente irá tomar aquelas ações pensadas.
* Message Composer
	* Codificar a mensagem para o servidor.

# Iniciando Agente

* Entrada no Servidor
```sheel
scene rsg/caminho_dentro_do_servidor/nao_hetero.rsg 0
```
*
  * Tal mensagem representa o pedido de construção do robô na simulação.
  * O arquivo .rsg fornece todas os detalhes 3D do robô.
  * O segundo parâmetro define o tipo do NAO que deve ser renderizado.

> [!IMPORTANT]
> As equipes são obrigadas a usar diferentes tipos de NAO, no mínimo 3 tipos diferentes e 7 agentes no máximo  do mesmo tipo. 

* Inicialização dentro da Simulação

```sheel
init (unum 1)(teamname RoboIME)
```
*
  * Informará ao servidor o número do agente e de qual time pertence.
  * Dessa forma, o agente está disponível para iniciar a partida.
  * Dessa forma, o agente está disponível para iniciar a partida.

# Comandos

### Alguns _Effector's_

Os agentes controlam esses eixos e juntas usando:	
* _HingeJoint Effector_ 
  * Exemplo: ```lae3 5.3```
  * O primeiro parâmtro significa em inglês, Left Arm Effector 3, que indica qual
  junta se deseja focar.
  * O segundo parâmetro indica a velocidade da junta.

<br>

* _Beam Effector_
  * Exemplo: ```beam 10.0 -10.0 0.0```
  * Teletransporta o agente para uma posição (x, y), os dois primeiros parâmetros, e
  com uma determinada orientação, o terceiro parâmetro.
  * Aparentemente, o tamanho do campo é 13x20, não me pergunte a unidade de medida.
  * o canto onde a bola 'nasce' é o (0, 0).
  
<br>

* _Say Effector_
  * Exemplo: ```say helloworld```
  * Apesar dos agentes não poderem se comunicar usando TCP Sockets, eles literalmente
  poderem 'gritar' algo, literalmente mesmo. Como se uma mensagem fosse enviada para
  o servidor e este enviasse a todos os agentes do time.
  * **Há uma limitação de 20 bytes para essa mensagem.**
  * Esse comando é essencial para coordenação do time.

<br>

* _Synchronize Effector_
  * Exemplo: ```syn```
  * Responsável por sincronizar todas as mensagens enviadas e recebidas dos agentes.
  * Tudo deve rodar no sync mode para simular a competição real.
  * Permite partidas justas, já que força a espera de todos os comandos dos agentes.

<br>

* Exemplo de Mensagem Enviada Pelo Agente:
  * ```(beam -14.433 -0 0)(he1 3.20802)(he2 -12)(lle 9)..........(syn)```
  * O "syn" indica o fim da mensagem.

### Alguns _Perceptors_

Esse tipo de comando refere-se aos sensores dos agentes. A partir deles 
que o servidor informa aos agentes a situação deles:

* _Gyrorate Perceptor_
  * Exemplo: ```(GYR (n torso) (rt 0.01 0.07 0.46))```
  * Informa a aceleração vetorial do corpo.
  * Usado para compensar quedas ou viradas bruscas.
  
<br>

* _HingeJoint Perceptor_
  * Exemplo: ```HJ (n laj3) (ax -1.102))```
  * Note que agora não usamos mais o _e_ de effector, mas o _j_.
  * Informa o ângulo atual de uma determina junta.
  
<br>

* _ForceResistance Perceptor_
  * Exemplo: ```(FRP (n lf) (c -0.14 0.04 -0.2) (f 1.12 -0.2 13))```
  * O parâmetro _n_ indica o nome da parte.
  * O parâmetro _c_ indica o ponto em que uma força está sendo aplicada.
  * O parâmetro _f_ indica a força que está sendo aplicada, vetorialmente.
  * Útil para andar e correr.

<br>

* _Accelerometer Perceptor_
  * Exemplo: ```(ACC (n torso) (a 0.00 0.00 9.81))```
  * Como se fosse um medidor da aceleração do agente. 
  * A partir deste, podemos controlar a estabilidade do agente.

<br>

* _Vision Perceptor_
  * Extremamente importante e complicado, representa a câmera que fica na cabeça do agente.
  * Exemplo: ```(See (B (pol  8.51 -0.21 -0.17)) ...```
    * Parâmetro _B_ indica bola. Os números seguintes representam, respectivamente, distância da câmera à bola, ângulo horizontal em relação ao eixo x, ângulo vertical em relação ao plano xy. Ambos ângulos são obtidos da linha entre a câmera e a bola.	
  * Exemplo: ```... (P (team RoboIme) (id 1) (head (pol 12 -0.21 3.1)) (rlowerarm (pol ...)) ....```
    * O parâmetro _P_ indica os jogadores. Recebe-se uma espécie de dicionário com dados das partes do corpo e sua localização nas mesmas coordenadas polares.
    * Informa mais precisamente também "llowerarm", "rfoot", "lfoot".
  * Exemplo: ```... (L (pol n11 n12 n13) (pol n21 n22 n23))```
    * O parâmetro _L_ indica que se está vendo uma linha. O primeiro ponto indica o começo da linha e o segundo, o final.

<br>

* _GameState Perceptor_
  * Exemplo: ```(GS (t 0.00) (pm BeforeKickOff))```
  * Informa várias informações como tempo de partida ou pontuações.
 
<br>

* _HearPerceptor_
  * Exemplo: ```(hear 12.3 -11 helloyourself)```
  * Como os agentes recebem mensagens dos demais.
  * O primeiro número indica o momento que se escutou a mensagem.
  * O segundo número indica o ângulo em que o ruído foi ouvido. Note que não é informado qual jogador gritou exatamente.	
      * Quando o jogador envia uma mensagem e o simulador 'envia' ela depois um de um tempo, esse agente recebe "self" neste parâmetro.

> [!IMPORTANT]
> Todos os anos novos comandos podem ser introduzidos e outros podem ser removidos. Atente-se ao pdf de regras.