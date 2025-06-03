# Workflow de LocalizerV2

## Sumário

- [Visão Geral](#visão-geral)
- [Tipos de Referência](#tipos-de-referência)
- [Aviso Importante](#aviso-importante)
- [0. Tarefas e Checagens Básicas](#0-tarefas-e-checagens-básicas)
- [1. Encontrar o Vetor de Orientação do Eixo Z](#1-encontrar-o-vetor-de-orientação-do-eixo-z)
- [2. Calcular a Translação em z](#2-calcular-a-translação-em-z)
- [3. Estimar a Transformação Completa (Linhas 1 e 2 da matriz)](#3-estimar-a-transformação-completa-linhas-1-e-2-da-matriz)
- [4. Identificação de Elementos Visíveis e Ajuste Fino com Probabilidades de Distância](#4-identificação-de-elementos-visíveis-e-ajuste-fino-com-probabilidades-de-distância)
- [Último Passo: Atualização Final das Matrizes](#último-passo-atualização-final-das-matrizes)
- [Localização Baseada em Densidades de Probabilidade](#localização-baseada-em-densidades-de-probabilidade)
- [Gradiente](#gradiente)

---

## Visão Geral

Este documento descreve o fluxo de trabalho para estimar a pose (posição e orientação) do agente utilizando referências visuais no campo, como marcos (landmarks), segmentos de linha e pontos de contato dos pés.

---

## Tipos de Referência

As referências para localização podem ser obtidas de:
- **Landmarks**: identificados pelo servidor, podendo ser bandeiras de canto ou traves do gol.
- **Linhas**: segmentos sempre localizados no chão.
- **Pontos de contato dos pés**: assume-se que o ponto de contato está no plano do solo.

---

## Aviso Importante

Quando há apenas informações parciais, elas podem ser usadas para atualizar a posição da cabeça (por exemplo, nas componentes x/y/z), mesmo sem informação visual. Porém, a matriz de transformação (incluindo a translação) **não é alterada** para evitar que a posição percebida de objetos já vistos seja afetada. Por isso, a estimativa de mundo (worldstate) **não deve depender da posição da cabeça para converter coordenadas relativas em absolutas**; deve-se usar a matriz de transformação ou métodos internos. A posição da cabeça ainda pode ser útil para outros fins, como aprendizado de máquina.

---

## 0. Tarefas e Checagens Básicas

### Cenários Excluídos:
- **0 landmarks e menos de 2 linhas:** requisito do passo 1 não atendido.
- **0 linhas:** permitido no passo 1 apenas se houver 3 referências de solo (exemplo raro: 2 pés e um canto, situação indesejada).

---

## 1. Encontrar o Vetor de Orientação do Eixo Z

### 1.1. Caso com 3 ou mais referências de solo não colineares:
- **Suposição:** referências não são colineares (ex.: 2 linhas, seus pontos nunca são colineares; 1 linha + 2 pés colineares é raro).
- **Solução:** usar Decomposição de Valores Singulares (SVD) para encontrar o vetor normal ao plano do solo.

### 1.2. Caso com menos de 3 referências de solo não colineares:
- Se houver 1 bandeira de canto, então ou temos 3 referências, ou o caso é impossível.
- Assumindo 0 bandeiras de canto, temos as seguintes combinações possíveis:

|                | 0 linhas + 0/1/2 pés | 1 linha + 0 pés |
|----------------|----------------------|-----------------|
| 0 traves       | ------               | ------          |
| 1 trave        | ------               | A, C            |
| 2 traves       | *                    | B, C            |

- Se houver 1 ou 2 traves e apenas 1 linha, assume-se que a linha é a linha do gol.

#### Soluções:
- **1.2.A:** Se for a linha do gol: encontre o ponto mais próximo (p) da linha à trave (g), $Z_{vec} = \frac{g - p}{|g - p|}$
- **1.2.B:** Se for a linha do gol: use o ponto médio das traves (m), $Z_{vec} = \frac{g - p}{|g - p|}$
- **1.2.C:** Se não for a linha do gol:  
  - (I) Se houver duas traves (linha do travessão) e uma linha ortogonal: Z = travessão × linha (ou vice-versa)
  - (II) Se a coordenada z foi fornecida externamente (ex.: aprendizado de máquina): use qualquer linha horizontal e iguale o z percebido com o absoluto
  - (III) Se o z não foi fornecido: pule para o último passo
- **1.2.\***: Este cenário é inválido/testado, pois pode haver soluções ambíguas, sendo necessário fonte externa de z para desambiguar.

### 1.3. Impossível ou não implementado
- Pule para o último passo.

---

## 2. Calcular a Translação em z

Sabendo a matriz de transformação preliminar, pode-se calcular a translação em z usando um ponto com coordenada absoluta conhecida:

- Para um ponto `p`:
  ```
  p.relx * zx + p.rely * zy + p.relz * zz + ? = p.absz
  ```
- Fazendo isso para todos os pontos e tirando a média, obtém-se z.

---

## 3. Estimar a Transformação Completa (Linhas 1 e 2 da matriz)

### Qualidade da Solução para Diferentes Combinações:

|                 | 0 landmarks | 1 trave | 1 canto | >=2 landmarks |
|-----------------|------------|---------|---------|---------------|
| 0 linhas longas | ---        | ---     | ---     | A             |
| 1 linha longa   | ---        | B+      | B       | A             |
| 2 linhas longas | B          | B+      | B++     | A             |

- **A:** Solução única  
  - Calcule orientação dos eixos X e Y a partir de 2 landmarks.
  - Média da translação para cada landmark visível.
  - Ajuste fino da translação/rotação XY.

- **B:** Mais de uma solução, usa-se a última posição conhecida como referência  
  - Requisitos mínimos:
    - Linha mais longa >1.6m para garantir orientação.
    - Exatamente uma solução plausível.
  - Notas:
    - B+ (solução raramente única)
    - B++ (solução praticamente única, mas não pode ser computada pelo algoritmo do caso A)
  - Passos:
    - Encontre 4 possíveis orientações baseando-se na linha mais longa (alinhada a X ou Y).
    - Calcule possíveis translações XY para cada orientação (usando landmarks ou última posição).
    - Otimize a translação XY para cada orientação.
    - Avalie a qualidade das soluções ("plausível", "provável", etc).
    - Escolha a melhor solução e faça ajuste fino.

---

## 4. Identificação de Elementos Visíveis e Ajuste Fino com Probabilidades de Distância

-- 

## Último Passo: Atualização Final das Matrizes

- Se a matriz preliminar não foi completamente definida, a matriz de transformação real não é alterada — mas a posição da cabeça reflete as últimas mudanças.

---

# Localização Baseada em Densidades de Probabilidade

## Estimando o Erro de Medidas de Distância

Para uma medição de distância do RCSSSERVER3D:

- **Erro:**  
  ```
  E = d/100 * A~N(0,0.0965^2) + B~U(-0.005,0.005)
  ```
  - d: distância do ponto ao objeto
  - A: ruído gaussiano
  - B: ruído uniforme

- **PDF do erro:**  
  - PDF[E](w) é a convolução das PDFs do ruído gaussiano e uniforme.

Para n medições independentes:
```
PDF[En](p,on,mn) = PDF[E1](p,o1,m1) * PDF[E2](p,o2,m2) * ... * PDF[En](p,on,mn)
```

Adicionando estimativa de z:
```
PDF[zE](wz) = PDF[N(média, desvio^2)](wz)
```
onde `wz = estz - pz`

---

## Gradiente

O gradiente da probabilidade conjunta em relação à posição é dado pela soma dos gradientes individuais ponderados pela própria probabilidade, considerando também a componente z, se aplicável.

---

**Resumo:**  
O workflow detalha como usar diferentes referências do campo para estimar a pose do agente, os critérios para aceitar/rejeitar soluções e a modelagem estatística do erro das medições utilizadas no processo de localização.