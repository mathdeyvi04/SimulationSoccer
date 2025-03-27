/*
Código desenvolvido por:
    Matheus Deyvisson, 2025
*/

#ifndef A_ESTRELA_H
#define A_ESTRELA_H

struct Node{
    // Inerentes À Grafos
    // Esses não tinha como colocar em português.
    Node* left;
    Node* right;
    Node* up;

    // Inerentes ao algoritmo.
    Node* parente;
    float p_g,
    float p_f
}

extern void a_estrela(
    float parametros[],
    int quantidade_de_parametros
);

extern float caminho_final[
    // Acredito que seja o comprimento total
    2050
]

extern int tamanho_caminho_final;

#endif A_ESTRELA_H
