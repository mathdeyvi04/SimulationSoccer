#ifndef LOCALIZERV2_H
#define LOCALIZERV2_H

#include "Singular.h"
#include "RobovizField.h"
#include "Matriz.h"
#include "Ruido_de_Campo.h"

/*
#include <gsl/gsl_multifit.h> //Linear least-squares fitting
#include <gsl/gsl_linalg.h>   //Singular value decomposition
#include <gsl/gsl_multimin.h> //Multidimensional minimization
*/

/* Funções Matemáticas Que Serão Úteis

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
	
	for(int i=0; i<SIZE; i++){
		gsl_vector_set(v, i, content[i]);
	}
	return v;
}
*/


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

	// Espero vir em seguida para poder mudar estes nome.

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

    bool find_xy();
    bool guess_xy();

    bool fine_tune_aux(
    	float &initial_angle,
    	float &initial_x, 
    	float &initial_y, 
    	bool use_probabilities
    );
    bool fine_tune(
    	float initial_angle, 
    	float initial_x, 
    	float initial_y
    );

    
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

			Vetor3D ponto_esf_mais_perto = segmento_mais_perto.segment_ponto_na_reta_mais_perto_cart(mkr_desc.pos_rel_cart).para_esferica();

			total_log_prob += Ruido_de_Campo::log_prob_r(ponto_esf_mais_perto.x, mkr_desc.pos_rel_esf.x);
			total_log_prob += Ruido_de_Campo::log_prob_h(ponto_esf_mais_perto.y, mkr_desc.pos_rel_esf.y);
			total_log_prob += Ruido_de_Campo::log_prob_v(ponto_esf_mais_perto.z, mkr_desc.pos_rel_esf.z);
			
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

    /*
    static double map_error_2d(
    	const gsl_vector *v,
    	void *params
    );
    */

    void commit_everything();

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

    std::array<Vetor3D, 10> position_history;
    unsigned int position_history_ptr = 0;

    float ultima_altura_conhecida = 0.5; 

    unsigned int _passos_desde_ultima_atualizacao =     0;
    bool 		 _se_esta_pronta_para_atualizacao = false; // Leia a  definição dentro da parte pública
    bool  _se_head_z_esta_pronta_para_atualizacao = false;


    ///////////////////////////////////////////////////////////////////////////////
	/// Depuração
	///////////////////////////////////////////////////////////////////////////////

    int stats_sample_position_error(
    	const Vetor3D sample,
    	const Vetor3D& cheat,
    	double error_placeholder[]
    );
    void stats_reset();
    
    double errorSum_fineTune_before[7] = {0};        //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_fineTune_euclidianDist[7] = {0}; //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_fineTune_probabilistic[7] = {0}; //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_ball[7] = {0};                   //[0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    int counter_fineTune = 0;
    int counter_ball = 0;

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

    	contador_do_estado_do_sistema[novo_estado]++;
    	estado_do_sistema = novo_estado;

    }
    int contador_do_estado_do_sistema[STATE::ENUMSIZE] = {0};


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
	Vetor3D transform_relativo_para_absoluta( const Vetor3D pos_rel ) const;

	/*
	Transformar coordenadas absolutas para relativas usando Field_to_Head_Transform
	*/
	Vetor3D transform_absoluta_para_relativo( const Vetor3D pos_abs ) const;

	/*
	- Calcular posição e orientãção 3D.
	
	- Caso haja nova informação disponível:

		- Setará "is_updodate" como True.

	- Caso não:

		- Fornecerá a última posição conhecida.
	*/
	void run();

	/*
	Função depuradora, nos ajudando a identificar o erro e possível solução
	a partir de um print.
	*/
	void reportar_error() const;

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
	Vetor3D obter_velocidade(unsigned int n) const;

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