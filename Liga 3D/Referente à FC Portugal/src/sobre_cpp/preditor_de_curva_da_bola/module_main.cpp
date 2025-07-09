/*
Matheus Deyvisson, 2025.

Para mais comentários e explicações acerca do que está sendo feito aqui,
sugiro que leia o arquivo de mesmo nome disponível na pasta de a_estrela.
*/
#include "preditor_de_curva_da_bola.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace std;

py::array_t<float> get_ball_kinematic_prediction(
	py::array_t<float> pos_ball,
	py::array_t<float> vel_ball
){
	/*
	Descrição:
		Função que fornecerá a portabilidade da execução da função
		C++ em Python.
	*/

	py::buffer_info buffer_de_pos = pos_ball.request();
	float* ptr_pos = (float*) buffer_de_pos.ptr;

	float pos_ball_x = ptr_pos[ 0 ];
	float pos_ball_y = ptr_pos[ 1 ];

	py::buffer_info buffer_de_vel = vel_ball.request();
	float* ptr_vel = (float*) buffer_de_vel.ptr;

	float vel_ball_x = ptr_vel[ 0 ];
	float vel_ball_y = ptr_vel[ 1 ];

	obter_previsao_cinematica(
		pos_ball_x,
		pos_ball_y,
		vel_ball_x,
		vel_ball_y
	);
	
	py::array_t<float> a_ser_retornado = py::array_t<float>(
		quantidade_de_pontos_de_posicao + quantidade_de_pontos_de_posicao + (quantidade_de_pontos_de_posicao / 2)
	);
	py::buffer_info buffer_de_saida = a_ser_retornado.request();
	float *ptr = (float*) buffer_de_saida.ptr;
	
	int index = 0;
	
	while(
		index < quantidade_de_pontos_de_posicao
	){
		ptr[index] = predicao_da_posicao[index];
		
		index++;
	}
	
	/*
	Observe que para cada loop, começamos com index = 0.
	À primeira vista, não faz sentido. Entretanto, o ponteiro 
	iniciado é shiftado de tal que forma que ele indique o primeiro
	elemento do próximo array e isso nos permite a utilização de
	indexs mais concisos.
	*/
	ptr += quantidade_de_pontos_de_posicao;
	index = 0;
	
	while(
		index < quantidade_de_pontos_de_posicao
	){
		ptr[index] = predicao_da_velocidade[index];
		
		index++;
	}
	
	ptr += quantidade_de_pontos_de_posicao;
	index = 0;
	
	while(
		index < quantidade_de_pontos_de_posicao / 2
	){
		ptr[index] = predicao_da_speed[index];
		
		index++;
	}
	
	return a_ser_retornado;
}


py::array_t<float> get_possible_intersection_with_ball(
	py::array_t<float> pos_robot,
	float max_speed_do_robo_por_passo,
	py::array_t<float> pos_ball_predict

){
	/*
	Descrição:
		Função que fornecerá a portabilidade da execução da função
		C++ em Python.
	*/

	py::buffer_info buffer_de_pos_robot = pos_robot.request();
	float* ptr_pos_robot = (float*) buffer_de_pos_robot.ptr;

	py::buffer_info buffer_de_pos_prevista = pos_ball_predict.request();
	float* pos_ball = (float*) buffer_de_pos_prevista.ptr;
	int quant_de_previcoes = buffer_de_pos_prevista.shape[0];

	float ret_x, ret_y, ret_d;  // Note que criamos os valores aqui!
	
	obter_previsao_de_intersecao_com_bola(
		ptr_pos_robot[0], // x
		ptr_pos_robot[1], // y
		max_speed_do_robo_por_passo,
		pos_ball,
		quant_de_previcoes,
		/*
		Resultado será retornado para os seguintes endereços de memória.
		*/
		ret_x,
		ret_y,
		ret_d
	);
	
	py::array_t<float> a_ser_retornado = py::array_t<float>( 3 );  // Alocamos
	py::buffer_info buffer_de_saida = a_ser_retornado.request();
	float* ptr = (float*) buffer_de_saida.ptr;
	
	ptr[ 0 ] = ret_x;
	ptr[ 1 ] = ret_y;
	ptr[ 2 ] = ret_d;
	
	return a_ser_retornado;
}

using namespace pybind11::literals;

PYBIND11_MODULE(
	preditor_de_curva_da_bola,
	m
){
	m.doc() = "Module responsible for testing predictions of cinematic attributes and the possibility of intersection with the ball.";

	m.def(
		"get_ball_kinematic_prediction",
		&get_ball_kinematic_prediction,
		R"pbdoc(
		Description:
		    We will attempt to predict how the ball will behave kinematically,
		    including taking drag into.

		    The numerical values referenced here have not been altered from the original
		    source material. I believe they are experimental.

		Parameters:
		    The initial kinematic attributes:
		    - float pos_ball[2]
		    - float vel_ball[2]

		Return:
		    A super vector containing the values in sequence:

			T = len(vector_returned)

		    - position predictions           -> (x, y)   = (i, i + 1)
		    	which goes from 0 to T - 1.
		    - velocity vector predictions    -> (vx, vy) = (i + T, i + 1 + T)
		    	which goes from T to 2T - 1.
		    - speed predictions              -> (|v|)    = (i + 1.5T)
		    	which goes from 2T to 2.5T - 1.

		    The arrows indicate the organization format. Knowing the total number of
		    elements in the vector, you can calculate:

		    number_of_position_points = total / 2.5;

		    The value obtained from this calculation is exactly the index at which
		    the position values end and the velocity vector values begin.
		)pbdoc"
		"pos_ball"_a,
		"vel_ball"_a
	);
	
	m.def(
		"get_possible_intersection_with_ball",
		&get_possible_intersection_with_ball,
		R"pbdoc(
		Description:
			Checks the possible intersection of the robot with the ball. 

			Note that the variable max_speed... is used to assume that the 
			robot is moving at maximum speed toward the ball, in order to 
			provide an estimate.

			- If an intersection occurs, it returns the relative position (x, y) 
			and the corresponding distance related to the intersection.

			- If not, it will return the last calculated relative position (x, y) 
			and corresponding distance.

			A way to determine whether contact is possible is to observe the given 
			relative distance.

		Parametros: 
			- float pos_robot[2]: initial position's robot
			- float max_speed_do_robo_por_passo: Maximum robot displacement per time step
			- float posicoes_da_bola[]:  Vector of future positions of the ball and the respective number of points.
			
		Retorno:
			An array thats:
			- [0] -> x position of the intersection point  
			- [1] -> y position of the intersection point  
			- [2] -> distance between the robot and the intersection point  
		)pbdoc"
		"pos_robot"_a,
		"max_speed_do_robo_por_passo"_a,
		"posicoes_da_bola"_a
	);
}
