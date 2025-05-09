# Documentação do Algoritmo LocalizerV2

Créditos absolutos ao:
- Miguel Abreu
- m.abre@fe.up.pt

### Sumário

- [Passo 0: Análise de Requisitos Mínimos](#passo-0-análise-de-requisitos-mínimos)
- [Passo 1: Obter orientação do vetor de eixo Z](#passo-1-obter-orientação-do-vetor-de-eixo-z-e-adicioná-lo-à-matriz-de-transformação)
- [Passo 2: Calcular módulo do vetor Z](#passo-2-calcular-o-valor-de-translação-em-coordenada-z)
- [Passo 3: Estimar Vetores de Orientação do Eixo X e Y](#passo-3-estimar-as-duas-primeiras-linhas-da-matriz-de-transformação)
- [Passo 4: Refinamentos](#passo-4-refinamentos)
- [Passo 5: Finalizar Processos e Estimativas](#passo-5-finalizar-o-processo-de-localização-e-de-estimativa-da-pose)
- [Localização Baseada Em Densidade de Probabilidade](#localização-baseada-em-densidade-de-probabilidades)

### Descrição Rápida

O texto a seguir descreve com bastante profundidade um sistema de
localização tridimensional usado na lógica de visão dos agentes.

Trata-se sobre como estimar a posição e orientação da cabeça
de um agente no espaço com base em dados visuais e referências
geométricas, como segmentos de linha específicos, pontos de contato
no chão e marcadores de chão, como os cantos do campo.

> [!WARNING]
> 
> Mesmo quando não temos uma cena completa, por exemplo há somente uma trave vísivel ou linha 
> detectada, o sistema ainda poderá atualizar a posição da cabeça a partir de dados não visuais.
> 
> Entretanto, neste caso a matriz de transformação não será atualizada com base nos dados parciais,
> pois ela serve como referência absoluta para todos os objetos já vistos, logo alterá-la incorretamente
> afetaria a consistência histórica.
> 
> Portanto, infere-se que o sistema de mundo deve usar apenas a matriz de transformação para converter
> em coordenadas absolutas., já que a visão da cabeça pode ser parcial e imprecisa. Ademais, as
> informações da cabeça ainda podem úteis para outros propósitos, como machine learning.

### Passo 0: Análise de Requisitos Mínimos

- Caso não haja no mínimo qualquer extremidade de campo e 2 linhas;

- Caso haja 0 linhas vísiveis, será somente permitido se houver 3 marcadores
no chão:
    Mesmo sem linhas, ainda seria possível localizar-se com 3 marcadores,
    entretanto, há uma exceção marginal: 2 pontos de pé e uma extremidade,
    o que teoricamente é possível, mas raro e instável.

### Passo 1: Obter orientação do vetor de eixo Z e adicioná-lo à matriz de transformação

- Caso 1: Há 3 marcadores de chão não-colineares.

Usamos Singular Value Decomposition para encontrar o vetor normal ao plano.

- Caso 2: Há menos de 3 marcadores de chão não-colineares.

O grau de dificuldade cresce consideravelmente, pois não é possível definir um plano
com dados parciais. Sendo assim, nos restam outros métodos menos precisos.

Observe que se houver uma extremidade de campo, ou há mais de 3 marcadores de chão - entrando em contradição com a condição inicial - ou é impossível de resolver. Sendo
assim, descartamos a possibilidade de existência de extremidades de campo.

Vamos construir uma tabela de possibilidades para facilitar demonstração:

| X           | 0 lines + 0/1/2 feet | 1 line + 0 feet |
|-------------|----------------------|-----------------|
| 0 goalposts | -----                | -----           |
| 1 goalpost  | -----                | A, C            |
| 2 goalposts | *                    | B, C            |

- SubConclusão: 0 goalposts
    - É expressamente impossível solucionar o caso.

- SubConclusão: 1 goalpost

    - Há infinitas soluções possíveis caso haja os 2 pés.
    - Mas caso haja 1 linha, há duas possíveis soluções, A e C.
    
- SubConclusão: 2 goalposts

    - Caso haja 1 linha, haverá soluções B e C.

#### Soluções para Caso 2 do Passo 1

- Solução A:

Calcular o ponto p da linha mais próximo à trave g.

$Z_{vec} = \frac{g - p}{|g - p|}$

- Solução B:

Calular o ponto p da linha mais próximo ao ponto médio m entre as 
traves g1 e g2:

$Z_{vec} = \frac{g - m}{|g - m|}$

Esta solução é mais precisa que a solução A.

- Solução C:
    
Caso a linha não seja a linha do gol, há 3 possibilidades:

> Possbilidade I: 2 traves e uma linha ortogonal
>
> $Z_{vec} = \vec{LinhaDasTraves}\times \vec{Linha}$


> Possibilidade II: O valor de z foi inferido externamente por aprendizado de máquina.
>
> - Procurar qualquer linha horizontal.
> 
> - Denotar M como qualquer marcador de valor z conhecido.
> 
> - Calcular $Z_{vec}$ tal que (HorLine.Zvec=0) and (Zvec.Mrel=Mabsz-Z).

> Possibilidade III: Caso não haja z 
> 
> Literalmente pulamos este caso devido à insegurança dos dados.


- Solução \*:

Cenário foi testado, mas é inválido, logo descartado.

Em certas posições corporais, existem duas soluções e, embora uma esteja correta
e geralmente produza um erro menor, a outra é um ótimo local fora do campo. Pode-se
excluir a solução fora do campo com algumas modificações um pouco caras na função
de erro da otimização, mas o cenário fora do campo não é fora da realidade, então este
não é o caminho.

Adicionar uma fonte z externa pode ajudar a aumentar o erro do ótimo local errado,
mas pode não ser suficiente. Outra deficiência desse cenário é quando vemos as
traves do gol oposto, criando um grande erro visual.

### Passo 2: Calcular o valor de translação em coordenada Z


O que temos até o momento da matriz de transformação:


| -  | -  | -  | -     |
|----|----|----|-------|
| -  | -  | -  | -     |
| zx | zy | zz | z_abs |
| 0  | 0  | 0  | 1     |

Do passo anterior, sabemos o valor da orientação de z, vulgo (zx, zy, zz).

A partir de pontos p com coordenadas relativas conhecidas e coordenada z absoluta 
conhecida, é possível obtermos o valor de z com a expressão:

- p.relx * zx + p.rely * zy + p.relz * zz + mod_ = p.absz

Com isso, teremos múltiplos z_abs e estimamos um z_abs médio.

### Passo 3: Estimar as duas primeiras linhas da matriz de transformação

Suponha um elemento visual tal que a linha que o ligue ao agente seja de um comprimento
específico. Teremos duas categorias de linha:

- short line

    - comprimento < 0.5m
    - Díficil de calcular orientação e é insuficiente para gerar otimização

- long line

    - comprimento >= 0.5m

Sendo assim, há uma tabela de possibilidades que deverão ser consideradas:

|                 | 0 landmarks | 1 goalpost | 1 corner | ≥ 2 landmarks |
|-----------------|-------------|------------|----------|----------------|
| 0 long lines    | ---         | ---        | ---      | A              |
| 1 long line     | ---         | B+         | B        | A              |
| 2 long lines    | B           | B+         | B++      | A              |

- Solução A: Há solução única a partir 2 marcadores de chão
    
    - Calcular orientação do eixo X e eixo Y a partir dos marcadores
    - Obter a média a partir de todos
    - Refinar o resultado

- Solução B: Há mais de uma solução possível

    - Requisitos mínimos para podermos ser solucionável.

        - Linha mais comprida com comprimento > 1.6m de tal forma que a extração de orientação
        seja possível enquanto não estamos errando com a linha de anel.

        - Há apenas uma solução válida.

    - Notas de Esclarecimento:

        - B+

            - A solução válida é raramente única.

        - B++

            A solução é virtualmente única, mas impossível de ser calculada em cenários de A*.

    - Passos de Cálculo

        - Identificar 4 possibilidades de orientação baseadas na linha mais longa.
        - Determinar uma possibilidade razoável:

            - Caso o agente veja 1 marcador, calcular translação de cada uma das
            4 orientações baseado no marcador.

            - Caso o agente veja 0 marcadores, usará última posição.

        - Otimizar a translação em X e Y para cada possibilidade de orientação.
      - Realizar verificação de qualidade:

          Solução válida se:

          - Otimização convergir para um local mínimo
          - Distância para última posição conhecida for < 50cm
          - Erro médio de mapeamento < 0.12m / ponto

          Caso não haja solução válida, a solução mais provável será:

          - Distância para última posição conhecida for < 30cm
          - Erro médio de mapeamento < 0.06m / ponto
        
      - Refiniar a solução da submatriz responsável pela translação e rotação de XY.
    

### Passo 4: Refinamentos
	
- Identificar elementos vísiveis

    Verificando se a pose e orientação obtidos faz sentido.

- Otimizar e refinar estimativa da posição e orientação

    Com foca nas probabilidades de distância.


### Passo 5: Finalizar o processo de localização e de estimativa da pose

- Caso a matriz não esteja completamente setada, o sistema não atualizará a matriz global

- Mesmo que não haja alteração da matriz, a posição da cabeça sempre será atualizada

## Localização Baseada em Densidade de Probabilidades

Tendo em vista o caráter matemático, não farei alterações. Manterei a mesma linguagem do author.

#### Sobre Função de Densidade PDF

```
For 1 distance measurement from RCSSSERVER3D:

Error E = d/100 * A~N(0,0.0965^2) + B~U(-0.005,0.005)
PDF[d/100 * A](w) = PDF[N(0,(d/100 * 0.0965)^2)](w)
PDF[E](w) = PDF[N(0,(d/100 * 0.0965)^2)](w) convoluted with PDF[U(-0.005,0.005)](w)

where d is the distance from a given [p]oint (px,py,pz) to the [o]bject (ox,oy,oz)
and w is the [m]easurement error: w = d - measurement = sqrt((px-ox)^2+(py-oy)^2+(pz-oz)^2) - measurement

PDF[E](w) -> PDF[E](p,o,m)

For n independent measurements:

PDF[En](p,on,mn) = PDF[E1](p,o1,m1) * PDF[E2](p,o2,m2) * PDF[E3](p,o3,m3) * ...

Adding z estimation:

PDF[zE](wz) =  PDF[N(mean,std^2)](wz) 
where wz is the zError = estz - pz

PDF[zE](wz) -> PDF[zE](pz,estz)
PDF[En](p,on,mn,estz) =  PDF[En](p,on,mn) * PDF[zE](pz,estz)
```

#### Sobre Gradiente

```
Grad(PDF[En](p,on,mn,estz)) wrt p = Grad( PDF[E1](p,o1,m1) * ... * PDF[E2](p,on,mn) * PDF[zE](pz,estz)) wrt {px,py,pz}

Generalizing the product rule for n factors, we have:

Grad(PDF[En](p,on,mn)) wrt p = sum(gradPDF[Ei] / PDF[Ei]) * prod(PDF[Ei])
Grad(PDF[En](p,on,mn)) wrt p = sum(gradPDF[Ei] / PDF[Ei]) * PDF[En](p,on,mn)

note that: gradPDF[zE](pz,estz) wrt {px,py,pz} = {0,0,d/d_pz}
```


















