#include "LocalizerV2.h"
#include "World.h"  // Incluirá Singular.h e AlgLin.h, este último possuindo math.h e cmath

/*
Precisamos te um arquivo .cpp para que ele seja executado em conjunto.
*/
using namespace std;

static World& mundo_existente = Singular<World>::obter_instancia();

int LocalizerV2::estimar_error_posicional(
	const Vetor3D sample, 
	const Vetor3D& cheat,
	double error_placeholder[]
){
	/*
	Descrição:
		Toma uma amostra do erro de posição atual.

	Parâmetros:
		-> sample: posição estimada do agente
		-> cheat: posição real proibida e providenciada pelo servidor.
	*/
	if(
		mundo_existente.pos_abs_do_robo_cart_cheat == Vetor3D(0, 0, 0)
	){

		return 0;
	}

	double x_err = sample.x - cheat.x;
	double y_err = sample.y - cheat.y;
	double z_err = sample.z - cheat.z;
	double xx_err = x_err * x_err; 
	double yy_err = y_err * y_err;
	double zz_err = z_err * z_err;

	error_placeholder[0] += x_err;
	error_placeholder[1] += y_err;
	error_placeholder[2] += z_err;

	double sq_err_2d = xx_err + yy_err;
	error_placeholder[3] += sqrt(sq_err_2d);
	error_placeholder[4] += sq_err_2d;

	double sq_err_3d = xx_err + yy_err + zz_err;
	error_placeholder[5] += sqrt(sq_err_3d);
	error_placeholder[6] += sq_err_3d;

	return 1;
}

bool LocalizerV2::refinamento(
	float initial_angle, 
	float initial_x, 
	float initial_y
){
	/*
	Aplicamos o refinamento direto na matriz de transformação preliminar.
	* 1st - improve map fitting
		* 2nd - identify line segments and their endpoints
		* 3rd - fine tune again using known markers
	*/

	RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

	// Contamos as estatísticas antes do refinamento
	counter_tuneo_de_refinamento += estimar_error_posicional(
															  Vetor3D(initial_x, initial_y, _Head_to_Field_Prelim.obter(11)),
															  mundo_existente.pos_abs_do_robo_cart_cheat,
															  errorSum_fineTune_before
															);

	if(
		!refinamento_aux(
						 initial_angle,
						 initial_x,
						 initial_y,
						 false
						)
	){

		return false;
	}

	// Estatísticas para o primeiro refinamento.
	estimar_error_posicional(
							  Vetor3D(initial_x, initial_y, _Head_to_Field_Prelim.obter(11)),
							  mundo_existente.pos_abs_do_robo_cart_cheat,
							  errorSum_fineTune_euclidianDist
							);

	campo_existente.atualizar_marcadores_a_partir_da_transformacao(_Head_to_Field_Prelim);

	// Dessa vez com probabilidades
	refinamento_aux(initial_angle, initial_x, initial_y, true);

	estimar_error_posicional(
						      _Head_to_Field_Prelim.obter_vetor_de_translacao(),
						      mundo_existente.pos_abs_do_robo_cart_cheat,
							  errorSum_fineTune_probabilistic
							);

	campo_existente.atualizar_marcadores_desconhecidos_a_partir_da_transformacao(_Head_to_Field_Prelim);

	return true;
}

void LocalizerV2::run(){
	/*
	- Calcular posição e orientação 3D.
	
	- Caso haja nova informação disponível:

		- Setará "is_updodate" como True.

	- Caso não:

		- Fornecerá a última posição conhecida.
	*/

	RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

	atualizar_estado_do_sistema(RUNNING);

	// ----- Fluxo de Trabalho: 0

	_se_esta_pronta_para_atualizacao = false;
	_se_head_z_esta_pronta_para_atualizacao = false;
	_passos_desde_ultima_atualizacao++;

	resetar();

	campo_existente.atualizar_marcadores();

	int numero_de_segm       = campo_existente.lista_de_todos_os_segmentos .size();
	int numero_de_marcadores = campo_existente.lista_de_corners_e_goalposts.size();

	if(
		(numero_de_marcadores == 0 && numero_de_segm < 2) || (numero_de_segm == 0)
	){

		if(
			numero_de_segm == 0 && numero_de_marcadores == 0
		){

			atualizar_estado_do_sistema(BLIND);
		}
		else{

			atualizar_estado_do_sistema(MINFAIL);
		}

		return;
	}

	// ------ Fluxo de Trabalho 1 - 2

	if(
		!obter_orientacao_do_eixo_z()
	){

		return;
	}

	// ----- Fluxo de Trabalho 3 - 4

	if(
		!( (numero_de_marcadores > 1 ) ? obter_translacao_rotacao_xy() : guess_xy() )
	){

		return;
	}

	// ----- Atualizamos variáveis públicas

	commit_system();

	atualizar_estado_do_sistema(DONE);

	if(
		mundo_existente.se_bola_esta_visivel
	){

		counter_ball += estimar_error_posicional(
												  _Head_to_Field_Prelim * mundo_existente.pos_rel_da_bola_cart,
												  mundo_existente.pos_abs_da_bola_cart_cheat,
												  errorSum_ball
												);
	}
	
	//reportar_situacao();
}

int main(){
	printf("\n\nExecutado com sucesso.\n\n");

	return 0;
}