#include <cmath>
#include "preditor_de_curva_da_bola.h"

/*
Cada vetor deste, terá 600 elementos disponíveis pois valor colocar:

v[ i     ] = componente_x
v[ i + 1 ] = componente_y
*/
float predicao_da_posicao   [ 600 ]; // Como Posição 
float predicao_da_velocidade[ 600 ]; // Como Vetor
float predicao_da_speed     [ 300 ]; // Denominamos Speed = Módulo
int   quantidade_de_pontos_de_posicao;


void obter_previsao_de_intersecao_com_bola(
	/*
	Componentes da Posição.
	*/
	float x, 
	float y,
	/*
	Máximo módulo de velocidade por passo.
	*/
	float max_speed_do_robo_por_passo,
	/*
	Vetor de posições da bola e seu respectiva quantidade
	de pontos.
	*/
	float posicao_da_bola[],
	float quantidade_de_pontos_de_posicao,
	/*
	Valores que serão retornados!
	*/
	float &ret_x,
	float &ret_y,
	float &ret_d
){
	/*
	Descrição:
		Obtém a interseção com a bola em movimento.
		
	Retorno:
		ret_x -> ponteiro para posição x do ponto de intersção
		ret_y -> ponteiro para posição y do ponto de interseção
		ret_d -> ponteiro para distância entre robô e ponto de interseção
	*/
	
	float max_distancia_de_contato = 0.2;
	
	int index = 0;
	while(1){
		/*
		Vamos pegar a distância relativa percorrida entre dois slots de posição.
		*/
		float vec_x = posicao_da_bola[index++] - x;
		float vec_y = posicao_da_bola[index++] - y;
		
		// Distância percorrida pela bola nesse intervalo de tempo.
		// Vulgo intervalo de tempo para termos dois slots de posição registrados.
		float dist_sq = vec_x * vec_x + vec_y * vec_y;  
		
		if(
			(
				dist_sq < max_distancia_de_contato 
			) || (
				index > quantidade_de_pontos_de_posicao
			)
		){
			ret_d = sqrtf(dist_sq);
			
			ret_x = posicao_da_bola[index - 2];
			ret_y = posicao_da_bola[index - 1];
			
			break;
		}
		
		max_distancia_de_contato += max_speed_do_robo_por_passo;
	}
}


void obter_previsao_cinematica(
	double pos_ball_x,
	double pos_ball_y,
	double vel_ball_x, 
	double vel_ball_y
){
	/*
	Descrição:
		Tentaremos prever como a bola se comportará cinematicamente.
		
		Os números citados aqui não foram alterados do material base.
		Acredito que sejam experimentais.
	*/
	
	/*
	Por miguel abreu:
		acceleration = Rolling Drag Force * mass (constant = 0.026 kg)
    	acceleration = k1 * velocity^2 + k2 * velocity
    */
	const double coef_arrasto_quadratico = - 0.01;
	const double coef_arrasto_linear     = -    1;
	
	const double coef_arrasto_quadratico_x = (vel_ball_x < 0) ? - coef_arrasto_quadratico : coef_arrasto_quadratico;
	const double coef_arrasto_quadratico_y = (vel_ball_y < 0) ? - coef_arrasto_quadratico : coef_arrasto_quadratico;
	
	// Inicializamos os vetores.
	predicao_da_posicao   [0] = pos_ball_x;
	predicao_da_posicao   [1] = pos_ball_y;
	predicao_da_velocidade[0] = vel_ball_x;
	predicao_da_velocidade[1] = vel_ball_y;
	predicao_da_speed     [0] = sqrt(vel_ball_x * vel_ball_x + vel_ball_y * vel_ball_y);
	
	int index = 2;
	
	double delta_t = 0.02;
	double delta_t_sq = delta_t * delta_t;
	
	while(
		// Enquanto não preenchermos o vetor
		index < QUANT_DE_ELEMENTOS * 2
	){
		
		double acel_x = vel_ball_x * vel_ball_x * coef_arrasto_quadratico_x + vel_ball_x * coef_arrasto_linear;
		double acel_y = vel_ball_y * vel_ball_y * coef_arrasto_quadratico_y + vel_ball_y * coef_arrasto_linear;
		
		double dx = vel_ball_x * delta_t + acel_x * delta_t_sq;
		double dx = vel_ball_y * delta_t + acel_y * delta_t_sq;
		
		pos_ball_x += dx;
		pos_ball_y += dy;
		
		if(
			(
				// Uma métrica para medirmos rapidamente distância
				(fabs(dx) + fabs(dy)) < 0.005
			) or (
				// Caso tenha passado do limite em x
				fabs(pos_ball_x) > 15
			) or (
				// Caso tenho passado do limite em y
				fabs(pos_ball_y) > 10
			)
		){
			break;
		}
		
		vel_ball_x += acel_x * delta_t;
		vel_ball_y += acel_y * delta_t;
		
		// Apesar de calcularmos em double, guardaremos em float.
		predicao_da_speed     [ index  / 2 ] = sqrt(vel_ball_x * vel_ball_x + vel_ball_y * vel_ball_y);
		predicao_da_velocidade[ index      ] = vel_ball_x;
		predicao_da_posicao   [ index++    ] = pos_ball_x;
		predicao_da_velocidade[ index      ] = vel_ball_y;
		predicao_da_velocidade[ index++    ] = pos_ball_y;
	}
	
	quantidade_de_pontos_de_posicao = index;
}


