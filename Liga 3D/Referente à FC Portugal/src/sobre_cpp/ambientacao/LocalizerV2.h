#ifndef LOCALIZERV2_H
#define LOCALIZERV2_H

#include "Singular.h"
#include "RobovizField.h"
#include "Matriz.h"
#include "Ruido_de_Campo.h"
#include <iostream>
#include <cstdio>

/*
#include <gsl/gsl_multifit.h> //Linear least-squares fitting
#include <gsl/gsl_linalg.h>   //Singular value decomposition
#include <gsl/gsl_multimin.h> //Multidimensional minimization
*/

/* Funções Matemáticas Que Serão Úteis */

Acredito que seja desnecessário a tradução dado o objetivo matemático das mesmas.

void add_gsl_regression_sample(
	gsl_matrix* matriz, 
	gsl_vector* vetor, 
	int sample_no, 
	const Vetor3D& relativeCoord, 
	double absoluteCoord, 
	double translCoeffMult=1
){

	gsl_matrix_set(matriz, sample_no, 0, relativeCoord.x);
	gsl_matrix_set(matriz, sample_no, 1, relativeCoord.y);
	gsl_matrix_set(matriz, sample_no, 2, relativeCoord.z); 
	gsl_matrix_set(matriz, sample_no, 3, translCoeffMult); 
	gsl_vector_set(vetor,  sample_no,    absoluteCoord  );
}

template<size_t SIZE>
gsl_vector* create_gsl_vector(
	const std::array<double, SIZE> &content
){

	gsl_vector* v = gsl_vector_alloc (SIZE);
	
	for(
		int i=0; 
		    i<SIZE; 
			i++
	){

		gsl_vector_set(v, i, content[i]);
	}
	return v;
}


Vetor2D get_ground_unit_vec_perpendicular_to( const Vetor3D& vetor ){
	/*
	Em inglês é bem melhor.
	
	Descrição:
		Obtém vetor unitário no plano z = 0, perpendicular a um dado vetor.
		Matematicamente, (0,0,1)x(vetor)/|vetor|.
	*/

	// Definimos como padrão os vetores convencionais, (1, 0) e (0, 1)
	float gx = 1;
	float gy = 0;
	const float aux = sqrtf(vetor.x * vetor.x + vetor.y * vetor.y); // Vetor 

	if(
		aux != 0 // Já sabemos que será maior que 0
	){
		
		gx = - vetor.y / aux;
		gy =   vetor.x / aux;	
	}

	return Vetor2D( gx, gy );
}

Vetor3D rotacionar_em_torno_do_eixo_do_chao( Vetor3D vetor, Vetor3D Z_vec ){
	/*
	Descrição:
		Aplica uma rotação 3D em torno de um eixo no plano do chão, onde o eixo é perpendicular
		à projeção do vetor Z_vec no plano xy.
		
		Semelheante à: u=(0,0,1)x(Zvec)/|Zvec|
	*/

	Vetor2D vetor_unit_perpendicular_no_plano = get_ground_unit_vec_perpendicular_to( Z_vec );

	/* Desnecessário qualquer alteração de tradução */

	//Angle between unit normal vector of original plane and unit normal vector of rotated plane:
	//cos(a) = (ov.rv)/(|ov||rv|) = ((0,0,1).(rvx,rvy,rvz))/(1*1) = rvz
	float& cos_a = Z_vec.z; 
	//assert(cos_a <= 1);
	if(cos_a > 1) cos_a = 1; //Fix: it happens rarely, no cause was yet detected
	float sin_a = -sqrtf(1 - cos_a*cos_a); //invert sin_a to invert a (direction was defined in method description)

	const float i_cos_a = 1 - cos_a;
	const float uxuy_i  = vetor_unit_perpendicular_no_plano.x * vetor_unit_perpendicular_no_plano.y * i_cos_a;
	const float uxux_i  = vetor_unit_perpendicular_no_plano.x * vetor_unit_perpendicular_no_plano.x * i_cos_a;
	const float uyuy_i  = vetor_unit_perpendicular_no_plano.y * vetor_unit_perpendicular_no_plano.y * i_cos_a;
	const float uxsin_a = vetor_unit_perpendicular_no_plano.x * sin_a;
	const float uysin_a = vetor_unit_perpendicular_no_plano.y * sin_a;

	float x = (cos_a  + uxux_i ) * vetor.x +             uxuy_i * vetor.y + uysin_a * vetor.z;
	float y =             uxuy_i * vetor.x + (cos_a  + uyuy_i ) * vetor.y - uxsin_a * vetor.z;
	float z =          - uysin_a * vetor.x +            uxsin_a * vetor.y +   cos_a * vetor.z;

	return Vetor3D(x,y,z);
}

void    calcular_eixos_XY_a_partir_de_eixo_Z(
	const Vetor3D& vet_unit_z, 
	float         agent_angle, 
	      Vetor3D&       Xvec, 
	      Vetor3D&       Yvec
){
	/*
	Descrição:
		Calcular vetores X e Y unitários a partir de um vetor unitário Z normal ao plano do solo.
	*/

	// Explicação Extremamente Profunda Sobre O Algoritmo Matemático

	/**
	 * There are two coordinate systems being considered in this method:
	 * - The actual agent's vision (RELATIVE system -> RELsys)
	 * - A rotated perspective where the agent's seen Zvec is the real Zvec (ROTATED system -> ROTsys)
	 * 		(the agent's optical axis / line of sight is parallel to ground ) 
	 * 
	 * SUMMARY:
	 * 		I provide an angle which defines the agent's rotation around Zvec (in the ROTsys)
	 * 		E.g. suppose the agent is rotated 5deg, then in the ROTsys, the agent sees Xvec as being rotated -5deg
	 * 		I then rotate Xvec to the RELsys, and compute the Yvec using cross product
	 * 
	 * STEPS:
	 * 1st. Compute ROTsys, by finding rotation of plane defined by normal vector Zvec, in relation to seen XY plane 
	 * (whose seen normal vector is (0,0,1)). We need: axis of rotation (unit vector lying on XY plane) and angle (rads):
	 * 		rotation axis:  
	 * 			u = (0,0,1)x(Zvec) (vector perpendicular to XY plane and Zvec)
	 * 			  = (-ZvecY,ZvecX,0) (rotation will be counterclockwise when u points towards the observer)
	 * 							     (so a negative angle will bring the agent to the ROTsys)
	 * 		angle between Zvec and (0,0,1): 
	 * 			a = acos( ((0,0,1).(Zvec)) / (|(0,0,1)|*|Zvec|) )
	 *      	  =	acos( ((0,0,1).(Zvec)) )
	 * 			  =	acos( ZvecZ )
	 * 
	 * 2nd. Establish Xvec in ROTsys:
	 * 		Let agent_angle be the agent's angle. Then Xvec's angle is (b=-agent_angle).
	 * 			Xvec = (cos(b),sin(b),0)
	 * 
	 * 3rd. Rotate Xvec to RELsys:
	 * 		Let R be the rotation matrix that rotates from ROTsys to RELsys (positive angle using u):
	 * 		Xvec = R * Xvec
 	 * 		     = R * (cos(b),sin(b),0)
	 *           = (R00 * cos(b) + R01 * sin(b), R10 * cos(b) + R11 * sin(b), R20 * cos(b) + R21 * sin(b))
	 * 		where R is: (rotation matrix from axis and angle https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle)
	 * 			R00 = cos(a) + ux*ux(1-cos(a))    R01 = ux*uy(1-cos(a))
	 * 			R10 = uy*ux(1-cos(a))             R11 = cos(a) + uy*uy(1-cos(a))
	 *      	R20 = -uy*sin(a)                  R21 = ux*sin(a)
	 * 		so Xvec becomes:
	 * 			XvecX = cos(a)*cos(b) + (1-cos(a))*(ux*ux*cos(b) + ux*uy*sin(b))
	 * 			XvecY = cos(a)*sin(b) + (1-cos(a))*(uy*uy*sin(b) + ux*uy*cos(b))
	 * 			XvecZ = sin(a)*(ux*sin(b) - uy*cos(b))
	 * 
	 * 4th. To find Yvec we have two options:
	 * 		A. add pi/2 to b and compute Yvec with the same expression used for Xvec
	 * 		B. Yvec = Zvec x Xvec  (specifically in this order for original coordinate system)
	 */

	Vetor2D u = get_ground_unit_vec_perpendicular_to( vet_unit_z );

	const float& cos_a = vet_unit_z.z; 
	const float sin_a = sqrtf(1 - cos_a*cos_a);
	const float uxuy = u.x * u.y;
	const float b = -agent_angle; //Xvec's angle
	const float cos_b = cosf(b);
	const float sin_b = sinf(b);
	const float i_cos_a = 1-cos_a;

	Xvec.x = cos_a * cos_b + i_cos_a * ( u.x*u.x*cos_b + uxuy*sin_b );
	Xvec.y = cos_a * sin_b + i_cos_a * ( u.y*u.y*sin_b + uxuy*cos_b );
	Xvec.z = sin_a * ( u.x*sin_b - u.y*cos_b );

	Yvec = vet_unit_z.CrossProduct(Xvec); //Using original coordinate system
}

// Algumas variáveis estão como seus respectivos nomes em Inglês pq é realmente melhor.

class LocalizerV2 {
	friend class Singular<LocalizerV2>;

private: 

	///////////////////////////////////////////////////////////////////////////////
	/// Métodos Privados Principais
	///////////////////////////////////////////////////////////////////////////////

	bool obter_orientacao_do_eixo_z(){
		/*
		Conforme descrito na documentação .md, vamos fazer o algoritmo demonstrado.

		Retornará True  caso consiga.
		Retornará False caso não consiga.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		const int numero_de_traves_de_gol = campo_existente.lista_de_goalposts.size();

		if(
			campo_existente.quantidade_de_marcadores_de_chao_nao_colineares >= 3
		){

			// Basta então
			// calcular_plano_do_chao_e_altura_do_agente();

			return true;
		}

		/*
		Se chegamos neste ponto, há mais de 0 linhas. 

		- Ter 1 linha, é o mais comum.
		- Ter 2 linhas também é possível caso uma delas seja pequena demais para criar 2 marcadores.
		- Ter 2 linhas pequenas também é uma possibilidade, oq é ruim para nós.

		Sendo assim, devemos buscar a maior linha e verificar se ela é grande o suficiente.
		*/

		const Linha *linha_qualquer = &campo_existente.lista_de_todos_os_segmentos.front();
		
		if(
			campo_existente.lista_de_todos_os_segmentos.size() > 1
		){

			for(
				const Linha& outra_linha_qualquer : campo_existente.lista_de_todos_os_segmentos
			){

				if(
					outra_linha_qualquer.comprimento > (*linha_qualquer).comprimento
				){

					// Há uma nova linha maior
					linha_qualquer = &outra_linha_qualquer;
				}
			}
		}

		if(
			// A maior linha é muito pequena para gerar pontos de referência.
			(*linha_qualquer).comprimento < 1
		){

			atualizar_estado_do_sistema(FAILzLine);
			return false;
		}

		if(	
			// 1 extremidade e 1 linha ou 2 linhas pequenas
			numero_de_traves_de_gol == 0
		){

			atualizar_estado_do_sistema(FAILzNOgoal);
			return false;
		}

		/*
		Vamos nos preparar para as soluções A e B.
		*/

		// Obter qualquer ponto da linha que cruza as traves
		Vetor3D pt_crossline;
		vector<RobovizField::sMarcador>& lista_de_traves = campo_existente.lista_de_goalposts;
		
		if(
			// Caso haja somente 1
			numero_de_traves_de_gol == 1
		){

			pt_crossline = lista_de_traves.front().pos_rel_cart;
		}
		else{
			if(
				// Caso haja 2 traves do mesmo  gol
				// Comparamos x pois o campo está na horizontal.
				lista_de_traves[0].spos_abs.x = lista_de_traves[1].spos_abs.x
			){

				pt_crossline = Vetor3D::ponto_medio(
														   lista_de_traves[0].pos_rel_cart,
														   lista_de_traves[1].pos_rel_cart
														 );
			}
			else{

				// Extremamente raro, há outras soluções possíveis
				// Mas computacionalmente desvantajosas. Logo:
				pt_crossline = lista_de_traves.front().pos_rel_cart;
			}
		}

		/*
		Identificamos e aplicamos as soluções A e B
		*/
		Vetor3D ponto_da_reta_mais_perto = (*linha_qualquer).ponto_na_reta_mais_perto_cart(pt_crossline);
		Vetor3D possivel_Z_vec = pt_crossline - ponto_da_reta_mais_perto;

		float possivel_comprimento_Z_vec = possivel_Z_vec.modulo();
		if(
			// Nesta verificação, definimos soluções A e B
			fabsf(
				possivel_comprimento_Z_vec - 0.8 
			) < 0.05
		){

			Vetor3D unitario_Z_vec = possivel_Z_vec / possivel_comprimento_Z_vec;

			// Sabemos a orientação
			_Head_to_Field_Prelim.setar(2, 0, unitario_Z_vec.x);
			_Head_to_Field_Prelim.setar(2, 1, unitario_Z_vec.y);
			_Head_to_Field_Prelim.setar(2, 2, unitario_Z_vec.z);

			// Ir para próximo passo
			obter_head_z(unitario_Z_vec);
			return true;
		}

		// Caso não seja solução A e B, vamos aplicar a solução C

		Vetor3D pt_crossline_left, midp_crossline;

		// Todos são sFixedMkr
		const auto& goal_mm = RobovizField::gMkrs::goal_mm;
		const auto& goal_mp = RobovizField::gMkrs::goal_mp;
		const auto& goal_pm = RobovizField::gMkrs::goal_pm;
		const auto& goal_pp = RobovizField::gMkrs::goal_pp;

		if(
			goal_mm.se_esta_visivel && goal_mp.se_esta_visivel
		){

			pt_crossline_left = goal_mm.pos_rel_cart - goal_mp.pos_rel_cart;
			midp_crossline    = (goal_mm.pos_rel_cart + goal_mp.pos_rel_cart) / 2;
		}
		else{

			if(
				goal_pp.se_esta_visivel && goal_pm.se_esta_visivel
			){

				pt_crossline_left = goal_pp.pos_rel_cart - goal_pm.pos_rel_cart;
				midp_crossline    = (goal_pp.pos_rel_cart + goal_pm.pos_rel_cart) / 2;
			}
		}

		/*
		Verifica se o ângulo entre a linha e a linha de traves está entre 45° e 135°.

		Matemática:
			45deg < acos(line.crossbar / |line||crossbar|) < 135deg  <=>
	  		| line.crossbar / |line||crossbar| | < cos(45deg)        <=>
	  		| line.crossbar | < cos(45deg) * |line| * ~2.1           <=>
	  		| line.crossbar | < 1.485 * |line|
		*/
		Vetor3D vet_diret = (*linha_qualquer).obter_diretor_cart();
		if(
			numero_de_traves_de_gol > 1 && fabsf(
												  vet_diret.InnerProduct(
												  						  pt_crossline_left
												  						)
												) < 1.485 * (*linha_qualquer).comprimento
		){

			possivel_Z_vec = vet_diret.CrossProduct(
													pt_crossline_left
												   );

			possivel_Z_vec = possivel_Z_vec * ((
				(*linha_qualquer).inicio_c.obter_distancia(
											midp_crossline
										  ) > (*linha_qualquer).final_c.obter_distancia(
						  												 midp_crossline
						  											   )) ? 1.0 : -1.0);

			possivel_Z_vec = possivel_Z_vec.normalize();

			_Head_to_Field_Prelim.setar(2,0,possivel_Z_vec.x);
			_Head_to_Field_Prelim.setar(2,1,possivel_Z_vec.y);
			_Head_to_Field_Prelim.setar(2,2,possivel_Z_vec.z);

			obter_head_z(possivel_Z_vec);
			return true;
		}

		// Caso chegue até aqui
		atualizar_estado_do_sistema(FAILz);
		return false;
	}

	void calcular_plano_do_chao_e_altura_do_agente(){
		/*
			- Estimar o melhor plano de chão baseado nos marcadores de chão.
			
			- Calcular a altura do agente baseado no centróide dos marcadores de chão.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		const vector<RobovizField::sMarcador>& marcadores_de_chao_pesados = campo_existente.lista_de_marcadores_de_chao_pesados;
		const int quantidade_de_marcadores = marcadores_de_chao_pesados.size();

		// ---------------------------------------------------------------------------------

		// Iniciar determinados vetores e matrizes
		gsl_matrix *A, *V;
		gsl_vector *S, *vec_aux;

		A = gsl_matrix_alloc(
			quantidade_de_marcadores,
			3
		);
		V = gsl_matrix_alloc(
			3,
			3
		);
		S = gsl_vector_alloc(
			3
		);
		vec_aux = gsl_vector_alloc(
			3
		);

		// Obter centróide
		Vetor3D centroide(0, 0, 0);
		for(
			const RobovizField::sMarcador& mkr_chao : marcadores_de_chao_pesados
		){

			centroide += mkr_chao.pos_rel_cart;
		}

		centroide /= (float) quantidade_de_marcadores;

		// Inserir todos os marcadores dentro da matriz após subtrair do centróide
		for(
			int i = 0;
				i < quantidade_de_marcadores;
				i++
		){

			gsl_matrix_set( A, i, 0, marcadores_de_chao_pesados[i].pos_rel_cart.x - centroide.x );
			gsl_matrix_set( A, i, 1, marcadores_de_chao_pesados[i].pos_rel_cart.y - centroide.y );
			gsl_matrix_set( A, i, 2, marcadores_de_chao_pesados[i].pos_rel_cart.z - centroide.z );
		}

		/*
		Utiliza a ferramenta SVD já disponível.

		Decomporá a matriz A em U * S * V^(T), matrizes as quais:

		U -> matriz ortogonal dos vetores próprios da matriz AA^(T)
		S -> vetor com valores singulares
		V -> matriz ortogonal dos vetores próprios da matriz A^(T)A
		*/
		gsl_linalg_SV_decomp( A, V, S, vec_aux);

		// Capturar o plano ax + by + cy = d
		double a = gsl_matrix_get( V, 0, 2 );
		double b = gsl_matrix_get( V, 1, 2 );
		double c = gsl_matrix_get( V, 2, 2 );
		double d = a * centroide.x + b * centroide.y + c * centroide.z;

		// Observe que |d| é caracterizado como uma estimativa da altura do agente.

		gsl_matrix_free (A);
		gsl_matrix_free (V);
		gsl_vector_free (S);
		gsl_vector_free (vec_aux);

		// --------------------------------------------------------------------------------------

		/*
		Observe que não necessariamente o vetor normal (a, b, c) apontará para cima, sendo assim
		devemos gerenciar as regiões nas quais temos certezas.

		Sabe-se que:
			
			- ax + by + cz - d > 0  // SemiEspaço Superior
			- ax + by + cz - d = 0  // Plano
			- ax + by + cz - d < 0  // SemiEspaço Diminui

		Já que o agente está sempre acima do chão, basta que determinemos que o agente está no 
		mesmo semiespaço que o vetor normal.
		Para fazer isso, checamos se a origem do sistema relativo de coordenadas, cabeça do agente,
		retorna:

			- ax + by + cz - d > 0
			- - d > 0  // Cabeça do agente está em (0, 0, 0).
			-   d < 0

		Entretanto, caso o agente esteja caído, o plano otimizado pode estar deslocado. Sendo assim,
		caso tiver um ponto de referência melhor, podemos usar.
		*/

		if(
			! campo_existente.lista_de_goalposts.empty()
		){

			const Vetor3D& pt_aereo = campo_existente.lista_de_goalposts.front().pos_rel_cart;
			if(
				// Caso o gol esteja no semiespaço negativo, devemos inverter o vetor normal
				a * pt_aereo.x + b * pt_aereo.y + c * pt_aereo.z < d 
			){

				a = -a;
				b = -b;
				c = -c;
			}
		}
		else{

			if(
				// Caso não haja referências disponíveis, nos reta a cabeça do agente mesmo.
				//
				d > 0
			){

				a = -a;
				b = -b;
				c = -c;
			}
		}

		// Salvamos a informação do vetor normal como nova orientação do vetor.
		_Head_to_Field_Prelim.setar(2, 0, a);
		_Head_to_Field_Prelim.setar(2, 1, b);
		_Head_to_Field_Prelim.setar(2, 2, c);

		// Calculamos a altura do agente
		float altura = max(
 							- (centroide.x * a + centroide.y * b + centroide.z * c),
 							0.064
						  );

		_Head_to_Field_Prelim.setar(2, 3, altura);

		// Setamos os pontos 
		ultima_altura_conhecida = _final_z;
		_final_z = altura;
		_se_head_z_esta_pronta_para_atualizacao = true;
	}

	void obter_head_z(Vetor3D& Z_vec){
		/*
		Computar translação de Z, vulgo altura.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		Vetor3D vetor_resultante;
		for(
			const RobovizField::sMarcador& g_mkr : campo_existente.lista_de_marcadores_de_chao_pesados
		){

			vetor_resultante += g_mkr.pos_rel_cart;
		}

		vetor_resultante /= campo_existente.lista_de_marcadores_de_chao_pesados.size();

		// Mínima Altura
		float z = max(

			- vetor_resultante.InnerProduct(Z_vec),
			0.064f
		);

		_Head_to_Field_Prelim.setar(2, 3, z);

		ultima_altura_conhecida = _final_z;
		_final_z = z;
		_se_head_z_esta_pronta_para_atualizacao = true;
	}

    bool refinamento_aux(
    	float &initial_angle,
    	float &initial_x, 
    	float &initial_y, 
    	bool  use_probabilities
    ){
    	/*
		Descrição:
			Aplicamos o refinamento diretamente em:

				- initial_angle, initial_x, initial_y ( caso seja use_probabilities == false )
    			- _Head_to_Field_prelim ( caso use_probabilites == true)
		Parâmetros:
			-> initial_angle:
				Ângulo inicial entre Xvec e Zvec

			-> initial_x, initial_y
				Translações em x e y.

		A função a seguir é puramente matemática.
    	*/

    	int status, iter=0;
		gsl_vector* x  =  create_gsl_vector<3>({initial_x, initial_y, initial_angle});    // Initial transformation 
		gsl_vector* step_sizes = create_gsl_vector<3>({0.02, 0.02, 0.03});                // Set initial step sizes 
		gsl_multimin_function minex_func   = {map_error_euclidian_distance, 3, nullptr};  // error func, variables no., params
		if(use_probabilities) minex_func.f =  map_error_logprob;				          // probablity-based error function

		const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;   // algorithm type
		gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc (T, 3);            // allocate workspace
	  	gsl_multimin_fminimizer_set (s, &minex_func, x, step_sizes);                 // set workspace

		float best_x, best_y, best_ang;


	  	do{
			iter++;
			status = gsl_multimin_fminimizer_iterate(s);

			//*s holds the best solution, not the last solution
			best_x   = gsl_vector_get (s->x, 0);
			best_y   = gsl_vector_get (s->x, 1);
			best_ang = gsl_vector_get (s->x, 2);

			if (status) break;

			double size = gsl_multimin_fminimizer_size (s); //minimizer-specific characteristic size
			status = gsl_multimin_test_size (size, 1e-3); //This size can be used as a stopping criteria, as the simplex contracts itself near the minimum

	    }
		while ((status == GSL_CONTINUE || use_probabilities) && iter < 40);

		float best_map_error = s->fval;

		gsl_vector_free(x);
		gsl_vector_free(step_sizes);
		gsl_multimin_fminimizer_free (s);

		if(!use_probabilities){
			if(best_map_error > 0.10){
				atualizar_estado_do_sistema(FAILtune);
				return false;
			}else{
				initial_angle = best_ang;
				initial_x = best_x;
				initial_y = best_y;
				return true;
			}
		}

		/**
		 * At this point, use_probabilities is true
		 * Note: The transformations are directly tested on prelimHeadToField but it currently
		 * holds the last test, so we set it manually here to the best found solution
		 */

		//Convert angle into Xvec and Yvec
		Vetor3D Zvec(_Head_to_Field_Prelim.get(2,0), _Head_to_Field_Prelim.get(2,1), _Head_to_Field_Prelim.get(2,2));
		Vetor3D Xvec, Yvec;
		calcular_eixos_XY_a_partir_de_eixo_Z(Zvec, best_ang, Xvec, Yvec );

		_Head_to_Field_Prelim.set(0,0, Xvec.x);
		_Head_to_Field_Prelim.set(0,1, Xvec.y);
		_Head_to_Field_Prelim.set(0,2, Xvec.z);
		_Head_to_Field_Prelim.set(0,3, best_x);
		_Head_to_Field_Prelim.set(1,0, Yvec.x);
		_Head_to_Field_Prelim.set(1,1, Yvec.y);
		_Head_to_Field_Prelim.set(1,2, Yvec.z);
		_Head_to_Field_Prelim.set(1,3, best_y);

		return true;
    }

    bool refinamento(
    	float initial_angle,
    	float initial_x,
    	float initial_y
    );

    /*
	Obtém translação e rotação nos eixos X e Y.

	Solução única é garantida se Zvec estiver na direção certa.

	Necessário pelo menos 2 marcadores.
    */
    bool obter_translacao_rotacao_xy(){

    	RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

    	Vetor3D Zvec(
    		_Head_to_Field_Prelim.get(2,0), 
    		_Head_to_Field_Prelim.get(2,1), 
    		_Head_to_Field_Prelim.get(2,2)
    	);

    	RobovizField::sMarcador *mkr_1 = nullptr;
    	RobovizField::sMarcador *mkr_2 = nullptr;

    	if(
    		campo_existente.lista_de_corners.size() > 1
    	){

    		mkr_1 = &(campo_existente.lista_de_corners[0]);
    		mkr_2 = &(campo_existente.lista_de_corners[1]);
    	}
    	else{

    		if(
    			campo_existente.lista_de_corners.size() == 1
    		){

    			mkr_1 = &(  campo_existente.lista_de_corners[0]);
    			mkr_2 = &(campo_existente.lista_de_goalposts[0]);
    		}
    		else{

    			mkr_1 = &(campo_existente.lista_de_goalposts[0]);
    			mkr_2 = &(campo_existente.lista_de_goalposts[1]);
    		}
    	}

    	Vetor3D real_vec(
    					  (*mkr_2).spos_abs.x - (*mkr_1).spos_abs.x,
    					  (*mkr_2).spos_abs.y - (*mkr_1).spos_abs.y,
    					  (*mkr_2).spos_abs.z - (*mkr_1).spos_abs.z 
    					);
    	float real_angle = atan2f(real_vec.y, real_vec.x);

		Vetor3D seen_vec((*mkr_2).pos_rel_cart - (*mkr_1).pos_rel_cart);
		Vetor3D vec_rotacao = rotacionar_em_torno_do_eixo_do_chao(seen_vec, Zvec);

		float seen_angle = atan2f(vec_rotacao.y, vec_rotacao.x);
		/*Observe que, por serem vetores e não segmentos/linhas, os ângulos podem ser trocados.*/ 	

		float agente_angle = real_angle - seen_angle;

		Vetor3D Xvec, Yvec;
		calcular_eixos_XY_a_partir_de_eixo_Z(Zvec, agent_angle, Xvec, Yvec);

		/** Explicação Detalhada do que estamos propondo
		 * 
		 * Let m be a landmark, rel:(mx,my,mz), abs:(mabsx, mabsy, mabsz)
		 * XvecX*mx + XvecY*my + XvecZ*mz + AgentX = mabsx
		 * AgentX = mabsx - (XvecX*mx + XvecY*my + XvecZ*mz)
		 * AgentX = mabsx - (XvecX . m)
		 * 
		 * Generalizing for N estimates:
		 * AgentX = sum( mabsx - (XvecX . m) )/N
		 */
		float initial_x = 0, initial_y = 0;
		for(
			const RobovizField::sMarcador& mkr : campo_existente.lista_de_corners_e_goalposts
		){

			initial_x += mkr.spos_abs.x - Xvec.InnerProduct(mkr.pos_rel_cart);
			initial_y += mkr.spos_abs.y - Yvec.InnerProduct(mkr.pos_rel_cart);
		}

		initial_x /= campo_existente.lista_de_corners_e_goalposts.size();
		initial_y /= campo_existente.lista_de_corners_e_goalposts.size();

		return refinamento(agente_angle, initial_x, initial_y);
    }

    bool guess_xy(){
    	/*
		Processo extremamente complexo e de outro patamar de dificuldade.
		Aparentemente, trata-se de um modelo de decisão para escolha
		de vetores x e y.
    	*/

    	RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

    	// Obter vetores de estágios anteriores
		Vetor3D Zvec(
			_Head_to_Field_Prelim.get(2,0), 
			_Head_to_Field_Prelim.get(2,1), 
			_Head_to_Field_Prelim.get(2,2)
		);
		Vetor2D ultima_altura_conhecida_2d(posicao_da_cabeca.x, posicao_da_cabeca.y);

		//------------------------------------------------------------ Get longest line and use it as X or Y vector

		const Linha* maior_linha = &campo_existente.lista_de_todos_os_segmentos.front();
		for(const Linha& linha_qualquer : campo_existente.lista_de_todos_os_segmentos){

			if(linha_qualquer.comprimento > (*maior_linha).comprimento) longestLine = &linha_qualquer;
		}

		if(
			// Caso a linha seja pequena demais.
			(*maior_linha).comprimento < 1.6
		){

			atualizar_estado_do_sistema(FAILguessLine);
			return false; 
		}

		//Rotate line to real ground plane, where it loses the 3rd dimension
		Vetor3D longestLineVec = maior_linha->final_c - maior_linha->inicio_c;
		Vetor3D rotated_abs_line = rotacionar_em_torno_do_eixo_do_chao(longestLineVec, Zvec);

		//The line can be aligned with X or Y, positively or negatively (these angles don't need to be normalized) 
		float fixed_angle[4];
		fixed_angle[0] = -atan2f(rotated_abs_line.y,rotated_abs_line.x); //if longestLineVec is Xvec
		fixed_angle[1] = fixed_angle[0] + 3.14159265f; //if longestLineVec is -Xvec
		fixed_angle[2] = fixed_angle[0] + 1.57079633f; //if longestLineVec is Yvec
		fixed_angle[3] = fixed_angle[0] - 1.57079633f; //if longestLineVec is -Yvec

		//------------------------------------------------------------ Get initial translation

		//if we see 1 landmark, we use it, if not, we get the last position

		float initial_x[4], initial_y[4];
		bool noLandmarks = campo_existente.lista_de_corners_e_goalposts.empty();

		if(noLandmarks){

			for(int i=0; i<4; i++){
				initial_x[i] = ultima_altura_conhecida_2d.x;
				initial_y[i] = ultima_altura_conhecida_2d.y;
			}

		} else {

			Vetor3D Xvec = longestLineVec / maior_linha->comprimento;
			Vetor3D Yvec(Zvec.CrossProduct(Xvec));

			/**
			 * Let m be a landmark, rel:(mx,my,mz), abs:(mabsx, mabsy, mabsz)
			 * XvecX*mx + XvecY*my + XvecZ*mz + AgentX = mabsx
			 * AgentX = mabsx - (XvecX*mx + XvecY*my + XvecZ*mz)
			 * AgentX = mabsx - (XvecX . m)
			 */

			const RobovizField::sMarcador& mkr = campo_existente.lista_de_corners_e_goalposts.front();
			const float x_aux = Xvec.InnerProduct(mkr.pos_rel_cart);
			const float y_aux = Yvec.InnerProduct(mkr.pos_rel_cart);

			initial_x[0] = mkr.spos_abs.x - x_aux;
			initial_y[0] = mkr.spos_abs.y - y_aux;
			initial_x[1] = mkr.spos_abs.x + x_aux; // 2nd version: X is inverted
			initial_y[1] = mkr.spos_abs.y + y_aux; // 2nd version: Y is inverted
			initial_x[2] = mkr.spos_abs.x + y_aux; // 3rd version: X is inverted Y
			initial_y[2] = mkr.spos_abs.y - x_aux; // 3rd version: Y is X
			initial_x[3] = mkr.spos_abs.x - y_aux; // 4th version: X is Y
			initial_y[3] = mkr.spos_abs.y + x_aux; // 4th version: Y is inverted X
		}
		

		//------------------------------------------------------------ Optimize XY rotation for each possible orientation
		/*
		Simplesmente não tem o que fazer aqui, é um absurdo de incrível
		*/

		const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
		gsl_multimin_fminimizer *s[4] = {nullptr,nullptr,nullptr,nullptr};
		gsl_vector *ss[4], *x[4];
		gsl_multimin_function minex_func[4];

		size_t iter = 0;
		int status;
		double size;

		for(int i=0; i<4; i++){
			x[i]  = create_gsl_vector<2>({initial_x[i], initial_y[i]}); // Initial transformation 
			ss[i] = create_gsl_vector<2>({1, 1}); //Set initial step sizes to 1

			/* Initialize method */
			minex_func[i].n = 2;
			minex_func[i].f = map_error_euclidian_distance;
			minex_func[i].params = &fixed_angle[i];	

			s[i] = gsl_multimin_fminimizer_alloc (T, 2);
	  		gsl_multimin_fminimizer_set (s[i], &minex_func[i], x[i], ss[i]);
		}

		/* start iterating */
		bool running[4] = {true,true,true,true};
		float current_error[4] = {1e6,1e6,1e6,1e6};
		float lowest_error = 1e6;
		Vetor2D best_xy[4];
		const int maximum_iterations = 50;
		bool plausible_solution[4] = {false,false,false,false};
	  	do{
			iter++;
			for(int i=0; i<4; i++){
				if(!running[i]) continue;

				status = gsl_multimin_fminimizer_iterate(s[i]);

				current_error[i] = s[i]->fval;
				if(current_error[i] < lowest_error) lowest_error = current_error[i];

				// Possible errors: 
				// GSL_ERROR ("incompatible size of x", GSL_EINVAL); This should only be a concern during code design
				// GSL_ERROR ("contraction failed", GSL_EFAILED); Evaluation function produced non finite value
				if (status) { 
					running[i]=false; //This is not a valid solution
					continue; 
				}

				size = gsl_multimin_fminimizer_size (s[i]); //minimizer-specific characteristic size
				status = gsl_multimin_test_size (size, 1e-2); //This size can be used as a stopping criteria, as the simplex contracts itself near the minimum

				if(status != GSL_CONTINUE || (lowest_error * 50 < current_error[i])) { //finished or aborted
					best_xy[i].x = gsl_vector_get (s[i]->x, 0);
					best_xy[i].y = gsl_vector_get (s[i]->x, 1);
					running[i]=false; 
					plausible_solution[i]=(status == GSL_SUCCESS); //only valid if it converged to local minimum
					continue; 
				} 


			}	


	    } while (iter < maximum_iterations && (running[0] || running[1] || running[2] || running[3]));

		for(int i=0; i<4; i++){
			gsl_vector_free(x[i]);
			gsl_vector_free(ss[i]);
			gsl_multimin_fminimizer_free (s[i]);
		}


		//At this point, a solution is plausible if it converged to a local minimum
		//So, we apply the remaining criteria for plausiblity
		int plausible_count = 0;
		int last_i;
		for(int i=0; i<4; i++){
			if(!plausible_solution[i]) continue;
			bool isDistanceOk = (!noLandmarks) || posicao_da_cabeca.obter_distancia(best_xy[i]) < 0.5; //  distance to last known position
			if(current_error[i] < 0.12 && isDistanceOk){ // mapping error  
				plausible_count++; 
				last_i = i;
			}
		}

		// If there is 1 landmark, and multiple options, the distance to last known pos is now used to eliminate candidates
		if(!noLandmarks && plausible_count>1){
			plausible_count = 0;
			for(int i=0; i<4; i++){
				if(plausible_solution[i] && posicao_da_cabeca.obter_distancia(best_xy[i]) < 0.5){ // distance to last known position
					plausible_count++; 
					last_i = i;
				}
			}
		}

		//Check if best solution is good if all others are not even plausible
		if(plausible_count==0){
			atualizar_estado_do_sistema(FAILguessNone);
			return false; 
		}else if(plausible_count>1){
			atualizar_estado_do_sistema(FAILguessMany);
			return false;
		}else if(current_error[last_i] > 0.06 || (noLandmarks && posicao_da_cabeca.obter_distancia(best_xy[last_i]) > 0.3)){ // mapping error  /  distance to last known position
			atualizar_estado_do_sistema(FAILguessTest);
			return false;
		}

		return refinamento(fixed_angle[last_i],best_xy[last_i].x, best_xy[last_i].y);
	}

    /* Métodos de Mapeação de Erros */
    
	static double map_error_logprob(
    	const gsl_vector *vet,
    				void *params
    ){
		/*
		Descrição:
			Calcular mapa de erros usando probabilidade de distâncias.

			Observe que, por ser estática, essa função será executada mais de 
			uma vez em vários momentos.

		Retorno:
			- log( probabilidade_normalizada = (p1*p2*p3*...*pn)^(1/n) )
		*/

		float angle;
		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		// Obtemos o ângulo a aprtir 
		angle = ((*vet).size() == 3) ? gsl_vector_get(vet, 2) : *(float *) params;

		// Pegamos a matriz de transformação
		Matriz& _Head_to_Field_Prelim_instantanea = Singular<LocalizerV2>::obter_instancia()._Head_to_Field_Prelim;
		Vetor3D Z_vec( 
			_Head_to_Field_Prelim_instantanea.get(2, 0),
			_Head_to_Field_Prelim_instantanea.get(2, 1),
			_Head_to_Field_Prelim_instantanea.get(2, 2),			
		);

		Vetor3D X_vec, Y_vec;
		calcular_eixos_XY_a_partir_de_eixo_Z( Z_vec, angle, X_vec, Y_vec );

		// Estes serão os coeficientes que serão otimizados.
		_Head_to_Field_Prelim_instantanea.setar( 0, 0, X_vec.x);
		_Head_to_Field_Prelim_instantanea.setar( 0, 1, X_vec.y);
		_Head_to_Field_Prelim_instantanea.setar( 0, 2, X_vec.z);
		_Head_to_Field_Prelim_instantanea.setar( 0, 3, gsl_vector_get(vet, 0));
		_Head_to_Field_Prelim_instantanea.setar( 1, 0, Y_vec.x);
		_Head_to_Field_Prelim_instantanea.setar( 1, 1, Y_vec.y);
		_Head_to_Field_Prelim_instantanea.setar( 1, 2, Y_vec.z);
		_Head_to_Field_Prelim_instantanea.setar( 1, 3, gsl_vector_get(vet, 1));


		Matriz _Field_to_Head_Prelim_instantanea = _Head_to_Field_Prelim_instantanea.obter_matriz_de_transformacao_inversa();

		double total_log_prob      = 0;
		int    total_error_counter = 0;
    
    	// Adicionaremos o log da probabilidade de marcadores desconhecidos
		for(
			const RobovizField::sMarcador& mkr_desc : campo_existente.lista_de_marcadores_desconhecidos
		){

			// Sabemos o segmento mais perto, logo podemos trazer para perto 
			Vetor3D inicio_rel_cart = _Field_to_Head_Prelim_instantanea * ( (*(*mkr_desc.segm).spts[0]).obter_vetor() );
			Vetor3D final_rel_cart  = _Field_to_Head_Prelim_instantanea * ( (*(*mkr_desc.segm).spts[1]).obter_vetor() );

			Linha segmento_mais_perto(
									  inicio_rel_cart,
									  final_rel_cart,
									  (*mkr_desc.segm).comprimento 
			                          );

			Vetor3D ponto_mais_perto_esf = segmento_mais_perto.segment_ponto_na_reta_mais_perto_cart(mkr_desc.pos_rel_cart).para_esferica();

			total_log_prob += Ruido_de_Campo::log_prob_r(ponto_mais_perto_esf.x, mkr_desc.pos_rel_esf.x);
			total_log_prob += Ruido_de_Campo::log_prob_h(ponto_mais_perto_esf.y, mkr_desc.pos_rel_esf.y);
			total_log_prob += Ruido_de_Campo::log_prob_v(ponto_mais_perto_esf.z, mkr_desc.pos_rel_esf.z);
			
			total_error_counter++;			
		}

		// Adicionaremos o log da probabilidade de marcadores conhecidos
		for(
			const RobovizField::sMarcador& mkr : campo_existente.lista_de_marcadores_identificados
		){

			// Trazer marcador para perto do agente
			Vetor3D pos_rel_esf_do_mkr = ( _Field_to_Head_Prelim_instantanea * mkr.spos_abs.obter_vetor()).para_esferica();
			
			total_logprob += Ruido_de_Campo::log_prob_r(pos_rel_esf_do_mkr.x, mkr.pos_rel_esf.x);
			total_logprob += Ruido_de_Campo::log_prob_h(pos_rel_esf_do_mkr.y, mkr.pos_rel_esf.y);
			total_logprob += Ruido_de_Campo::log_prob_v(pos_rel_esf_do_mkr.z, mkr.pos_rel_esf.z);
			
			total_error_counter++;
		}	

		// Retornamos o valor desejado. 
		// Negativo pois a otimização mínima a função de perda.
		double logNormProb = - total_logprob / total_error_counter; 

		// Evitar infinitos
		return (!gsl_finite(logNormProb)) ? 1e6 : logNormProb;
    }

    static double map_error_euclidian_distance(
    	const gsl_vector *vet,
    	void *params
    ){
    	/*
		Descrição:
			Computar mapa de erros usando distâncias euclidianas 2D
    	*/

    	/* Mesmas linhas de comando que a função anterior */
		float angle;
		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		// Obtemos o ângulo a aprtir 
		angle = ((*vet).size() == 3) ? gsl_vector_get(vet, 2) : *(float *) params;

		// Pegamos a matriz de transformação
		Matriz& _Head_to_Field_Prelim_instantanea = Singular<LocalizerV2>::obter_instancia()._Head_to_Field_Prelim;
		Vetor3D Z_vec( 
			_Head_to_Field_Prelim_instantanea.get(2, 0),
			_Head_to_Field_Prelim_instantanea.get(2, 1),
			_Head_to_Field_Prelim_instantanea.get(2, 2),			
		);

		Vetor3D X_vec, Y_vec;
		calcular_eixos_XY_a_partir_de_eixo_Z( Z_vec, angle, X_vec, Y_vec );

		// Estes serão os coeficientes que serão otimizados.
		_Head_to_Field_Prelim_instantanea.setar( 0, 0, X_vec.x);
		_Head_to_Field_Prelim_instantanea.setar( 0, 1, X_vec.y);
		_Head_to_Field_Prelim_instantanea.setar( 0, 2, X_vec.z);
		_Head_to_Field_Prelim_instantanea.setar( 0, 3, gsl_vector_get(vet, 0));
		_Head_to_Field_Prelim_instantanea.setar( 1, 0, Y_vec.x);
		_Head_to_Field_Prelim_instantanea.setar( 1, 1, Y_vec.y);
		_Head_to_Field_Prelim_instantanea.setar( 1, 2, Y_vec.z);
		_Head_to_Field_Prelim_instantanea.setar( 1, 3, gsl_vector_get(vet, 1));

		float total_error 		  = 0;
		int   total_error_counter = 0;

		for(
			const Linha& linha_qualquer campo_existente.lista_de_todos_os_segmentos
		){

			// Transformar coordenadas
			Vetor3D inicio_linha_cart = _Head_to_Field_Prelim_instantanea * linha_qualquer.inicio_c;
			Vetor3D final_linha_cart  = _Head_to_Field_Prelim_instantanea *   linha_qualquer.fina_c;

			// Computar ângulo
			float angle_da_linha = 0;
			float tolerancia_para_angulo = 0;  // Apenas um valor de mínimo

			if(
				// Comprimento para que seja grande o suficiente
				linha_qualquer.comprimento > 0.8
			){

				angle_da_linha = atan2f(
										  final_linha_cart.y - inicio_linha_cart.y,
										  final_linha_cart.x - inicio_linha_cart.x
										);
				if(
					angle_da_linha < 0
				){	
					// Como é uma linha, não desejamos que possua valores negativos de ângulos.
					angle_da_linha += 3.14159265f;
				}

				tolerancia_para_angulo = 0.35f;  // Cerca de 20°
			}
			else{

				// Aqui o sanha começa, o custo começa a ficar alto demais, teremos um O(n^2).
				if(
					campo_existente.lista_de_todos_os_segmentos.size() <= 3
				){

					for(
						const Linha& linha_grande : campo_existente.lista_de_todos_os_segmentos
					){

						if(
							// Caso não seja uma linha grande ou seja a mesma linha do loop anterior
							linha_grande.comrimento < 2 || &linha_grande == &linha_qualquer
						){

							continue;	
						}

						if(
							// Se estiverem quase se cruzando
							linha_grande.segment_distancia_ate_segment(linha_qualquer) < 0.5
						){
							// Observe que pode gerar falsos positivos com as linhas mediais e verticais
							// Entretanto, podem ser descartados

							Vetor3D inicio_da_linha_grande_cart = _Head_to_Field_Prelim_instantanea * linha_grande.inicio_c;
							Vetor3D final_da_linha_grande_cart  = _Head_to_Field_Prelim_instantanea *  linha_grande.final_c;

							angle_da_linha = atan2f(
													 final_da_linha_grande_cart.y - inicio_da_linha_grande_cart.y,
										 			 final_da_linha_grande_cart.x - inicio_da_linha_grande_cart.x
												   );

							// Sem saco para fatorar isso
							// add 90deg while keeping the angle between 0-180deg (same logic used when l.length > 0.8)
							if     (angle_da_linha < -1.57079632f){ angle_da_linha += 4.71238898f; } //Q3 -> add pi*3/2
							else if(angle_da_linha < 0           ){ angle_da_linha += 1.57079632f; } //Q4 -> add pi/2
							else if(angle_da_linha < 1.57079632f ){ angle_da_linha += 1.57079632f; } //Q1 -> add pi/2
							else                           		  { angle_da_linha -= 1.57079632f; } //Q2 -> subtract pi/2

							tolerancia_para_angulo = 1.22f;
							break;
						}
					}
				}
			}

			float min_error = 1e6f;
			for(
				const auto& segm_importante : RobovizField::cSegmentos::list 
			){

				// Pular a linha caso ela seja muito maior que a outra
				if(
					linha_qualquer.comprimento > (segm_importante + 0.7)
				){

					continue;
				}

				// Pular caso a orientação da linha não seja semelhante
				float angle_diff = fabsf(angle_da_linha - segm_importante.ang);
				if(
					angle_diff > 1.57079632f
				){

					angle_diff = 3.14159265f - angle_diff;
				}
				if(
					angle_diff > tolerancia_para_angulo
				){

					continue;
				}

				// O erro será a soma de distância entre um segmento de linha para ambas extremidades vistas
				float error = RobovizField::calcular_dist_segm_para_pt2d_c(segm_importante, inicio_linha_cart.para_2D());

				if(
					error < min_error
				){

					error += RobovizField::calcular_dist_segm_para_pt2d_c(segm_importante, final_linha_cart.para_2D());
				}

				if(
					error < min_error 
				){

					min_error = error;
				}
			}

			total_error += min_error;
			total_error_counter += 2;  // Como a linha tem 2 extremidades
		}


		for(
			const RobovizField::sMarcador& mkr : campo_existente.lista_de_corners_e_goalposts
		){

			Vetor3D pt = _Head_to_Field_Prelim_instantanea * mkr.pos_rel_cart;

			float error = pt.para_2D().obter_distancia(Vetor2D(mkr.spos_abs.x, mkr.spos_abs.y));
			total_error += (error > 0.5) ? error * 100 : error;
			total_error_counter++;
		}

		double media_de_error = total_error / total_error_counter;

		return (!gsl_finite(media_de_error)) ? 1e6 : media_de_error;
    }

    /* Método de Atualização para de Variáveis Públicas */

    const Matriz &Head_to_Field_Transform = _final_Head_to_Field_Transform; // -> rotação + translação
	const Matriz &Head_to_Field_Rotate    = _final_Head_to_Field_Rotate;    // -> rotação
	const Matriz &Field_to_Head_Transform = _final_Field_to_Head_Transform; // inversa da primeira
	const Matriz &Field_to_Head_Rotate    = _final_Field_to_Head_Rotate;


    void commit_system(){
    	/*
		Descrição:
			Atualizará todas as variáveis públicas após ter feito os testes e confirmações
			das variáveis privadas.
		*/

    	// Obtemos do relativo para absoluto
    	_final_Head_to_Field_Transform = _Head_to_Field_Prelim;  

    	// Obtemos do absoluto para relativo
    	_final_Head_to_Field_Transform.obter_inversa(_final_Field_to_Head_Transform);

    	int index_aux = 0;
    	for(
    		int i = 0;
    			i < 3;
    			i++
    	){

    		// Rotações do relativo para absoluto
    		_final_Head_to_Field_Rotate.setar( i + index_aux, _final_Head_to_Field_Transform.get(i + index_aux ));

    		// Rotações do absoluto para relativo
    		_final_Field_to_Head_Rotate.setar( i + index_aux, _final_Field_to_Head_Transform.get(i + index_aux ));
    		
    		index_aux += 4;
    	}

    	_final_translacao = _final_Head_to_Field_Transform.obter_vetor_de_translacao();

    	_se_head_z_esta_pronta_para_atualizacao = true;

    	_passos_desde_ultima_atualizacao = 0;

    	/*
		Atualizamos o histórico
		Note que devido à quantidade fixa devemos nos atentar a isso
    	*/
    	_historico_de_translacao[_counter_historico_de_translacao++] = _final_translacao;
    	if(
    		_counter_historico_de_translacao >= _historico_de_translacao.size()
    	){

    		_counter_historico_de_translacao = 0;
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
	/// Métodos Privados de Transformação Matricial
	///////////////////////////////////////////////////////////////////////////////

    Matriz _Head_to_Field_Prelim = Matriz();

    void resetar(){
    	/*
		Vamos resetar a matriz de transformação para uma versão preliminar.
    	*/

    	for(
    		int i = 0;
    			i < 12;
    			i++
    	){		
    		// Este será o padrão de inicialização.
    		_Head_to_Field_Prelim.setar(i, 9999);
    	}
    }

    //FINAL MATRIX - the user has access to a public const reference of the variables below
    Matriz _final_Head_to_Field_Transform; // rotation + translation
    Matriz _final_Head_to_Field_Rotate;    // rotation
    Matriz _final_Field_to_Head_Transform; // rotation + translation
    Matriz _final_Field_to_Head_Rotate;    // rotation

    Vetor3D  _final_translacao;

    float    _final_z; //independent z translation (may be updated more often)

    ///////////////////////////////////////////////////////////////////////////////
	/// Estatística Avançada
	///////////////////////////////////////////////////////////////////////////////

    std::array<Vetor3D, 10> _historico_de_translacao;
    unsigned int _counter_historico_de_translacao = 0;

    float ultima_altura_conhecida = 0.5; 

    unsigned int _passos_desde_ultima_atualizacao =     0;
    bool 		 _se_esta_pronta_para_atualizacao = false; // Leia a  definição dentro da parte pública
    bool  _se_head_z_esta_pronta_para_atualizacao = false;


    ///////////////////////////////////////////////////////////////////////////////
	/// Depuração
	///////////////////////////////////////////////////////////////////////////////

	/*
	Descrição:
		Toma uma amostra do erro de posição atual.

	Parâmetros:
		-> sample: posição estimada do agente
		-> cheat: posição real proibida e providenciada pelo servidor.
	*/
    int estimar_error_posicional(
    	const Vetor3D sample, 
    	const Vetor3D& cheat,
    	double error_placeholder[]
    );

    void stats_reset(){
    	/* Apenas resetará os valores de erro e de sistema obtidos após os cálculos */

    	counter_tuneo_de_refinamento = 0;

    	for(
    		int i = 0;
    			i < sizeof(errorSum_fineTune_before) / sizeof(errorSum_fineTune_before[0])
    			i++
    	){

    		errorSum_fineTune_before[i]        = 0;
    		errorSum_fineTune_probabilistic[i] = 0;
    		errorSum_fineTune_euclidianDist[i] = 0;
    	}

    	for(
    		int i = 0;
    			i < STATE::ENUMSIZE;
    			i++
    	){

    		contador_de_estados_do_sistema[i] = 0;
    	}
    }
    
    /* Não mudaremos estes nomes pq é muito melhor assim. */
    double errorSum_fineTune_before[7] = {0};        //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_fineTune_euclidianDist[7] = {0}; //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_fineTune_probabilistic[7] = {0}; //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_ball[7] = {0};                   //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    int    counter_tuneo_de_refinamento = 0;
    int    counter_ball = 0;

    // Uma forma de apresentarmos os erros de forma inteligente
    // Vamos manter em inglês por simplicidade
    enum STATE{
    	NONE,            /* Nenhum estado definido */
    	RUNNING,         /* Processo em execução   */
    	MINFAIL, 		 /* Falha mínima ocorreu   */
    	BLIND, 			 /* Não há visão registrada*/
    	FAILzNOgoal, 	 /* Falhas no cálculos de Z*/
    	FAILzLine, 
    	FAILz, 
    	FAILtune,        /* Falha no Ajuste Fino   */
    	FAILguessLine,   /* Falhas na etapa de guess*/
    	FAILguessNone, 	 
    	FAILguessMany, 
    	FAILguessTest, 
    	DONE, 			 /* Concluído              */
    	ENUMSIZE         /* Auxiliar               */
   	};
    STATE estado_do_sistema = NONE;

    void atualizar_estado_do_sistema(enum STATE novo_estado){

    	contador_de_estados_do_sistema[novo_estado]++;
    	estado_do_sistema = novo_estado;

    }
    int contador_de_estados_do_sistema[STATE::ENUMSIZE] = {0};


public:

	///////////////////////////////////////////////////////////////////////////////
	/// Métodos Públicos
	///////////////////////////////////////////////////////////////////////////////

	/*
	Inicializamos matrizes
	*/
	const Matriz &Head_to_Field_Transform = _final_Head_to_Field_Transform; // -> rotação + translação
	const Matriz &Head_to_Field_Rotate    = _final_Head_to_Field_Rotate;    // -> rotação
	const Matriz &Field_to_Head_Transform = _final_Field_to_Head_Transform; // inversa da primeira
	const Matriz &Field_to_Head_Rotate    = _final_Field_to_Head_Rotate;

	/*
	Parte de translação da matriz Head_to_Field_Transform
	*/
	const Vetor3D &posicao_da_cabeca                    =                 _final_translacao;

	/*
	Caso a posição da cabeça e a matriz de transformação estejam em condições
	de serem atualizadas, será setada como True.

	Caso não, isto é, não há elementos vísiveis ou outros motivos, estará como False.
	*/
	const bool &se_esta_pronta_para_atualizacao         = _se_esta_pronta_para_atualizacao;

	/*
	Número de passos de simulação desde útlima atualização.

	Caso se_esta_pronta_para_atualizacao == True, então deverá estar como zero.
	*/
	const unsigned int &passos_desde_ultima_atualizacao = _passos_desde_ultima_atualizacao;

	/*
	- Coordenada Z da cebeça

	Esta variável é normalmente equivalente à posicao_da_cabeca.z, mas pode ser diferente às vezes.
	Há situações em que a rotação e translação não podem ser computados, apesar da coordenada z
	ser obtível.

	Deve ser utilizada em aplicações em que o eixo Z seja uma coordenada independente, como opções
	de detecção para queda do robô ou observações de aprendizado.
	*/
	const float &head_z = _final_z; 

	/*
	Já que head_z pode ser calculada em situações onde autolocalização é impossível,
	esta variável é setada como True quando head_z está em condições de ser atualizada.
	*/
	const bool  &se_head_z_esta_pronta_para_atualizacao = _se_head_z_esta_pronta_para_atualizacao;

	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	/*
	Transformar coordenadas relativas para absolutas usando Head_to_Field_Transform
	*/
	Vetor3D transform_relativo_para_absoluta( const Vetor3D pos_rel ) const {
		
		return Head_to_Field_Transform * pos_rel;
	}

	/*
	Transformar coordenadas absolutas para relativas usando Field_to_Head_Transform
	*/
	Vetor3D transform_absoluta_para_relativo( const Vetor3D pos_abs ) const {

		return Field_to_Head_Transform * pos_abs;
	}

	/*
	- Calcular posição e orientação 3D.
	
	- Caso haja nova informação disponível:

		- Setará "is_updodate" como True.

	- Caso não:

		- Fornecerá a última posição conhecida.
	*/
	void run();

	void reportar_error() const {
		/*
		Função depuradora, providenciando um relatório estatístico de desempenho do 
		algoritmo. Projetado para avaliar e diagnosticar a qualidade dos ajustes.
		*/

		if(
			// Se for primeira vez
			counter_tuneo_de_refinamento == 0
		){

			cout << "LocalizerV2 reportando -> Verifique se o servidor está provendo dados privados (cheat data).\n -> counter_tuneo_de_refinamento = 0. \n";
			return;
		}

		if(
			counter_tuneo_de_refinamento < 2
		){

			return;
		}

		const int &c  = counter_tuneo_de_refinamento;
		const int &cb =                 counter_ball;
		const int c1  =                        c - 1;
		const int cb1 =                       cb - 1;

		/* Cada index dos vetores de erro significam algo específico,
  		   veja no local de declaração para mais informações.
		*/
		const double* ptr = errorSum_fineTune_before;
		float e1_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e1_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		float e1[] = { 
					  ptr[3]/c, 
					  sqrt(e1_2d_var), 
					  ptr[5]/c, 
					  sqrt(e1_3d_var), 
					  ptr[0]/c,
					  ptr[1]/c, ptr[2]/c 
					 };

		ptr = errorSum_fineTune_euclidianDist;
		float e2_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e2_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		float e2[] = { ptr[3]/c, sqrt(e2_2d_var), ptr[5]/c, sqrt(e2_3d_var), ptr[0]/c, ptr[1]/c, ptr[2]/c };

		ptr = errorSum_fineTune_probabilistic;
		float e3_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e3_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		float e3[] = { ptr[3]/c, sqrt(e3_2d_var), ptr[5]/c, sqrt(e3_3d_var), ptr[0]/c, ptr[1]/c, ptr[2]/c };

		ptr = errorSum_ball;
		float e4_2d_var=0, e4_3d_var=0;
		if(cb1 > 0){
			e4_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / cb) / cb1;
			e4_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / cb) / cb1;
		}
		float e4[] = { ptr[3]/cb, sqrt(e4_2d_var), ptr[5]/cb, sqrt(e4_3d_var), ptr[0]/cb, ptr[1]/cb, ptr[2]/cb };

		const int* st = contador_de_estados_do_sistema;
		printf("---------------------------------- LocalizerV2 Report ----------------------------------\n");
		printf("Estágios                    2D-MAE  2D-STD  3D-MAE  3D-STD   x-MBE    y-MBE    z-MBE\n");
		printf("Antes do Refinamento:       %.4f  %.4f  %.4f  %.4f  %7.4f  %7.4f  %7.4f\n",   e1[0],e1[1],e1[2],e1[3],e1[4],e1[5],e1[6]);
		printf("Após EuclidDist. fit:       %.4f  %.4f  %.4f  %.4f  %7.4f  %7.4f  %7.4f\n",   e2[0],e2[1],e2[2],e2[3],e2[4],e2[5],e2[6]);
		printf("Após Probabilistico fit:    %.4f  %.4f  %.4f  %.4f  %7.4f  %7.4f  %7.4f\n",   e3[0],e3[1],e3[2],e3[3],e3[4],e3[5],e3[6]);
		printf("Bola:                       %.4f  %.4f  %.4f  %.4f  %7.4f  %7.4f  %7.4f\n\n", e4[0],e4[1],e4[2],e4[3],e4[4],e4[5],e4[6]);
		printf("* MBE(Mean Bias Error) MAE(Mean Abs Error) STD(Standard Deviation)\n");
		printf("* Note: the cheat positions should be active in server (preferably with >2 decimal places)\n");
		printf("* cheat -> informações 'proibidas'.\n\n");
		printf("------------------LocalizerV2::run calls analysis:\n");
		printf("- Total:               %i \n", st[RUNNING]);
		printf("- Successful:          %i \n", st[DONE]);
		printf("- Blind agent:         %i \n", st[BLIND]);
		printf("- Almost blind:        %i \n", st[MINFAIL] + st[FAILzNOgoal] + st[FAILzLine] + st[FAILz]);
		printf("- Guess location fail: %i \n", st[FAILguessLine] + st[FAILguessNone] + st[FAILguessMany] + st[FAILguessTest]);
		printf("--- Lines too short:   %i \n", st[FAILguessLine]);
		printf("--- No solution:       %i \n", st[FAILguessNone]);
		printf("--- >1 solution:       %i \n", st[FAILguessMany]);
		printf("--- Weak solution:     %i \n", st[FAILguessTest]);
		printf("- Eucl. tune fail:     %i \n", st[FAILtune]); //Euclidian distance tune error above 6cm
		printf("----------------------------------------------------------------------------------------\n");
	}

	Vetor3D obter_velocidade(unsigned int index) const {
		/*
		- Obter velocidade 3D baseado na n-ésima posição 3D, (n_min, n_max) = (1, 9)

		Ao que parece o diferencial vem da seguinte forma:

			p0 -> posição atual
			p1 -> última posição
			p2 -> penúltima pos~ição
			...
			pn -> posição desejada

			Usa-se p0 - pn como dS
		*/

		int ultima_pos = _historico_de_translacao.size() - 1;

		Vetor3D pos_atual = _historico_de_translacao[ 
			( _counter_historico_de_translacao + ultima_pos )         % _historico_de_translacao.size() 
		];
		Vetor3D pos_final = _historico_de_translacao[ 
			( _counter_historico_de_translacao + ultima_pos - index ) % _historico_de_translacao.size() 
		];

		return pos_atual = pos_final;
	}

	/*
	Obtém última posição conhecida da coordenada z da cabeça.

	Esta variável é baseada em head_z, mas deve ser usada como coordenada independente e não 
	em transformações 3D.
	*/
	float obter_ultima_head_z() const {

		return ultima_altura_conhecida;
	}
};


#endif // LOCALIZERV2_H