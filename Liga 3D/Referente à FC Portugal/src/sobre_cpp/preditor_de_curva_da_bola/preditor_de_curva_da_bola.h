#ifndef PREDITOR_DE_CURVA_DA_BOLA_H
#define PREDITOR_DE_CURVA_DA_BOLA_H

/*
Cada vetor deste, terá 600 elementos disponíveis pois valor colocar:

v[ i     ] = componente_x
v[ i + 1 ] = componente_y
*/
#define QUANT_DE_ELEMENTOS 300
extern float predicao_da_posicao   [ QUANT_DE_ELEMENTOS * 2 ]; // Como Posição
extern float predicao_da_velocidade[ QUANT_DE_ELEMENTOS * 2 ]; // Como Vetor
extern float predicao_da_speed     [ QUANT_DE_ELEMENTOS     ]; // Denominamos Speed = Módulo
extern int   quantidade_de_pontos_de_posicao;

extern void obter_previsao_de_intersecao_com_bola(
	float x,
	float y,
	float max_speed_do_robo_por_passo,
	float posicao_da_bola[],
	float quantidade_de_pontos_de_posicao,
	/*
	Valores retornados.
	*/
	float &ret_x,
	float &ret_y,
	float &ret_d
);

extern void obter_previsao_cinematica(
	double bx,
	double by,
	double vx,
	double vy
);

#endif // PREDITOR_DE_CURVA_DA_BOLA
