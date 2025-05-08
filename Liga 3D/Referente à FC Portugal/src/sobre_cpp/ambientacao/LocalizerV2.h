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

// Algumas variáveis estão como seus respectivos nomes em Inglês pq é realmente melhor.

class LocalizerV2 {
	friend class Singular<LocalizerV2>;

private: 

	///////////////////////////////////////////////////////////////////////////////
	/// Métodos Privados Principais
	///////////////////////////////////////////////////////////////////////////////


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

	bool obter_orientacao_do_eixo_z(){
		/*
		Conforme descrito na documentação .md, vamos fazer o algoritmo demonstrado.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		const int numero_de_traves_de_gol = campo_existente.lista_de_goalposts.size();

		if(
			campo_existente.quantidade_de_marcadores_de_chao_nao_colineares >= 3
		){

			// Basta então
			calcular_plano_do_chao_e_altura_do_agente();

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




	}

	// Espero vir em seguida para poder mudar estes nome.

	void find_z(const Vetor3D& Zvec);
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

    /*
	static double map_error_logprob(
    	const gsl_vector *v,
    	void *params
    );
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
	/// Estatística de Depuração
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

		return last_z;
	}
};


#endif // LOCALIZERV2_H