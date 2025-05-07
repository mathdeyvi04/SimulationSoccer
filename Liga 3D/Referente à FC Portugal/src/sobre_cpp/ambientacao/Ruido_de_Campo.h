/*
Devido à genialidade, vamos manter exatamente os créditos:

efficient computation of relative probabilities (for the noise model of the RoboCup 3DSSL)
Miguel Abreu (m.abreu@fe.up.pt)
*/

#ifndef RUIDO_DE_CAMPO_H
#define RUIDO_DE_CAMPO_H
#include <cstdio>
#include <cmath>

#define LOG0_5 -0.69314718056f   // Base neperiana
#define LIM_DE_ARG 2

class RuidoCampo{
	/*
	Classe responsável por



	Por que usamos uma classe e não namespace?
		
		Apenas por padrão, todos os demais arquivos possuem classes.
		Este também deve ter.
	*/

private: 

	RuidoCampo() {}; // Disabilitar construtor.
	

	// Documentação providenciada pela equipe
	/**
     * This function returns ln(1-sgn(a)*erf(a)), but sgn(a)*erf(a) = |erf(a)|, because sgn(a) == sgn(erf(a))
     * So, it returns: ln(1-|erf(a)|), which is <=0
     * 
     * NOTE: condition to guarantee high precision: |a|>= 1
     * 
     * how to compute erf(a) ?
     * 		erf(a) = sgn(a)(1-e^erf_aux(a))
     * 
     * how to compute erf(a)+erf(b) ? 
     * 		erf(a)+erf(b) = sgn(a)(1-e^erf_aux(a)) + sgn(b)(1-e^erf_aux(b))
     * 		assuming a<0 and b>0:
     * 		              = e^erf_aux(a) -1 + 1 - e^erf_aux(b)
     * 	                  = e^erf_aux(a) - e^erf_aux(b)
     * 
     * 		example: erf(-7)+erf(7.1)
     * 			if we computed it directly:    
     * 				erf(-7)+erf(7.1) = -0.9999999(...) + 0.9999999(...) = -1+1 = 0 (due to lack of precision, even if using double)
     * 			if we use the proposed method: 
     * 				e^erf_aux(-7) - e^erf_aux(7.1) = -1.007340e-23 - -4.183826e-23 = 3.176486E-23
     * 
     * how to compute ln(erf(a)+erf(b)) ?
     * 		assuming a<0 and b>0:
     * 		ln(erf(a)+erf(b)) = ln( exp(erf_aux(a)) - exp(erf_aux(b)) )
     *                        = ln( exp(erf_aux(a)-k) - exp(erf_aux(b)-k) ) + k
     * 		where k = min(erf_aux(a), erf_aux(b)) 
     * 
     * how to compute ln(erf(a)-erf(b)) ? (the difference is just the assumption)
     *      assuming a*b >= 0
     * 
     *      ln(erf(a)-erf(b)) = ln( sgn(a)(1-e^erf_aux(a)) - sgn(a)(1-e^erf_aux(b)) ),   note that sgn(a)=sgn(b)
     * 
     *      rule: log( exp(a) - exp(b) ) = log( exp(a-k) - exp(b-k) ) + k
     * 
     *      if(a>0) 
     *          ln(erf(a)-erf(b)) = ln( 1 - e^erf_aux(a) - 1 + e^erf_aux(b))
     *                            = ln( exp(erf_aux(b)) - exp(erf_aux(a)) )
     *                            = ln( exp(erf_aux(b)-erf_aux(b)) - exp(erf_aux(a)-erf_aux(b)) ) + erf_aux(b)
     *                            = ln( 1 - exp(erf_aux(a)-erf_aux(b)) ) + erf_aux(b)
     *      if(a<0)
     *          ln(erf(a)-erf(b)) = ln( -1 + e^erf_aux(a) + 1 - e^erf_aux(b))
     *                            = ln( exp(erf_aux(a)) - exp(erf_aux(b)) )
     *                            = ln( exp(erf_aux(a)-erf_aux(a)) - exp(erf_aux(b)-erf_aux(a)) ) + erf_aux(a)
     *                            = ln( 1 - exp(erf_aux(b)-erf_aux(a)) ) + erf_aux(a)
     *   
     */
	static double 
	erf_aux(
		double valor
	){
		/*
		Descrição:
			Função complementar que calcula o valor aproximado da função erro complementar para valores grandes,
			já que a tradicional dada pela biblioteca cmath perde precisão.

			O algoritmo utilizado aqui é integralmente providenciado pelo equipe FC Portugal e trata-se
			de uma aproximação extremamente afiada da função erf para polinômio, basicamente uma Expansão de Taylor.

			A função fma trata-se da função Fused Multiply Add:

			x = fma( x, a, b );

			Ela realiza x + a * b de uma vez, sem fazer duas operações e dois truncamentos, logo é extremamente
			mais rápida e mais precisa. Completamente impressionante.

		Retorno:
			Calculamos - log(erfc(|valor|)).
		*/

 		double result          =           0;
		double intermediario   =           0;
	    double valor_em_modulo = fabs(valor);

	    /*
		Os números extremamente pequenos representam a forma que geraremos o resultado da expansão de Taylor.
		Eles já foram escolhidos de forma a minimizar o erro.
	    */
	    result =        fma (-5.6271698458222802e-018, valor_em_modulo, 4.8565951833159269e-016);
	    intermediario = fma (-1.9912968279795284e-014, valor_em_modulo, 5.1614612430130285e-013);
	    result = 		fma (                  result,   valor * valor,           intermediario);
	    result = 		fma (                  result, valor_em_modulo,-9.4934693735334407e-012);  
	    result = 		fma (                  result, valor_em_modulo, 1.3183034417266867e-010);  
	    result = 		fma (                  result, valor_em_modulo,-1.4354030030124722e-009);  
	    result = 		fma (                  result, valor_em_modulo, 1.2558925114367386e-008);  
	    result = 		fma (                  result, valor_em_modulo,-8.9719702096026844e-008);  
	    result = 		fma (				   result, valor_em_modulo, 5.2832013824236141e-007);  
	    result = 		fma (				   result, valor_em_modulo,-2.5730580226095829e-006);  
	    result = 		fma (				   result, valor_em_modulo, 1.0322052949682532e-005);  
	    result = 		fma (				   result, valor_em_modulo,-3.3555264836704290e-005);  
	    result = 		fma (				   result, valor_em_modulo, 8.4667486930270974e-005);  
	    result = 		fma (				   result, valor_em_modulo,-1.4570926486272249e-004);  
	    result = 		fma (				   result, valor_em_modulo, 7.1877160107951816e-005);  
	    result = 		fma (				   result, valor_em_modulo, 4.9486959714660115e-004);  
	    result = 		fma (				   result, valor_em_modulo,-1.6221099717135142e-003);  
	    result = 		fma (				   result, valor_em_modulo, 1.6425707149019371e-004);  
	    result = 		fma (				   result, valor_em_modulo, 1.9148914196620626e-002);  
	    result = 		fma (				   result, valor_em_modulo,-1.0277918343487556e-001);  
	    result = 		fma (				   result, valor_em_modulo,-6.3661844223699315e-001);  
	    result = 		fma (				   result, valor_em_modulo,-1.2837929411398119e-001);  
	    result = 		fma (				   result, valor_em_modulo,        -valor_em_modulo);

	    return result;
	}


	static double
	log_prob_normal_distribution(
		double media,
		double desvio_padrao,
		double inicio_do_intervalo,
		double final_do_intervalo
	){
		/*
		Descrição:
			Log da probilidade de uma distribuição normal de uma variável aleatória
			em um intervalo determinado.

			Matematicamente: Log(  Prob( inicio < x < final )   )
			
		Parâmetros:
			media -> média da variável aleatória

			... -> Demais autoexplicativos. 
		*/

		const double desvio_padrao_sqrt2 = desvio_padrao * M_SQRT2;

		// Fazemos a transformação para as medidas normais relativas à destribuição padrão.
		double erf1_x = (media - inicio_do_intervalo) / desvio_padrao_sqrt2;
		double erf2_x = (media -  final_do_intervalo) / desvio_padrao_sqrt2;

		/*////////////////////////////////////////////////////////////////////////
		
		Super Explicação Complexa
		
		Usamos esses parâmetros para calcular expressões como: 

		$ erf(erf1_x) - erf(erf2_x)

		Entretanto, para situações que não satisfazem as seguintes condições:

		$ |erf1_x| < LIM  ou  |erf2_x| < LIM
		$  erf1_x * erf2_x < 0

		Os valores obtidos por erf são diferentes dos reais, apesar de próximos.

		Por exemplo:
		
		$ erf(6.5) - erf(6) = 0  (valor real: 2.148e-17)

		Sendo assim, faz-se necessário o uso de uma função auxiliar, a qual
		denominaremos de erf_aux(x) e será implementada em instantes.

		*//////////////////////////////////////////////////////////////////////////

		/*
		Condições que permitem o uso natural da função implementada em cmath ou math.h
		*/
		if(
			(
				fabs(erf1_x) < LIM_DE_ARG
			) || (
				fabs(erf2_x) < LIM_DE_ARG
			) || (
				/* Verificação se possuem sinais diferentes */
				(erf1_x > 0) ^ (erf2_x > 0)
			)
		){

			/*
			Observe que matematicamente estamos fazendo o mesmo que: log(0.5 * (erf(erf1_x) - erf(erf2_x))),
			entretanto, separar os resultados é muito mais rápido.
			*/
			return log(
				erf(erf1_x) - erf(erf2_x)
			) + LOG0_5;
		}

		/*
		Logo, devemos usar a função erf_aux(x).
		Vamos usar a matemática para nos ajudar.

		Sabemos que erf1_x e erf2_x possuem o esmo sinal e estão ambos longe do eixo x = 0.
		Podemos usar o teorema complementar para calcular a probabilidade:

		$ Prob(x < L) = 1 - P(x > L)
		*/

		double erf1 = erf_aux(erf1_x);
		double erf2 = erf_aux(erf2_x);

		if(
			/* Ambas positivos */	
			erf1_x > 0
		){

			return log( 1.0 - exp(erf1 - erf2)) + erf2 + LOG0_5;
		}
		else{

			return log( 1.0 - exp(erf2 - erf1)) + erf1 + LOG0_5;
		}
	}


public: 

	/*////////////////////////////////////////////////////////////////////////////////////
	Cada uma das três funções a seguir utilizam parâmetros específicos para otimização e para
	validade dos resultados. Não altere eles sem saber oq está fazendo.
	*/////////////////////////////////////////////////////////////////////////////////////

	static double
	log_prob_r(
		double distancia,
		double raio_de_ruido
	){
		/*
		Log neperiano da probabilidade de uma distância real d sob um raio de ruído r.
		*/

		double inicio = 100.0 * ( (raio_de_ruido - 0.005) / distancia - 1);
		double final  = 100.0 * ( (raio_de_ruido + 0.005) / distancia - 1);

		return log_prob_normal_distribution(0, 0.0965, inicio, final);
	}

	/*
	A função a seguir possui uma definição diferente da apresentada no original: 

	"... real horizontal angle h, given noisy angle phi."

	Dando a entender que phi é o ângulo horizontal, entretanto já é dito que trata-se do vertical,
	conforme é apresentado no arquivo Vector3f.cpp, função to_polar(), do original ou no arquivo 
	Geometria.h, função para_esferica() do estudo.
	
	Outra possbilidade trata-se apenas de um equívoco de compreensão.
	*/
	static double 
	log_prob_theta(
		double ang_horizontal_theta,
		double ang_horizontal_de_ruido
	){
		/*
		Log neperiano da probabilidade de um ângulo real theta sob um ângulo de ruído.
		*/

		double inicio = ang_horizontal_de_ruido - 0.005 - ang_horizontal_theta;
		double final  = ang_horizontal_de_ruido + 0.005 - ang_horizontal_theta;
	
		return log_prob_normal_distribution(0, 0.1225, inicio, final);
	}


	/*
	Houve o mesmo problema da função descritamente anteriormente.
	*/
	static double 
	log_prob_phi(
		double ang_vertical_phi,
		double ang_vertical_de_ruido
	){
		/*
		Log neperiano de probabilidade de um ângulo real phi sob um ângulo de ruído.
		*/

		double inicio = ang_vertical_de_ruido - 0.005 - ang_vertical_phi;
		double final  = ang_vertical_de_ruido + 0.005 - ang_vertical_phi;
	
		return log_prob_normal_distribution(0, 0.1480, inicio, final);
	}
};

#endif // RUIDO_DE_CAMPO_H