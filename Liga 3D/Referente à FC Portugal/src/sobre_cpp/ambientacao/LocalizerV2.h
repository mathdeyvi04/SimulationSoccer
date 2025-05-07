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

	/*
	Conforme descrito na documentação acima deste 
	*/
	bool obter_orientacao_do_eixo_z();

	// Espero vir em seguida para poder mudar estes nome.

	void fit_ground_plane();

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

    float last_z = 0.5; 

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

    enum STATE{NONE, RUNNING, MINFAIL, BLIND, FAILzNOgoal, FAILzLine, FAILz, FAILtune, FAILguessLine, FAILguessNone, FAILguessMany, FAILguessTest, DONE, ENUMSIZE};
    STATE state = NONE;

    void stats_change_state(enum STATE s);
    int state_counter[STATE::ENUMSIZE] = {0};


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