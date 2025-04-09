#include "preditor_da_curva_da_bola.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace std;

py::array_t<float> ____(
	py::array_t<float> parametros
){
	/*
	Descrição:
		Faz a mesma coisa que aquela função de prever cinemática.
		Fornece 
	*/
	
	py::buffer_info buffer_de_entrada = parametros.request();
	float* parametros_ptr = (float*) buffer_de_entrada.ptr  ;
	
	float pos_ball_x = parametros_ptr[0];
	float pos_ball_y = parametros_ptr[1];
	float vel_ball_x = parametros_ptr[2];
	float vel_ball_y = parametros_ptr[3];
	
	obter_previsao_cinematica(
		pos_ball_x,
		pos_ball_y,
		vel_ball_x,
		vel_ball_y
	);
	
	
	
}











// Chame de kinematic_predict















