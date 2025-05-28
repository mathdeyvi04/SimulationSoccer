# preditor_de_curva_da_bola

Módulo C++ focado em previsibilidade de oportunidades com a bola.
  
* [preditor_da_curva_da_bola.h](../../src/sobre_cpp/preditor_de_curva_da_bola/preditor_de_curva_da_bola.h)
  * Declaração base de ponteiros e vetores que serão amplamente utilizados.
  
* [preditor_da_curva_da_bola.cpp](../../src/sobre_cpp/preditor_de_curva_da_bola/preditor_de_curva_da_bola.cpp)
  * Implementação de funções de previsão de interseção e de atributos cinemáticos.
  
## Highlights

Dentro da função `obter_previsao_cinematica`, a condição de parada do loop
principal possui uma componente significativa: o **diferencial de posição**.

Sabendo que o retorno da função é um vetor referente as posições previstas calculadas
pelo algoritmo, segue uma tabela do quanto cada algoritmo de parada influencia
o tamanho do vetor de previsão.

| Formato de Condição                       | Quantidade de slots  |
|-------------------------------------------|----------------------|
| `fabs(dx) < 0.0005 && fabs(dy) < 0.0005`  | 520  (Original)      |
| `fabs(dx) < 0.005  && fabs(dy) < 0.005`   | 292                  |
| `fabs(dx) < 0.005  && fabs(dy) < 0.05`    | 66                   |
| `(fabs(dx) + fabs(dy)) < 0.0005`          | 538                  |
| `(fabs(dx) + fabs(dy)) < 0.005`           | 310  (Aqui    )      |
| `(fabs(dx) + fabs(dy)) < 0.05`            | 84                   |

Lembre-se que quanto mais slots no vetor final, mais tempo a função demora para terminar.
