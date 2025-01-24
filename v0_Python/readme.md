# v0_Python
> Construção da Base da Interface e das Funcionalidades.
>
> Há apenas um jogador e uma bola, sendo aquele controlado pelo usuário.
> De tal forma que:
>
>* W, A, S, D
>   * movimentam-no;
>* Botão Esquerdo do Mouse Segurado
>   * Segura a bola carregando com ele;
>* Botão Direito do Mouse Segurado
>   * Começa a carregar força para chutar;
>* Botão Direito do Mouse Soltado
>   * Chuta a bola para a direção do mouse.

# Features

* Básicas
  * Cada objeto possui um arrasto linear que depende da velocidade;
  * A força do chute é calculada pelo quanto o usuário segura o botão direito do mouse;
  * Ao ser segurada, a bola possuirá o ponteiro de sua velocidade igual do jogador, na soltura terá sua velocidade igual à última velocidade do jogador, enquanto estava sendo segurada.
  * Ao ter um gol, a bola saí com velocidade específica no eixo x e uma velocidade aleatória no eixo y.