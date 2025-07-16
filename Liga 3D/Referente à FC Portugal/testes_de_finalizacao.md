# Testes de Finalização

Já é fato que os módulos a_estrela e preditor_de_curva_da_bola estão satisfatórios, entretanto...

Os esforços para o conserto do módulo ambientacao se mostraram inúteis.

O que será feito:

- Faremos dois tipos de verificações:
	- Usando debug do makefile, salvaremos o gabarito original na pasta correspondente. Denotaremos como teste primário.
	- Usando o servidor e o RoboViz, com o Utils, poderemos apenas fazer uma verificação visual, a não ser que
	deseje inúmeros prints (ironia). Denotaremos como teste secundário.

- Pouco a pouco trocaremos o código Localization e substituiremos pelo de nossa 
autoria. 

# Evolução

Desconsideraremos que qualquer diferença dos resultados esteja ligada a arquivos 
de baixa responsabilidade, como `Ruido_de_Campo.h` ou `Singular.h`.

###  Relacionados à AlgLin.h:

A única parte que sofreu modificações abruptas está ligada às Linhas e às Matrizes.
Faremos a troca de código apenas nestas partes.

- Alterando as Matrizes:
	- Teste Primário:   OK
	- Teste Secundário: OK

- Alterando as Linhas:
	- Teste Primário:   OK 
	- Teste Secundário: OK

- Alterando Modelo de Ruído:
	- Teste Primário:   OK 
	- Teste Secundário: OK

### Relacionados ao Roboviz:

- Alterando **RobovizField**:
	- Teste Primário:   OK 
	- Teste Secundário: NOPE

Verificando o código relacionado à `atualizar_a_partir_de_transformacao(...)`, pôde-se obter uma diferença na atribuição
de `melhor_segm`. O código não fazia uma segunda verificação de magnitude de erro, fato que causava ambiguidade 
na hora de decisão das linhas.

### Sem Escapatória

Não foi possível obter o porquê os resultados da matriz de transformação possui diferença entre os valores originais e
atribuídos pela RoboIME, faz-se necessário, então, um estudo do impacto dessa diferença.

### Conclusão.

Assim como fizemos na definição de matrizes, consideraremos a matriz como um vetor e calcularemos o quão diferentes
são as matrizes geradas pela norma do vetor diferença.

A seguir, o resultado de `Head_to_Field` obtido pelos respectivos times e a distância entre eles.

<img src="https://github.com/user-attachments/assets/2b3f97f0-adfb-428a-b76e-9b492e5421ca" height="500"/>

Concluímos, então, que não há impacto significativo na diferença dos resultados, **restanto apenas aprimorar**.























































