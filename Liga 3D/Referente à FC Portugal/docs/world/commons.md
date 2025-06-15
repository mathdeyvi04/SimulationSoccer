# [commons](../../src/world/commons)

* [BodyPart.py](../../src/world/commons/BodyPart.py)

Classe responsável por agrupar informações de partes corporais, seja suas juntas
ou sua massa.

* [JointInfo.py](../../src/world/commons/JointInfo.py)

Classe responsável por agrupar informações de juntas, recebendo como entrada
o dicionário de informações XML do respectivo tipo de robô.

* [OtherRobot.py](../../src/world/commons/OtherRobot.py)

Classe responsável por representar outros robôs e como o self.robô representará eles.
Não é trivial e possui apenas atributos condensadores de informações.

* [Draw.py](../../src/world/commons/Draw.py)

Classe responsável por agrupar ferramentas de comunicação com software Roboviz. 
Basicamente igual à como fizemos em RobovizLogger e RobovizDraw, o que me faz repensar o
porquê nos dedicamos a essas.

