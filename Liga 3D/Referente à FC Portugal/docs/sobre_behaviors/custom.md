# [custom](../../src/sobre_behaviors/custom)

### Presença de Classe `Env`

Em algumas dessas classes, como Walk e Dribble, há a definição de uma classe `Env`, a qual 
reúne e prepara os dados necessários para que as decisões de movimento sejam tomadas, sem 
que o controlador de comportamento precise lidar diretamente com detalhes de implementação 
do robô ou do ambiente.

### Presença de Arquivos `.pkl`

Alguns comportamentos são complexos e necessitam de maior eficiência, para tanto
utilizou-se aprendizado por reforço para obter-se resultados satisfatórios para cada 
comportamento. O resultado deste aprendizado foi condensado nos arquivos `.pkl`.

* [Step](../../src/sobre_behaviors/custom/Step)

Implementa um primitivo de passo simples, baseado em especificações
geométricas e cinemática inversa, sem inteligência adaptativa.

Ideal para movimentos padronizados, previsíveis ou testes de hardware.

* [Walk](../../src/sobre_behaviors/custom/Walk)

Utiliza um modelo de aprendizado por reforço (RL) para realizar caminhada omnidirecional. 
O comportamento é adaptativo, considerando o alvo, orientação e distância, e executa ações
aprendidas a partir de observações do ambiente. 

É mais sofisticada, capaz de se adaptar a alvos variados e situações dinâmicas, porém exige mais
custo computacional.

* [GetUp](../../src/sobre_behaviors/custom/GetUp)

Implementa o comportamento autônomo de levantar do chão, monitorando os sensores de orientação,
aceleração e estabilidade do robô para decidir, de forma robusta, qual é a melhor estratégia de levantar 
(de frente, de costas ou realizando um movimento de flip) a partir de qualquer posição de queda.

* [Fall](../../src/sobre_behaviors/custom/Fall)

Implementa um comportamento de queda controlada, utilizando um modelo de rede neural para 
gerar comandos de ação que levam o robô de forma segura até o solo. Considera a queda 
concluída quando a cabeça atinge uma altura próxima ao chão.

* [Dribble](../../src/sobre_behaviors/custom/Dribble)

Implementa o comportamento autônomo de drible para robôs humanoides utilizando técnicas de
aprendizado por reforço. Gerencia as fases de aproximação, controle da bola e finalização
do drible, ajustando dinamicamente orientação, velocidade e estratégia de movimentação conforme
a posição da bola e a situação no campo. 

Seu funcionamento integra módulos de percepção, caminhada e tomada de decisão,
garantindo que o robô execute dribles eficientes, adaptando-se tanto à presença 
da bola quanto à proximidade das linhas de campo.

* [BasicKick](../../src/sobre_behaviors/custom/BasicKick)

Implementa o comportamento de caminhar até a bola e realizar um chute simples para
robôs humanoides. Ela gerencia as etapas de aproximação, alinhamento e execução do 
chute, ajustando automaticamente parâmetros conforme o tipo do robô e as condições
do ambiente.











