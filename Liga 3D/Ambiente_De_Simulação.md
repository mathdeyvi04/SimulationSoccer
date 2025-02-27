# Sobre a Simulação

* Note que há 22 agentes sendo renderizados, além do simulador, algo bem complicado para computadores normais.

* É a junção de 2 softwares:
	* RCSSERVER3D
		* Agentes se conectam à simulação por meio de TCP Sockets e deste servidor.
	* SimSpark
		* Sistema de simulação genérico para várias simulações multiagentes. Ele suporta o desenvolvimento de simulações físicas para pesquisa em IA e robótica.
		* Usa Open Dynamics Engine (ODE) para detectar colisões e simular a dinâmica do corpo rígido.

## Comunicação
* Conexão usando TCP Socket para porta 3100.

### Client Agent Proxy
* Em cada time, há um SAProxy como se fosse localhost para os agentes do time enviarem suas mensagens e ele enviá-las para o Servidor Principal.
	* Há toda uma esquematização e arquitetura criada para essas comunicações e para o multiparalelismo.



























n 