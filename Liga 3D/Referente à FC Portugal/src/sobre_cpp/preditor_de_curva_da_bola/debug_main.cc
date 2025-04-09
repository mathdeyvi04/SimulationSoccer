/*
Para melhores explicações, sugiro verificar o debug_main.cc que há dentro
da pasta A*.
*/

#include "preditor_de_curva_da_bola.h"
#include <chrono>
#include <iostream>
#include <iomanip>

using std::cout;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

std::chrono::_V2::system_clock::time_point inicio, fim;

int main(){
	
	float pos_ball_x =  3;
	float pos_ball_y =  4;
	
	float vel_ball_x = -5;
	float vel_ball_y = -1;
	
	inicio = high_resolution_clock::now();
	
    obter_previsao_cinematica(
		pos_ball_x,
		pos_ball_y,
		vel_ball_x,
		vel_ball_y
	);
	
    fim = high_resolution_clock::now();
	
	cout << std::fixed << std::setprecision(8);
	
	for(
		int index = 0;
		index < quantidade_de_pontos_de_posicao;
		index += 2
	){
		cout << index / 2 << " pos: " << predicao_da_posicao[index] << "," << predicao_da_posicao[index + 1] << "\n";
		cout << index / 2 << " vel: " << predicao_da_velocidade[index] << "," << predicao_da_velocidade[index + 1] << "\n";
		cout << index / 2 << " spd: " << predicao_da_speed[index / 2] << "\n";
	}
	
	cout << "\n\n" << duration_cast<microseconds>(fim - inicio).count() << "us para predição de atributos cinemáticos.\n";
	
	float robot_x = -1;
	float robot_y = 1;
	float maxima_speed_por_passo = 0.7 * 0.02;
	float ret_x, ret_y, ret_d;
	
	inicio = high_resolution_clock::now();
    
	obter_previsao_de_intersecao_com_bola(
		robot_x,
		robot_y,
		maxima_speed_por_passo,
		predicao_da_posicao, 
		quantidade_de_pontos_de_posicao,
		
		ret_x, ret_y, ret_d
	);
    
	fim = high_resolution_clock::now();
	
	cout << "Interseção: " << ret_x << "," << ret_y << " \ndist: " << ret_d << "\n\n";
	
	cout << duration_cast<microseconds>(fim - inicio).count() << "us para interseção.\n\n";
}

