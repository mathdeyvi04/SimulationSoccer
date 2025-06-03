/*
Devido à genialidade, vamos manter exatamente os créditos:

efficient computation of relative probabilities (for the noise model of the RoboCup 3DSSL)
Miguel Abreu (m.abreu@fe.up.pt)

As funções definidas aqui são utilizadas no arquivo LocalizerV2.h, mais especificamente dentro de 
map_error_logprob().
*/
#ifndef RUIDO_DE_CAMPO_H
#define RUIDO_DE_CAMPO_H

#include <math.h>  // Importando assim, teremos acesso global às funções necessárias.

#define LOG05 -0.693147180559945f
#define SQRT2  1.414213562373095f

class Ruido_de_Campo {
	/*
	Descrição:
	    Classe utilitária para o cálculo eficiente de probabilidades relativas,
	    usadas no modelo de ruído do ambiente RoboCup 3D Soccer Simulation League (3DSSL).
	    
	    Fornece métodos para estimar a probabilidade logarítmica de leituras ruidosas
	    em sensores de distância e ângulo, com base em distribuições normais.

	Métodos Públicos:
	    - static double log_prob_r(double d, double r)
	        Retorna o logaritmo da probabilidade de uma distância real 'd'
	        ter gerado uma leitura ruidosa 'r'.

	    - static double log_prob_h(double h, double phi)
	        Retorna o logaritmo da probabilidade de um ângulo horizontal real 'h'
	        ter gerado uma leitura ruidosa 'phi'.

	    - static double log_prob_v(double v, double theta)
	        Retorna o logaritmo da probabilidade de um ângulo vertical real 'v'
	        ter gerado uma leitura ruidosa 'theta'.

	Métodos Privados:
	    - static double log_prob_normal_distribution(double mean, double std, double interval1, double interval2)
	        Calcula o logaritmo da probabilidade de uma variável aleatória normal (com média e desvio padrão dados)
	        cair no intervalo definido por [interval1, interval2].

	    - static double erf_aux(double a)
	        Função auxiliar para cálculos com funções erro (erf), com foco em manter alta precisão
	        numérica ao operar com diferenças entre funções erro em valores extremos.

	Observações:
	    - A classe utiliza estratégias logarítmicas para evitar problemas numéricos com valores de probabilidade muito pequenos.
	    - Recomendada para uso interno em sistemas de percepção ruidosa em ambientes simulados.
	*/

private:

	Ruido_de_Campo() {};

	/*
	Esta função retorna ln(1 - sgn(a) * erf(a)), mas sgn(a) * erf(a) = |erf(a)|, 
	porque sgn(a) == sgn(erf(a)). Portanto, ela retorna: ln(1 - |erf(a)|), o que é sempre ≤ 0.

	OBSERVAÇÃO: condição para garantir alta precisão: |a| ≥ 1
	
	Como calcular erf(a)?
	    erf(a) = sgn(a)(1 - e^erf_aux(a))

	Como calcular erf(a) + erf(b)?
	    erf(a) + erf(b) = sgn(a)(1 - e^erf_aux(a)) + sgn(b)(1 - e^erf_aux(b))
	    assumindo a < 0 e b > 0:
	        = e^erf_aux(a) - 1 + 1 - e^erf_aux(b)
	        = e^erf_aux(a) - e^erf_aux(b)

	Exemplo: erf(-7) + erf(7.1)
	    Se calculássemos diretamente:
	        erf(-7) + erf(7.1) = -0.9999999(...) + 0.9999999(...) = -1 + 1 = 0 
	        (devido à falta de precisão, mesmo usando double)
	    Se usarmos o método proposto:
	        e^erf_aux(-7) - e^erf_aux(7.1) = -1.007340e-23 - -4.183826e-23 = 3.176486E-23

	Como calcular ln(erf(a) + erf(b)), onde a e b tem sinais trocados?
	    assumindo a < 0 e b > 0:
	    ln(erf(a) + erf(b)) = ln( exp(erf_aux(a)) - exp(erf_aux(b)) )
	                        = ln( exp(erf_aux(a) - k) - exp(erf_aux(b) - k) ) + k
	    onde k = min(erf_aux(a), erf_aux(b))

	Como calcular ln(erf(a) - erf(b)), onde a e b possuem mesmo sinal?
	    assumindo a*b ≥ 0

	    ln(erf(a) - erf(b)) = ln( sgn(a)(1 - e^erf_aux(a)) - sgn(a)(1 - e^erf_aux(b)) ), 
	    observando que sgn(a) = sgn(b)

	    regra: log( exp(a) - exp(b) ) = log( exp(a - k) - exp(b - k) ) + k

	    Se (a > 0):
	        ln(erf(a) - erf(b)) = ln( 1 - e^erf_aux(a) - 1 + e^erf_aux(b) )
	                            = ln( exp(erf_aux(b)) - exp(erf_aux(a)) )
	                            = ln( exp(erf_aux(b) - erf_aux(b)) - exp(erf_aux(a) - erf_aux(b)) ) + erf_aux(b)
	                            = ln( 1 - exp(erf_aux(a) - erf_aux(b)) ) + erf_aux(b)

	    Se (a < 0):
	        ln(erf(a) - erf(b)) = ln( -1 + e^erf_aux(a) + 1 - e^erf_aux(b) )
	                            = ln( exp(erf_aux(a)) - exp(erf_aux(b)) )
	                            = ln( exp(erf_aux(a) - erf_aux(a)) - exp(erf_aux(b) - erf_aux(a)) ) + erf_aux(a)
	                            = ln( 1 - exp(erf_aux(b) - erf_aux(a)) ) + erf_aux(a)
	*/
	static double 
	erf_aux(
		// OBSERVAÇÃO: condição para garantir alta precisão: |a| ≥ 1
		double valor
	){	
		/*
		Descrição:
		    Avalia uma aproximação polinomial auxiliar de alta precisão usada no cálculo
		    da função erro (erf), empregando operações do tipo fused multiply-add (fma)
		    para melhorar a precisão numérica e a estabilidade da avaliação do polinômio.

		    O cálculo é baseado em coeficientes ajustados que são aplicados de forma sequencial,
		    reduzindo os erros de arredondamento típicos de somas e multiplicações separadas.

		Retorno:
		    - Valor real resultante da aproximação polinomial auxiliar associada à função erro.
		*/

	    double resultado, termo_absoluto, quadrado;

	    termo_absoluto = fabs(valor);
	    quadrado = valor * valor;

	    resultado = fma(-5.6271698458222802e-018, termo_absoluto, 4.8565951833159269e-016);
	    double termo_intermediario = fma(-1.9912968279795284e-014, termo_absoluto, 5.1614612430130285e-013);
	    resultado = fma(resultado, quadrado      , termo_intermediario     );
	    resultado = fma(resultado, termo_absoluto, -9.4934693735334407e-012);  
	    resultado = fma(resultado, termo_absoluto,  1.3183034417266867e-010);  
	    resultado = fma(resultado, termo_absoluto, -1.4354030030124722e-009);  
	    resultado = fma(resultado, termo_absoluto,  1.2558925114367386e-008);  
	    resultado = fma(resultado, termo_absoluto, -8.9719702096026844e-008);  
	    resultado = fma(resultado, termo_absoluto,  5.2832013824236141e-007);  
	    resultado = fma(resultado, termo_absoluto, -2.5730580226095829e-006);  
	    resultado = fma(resultado, termo_absoluto,  1.0322052949682532e-005);  
	    resultado = fma(resultado, termo_absoluto, -3.3555264836704290e-005);  
	    resultado = fma(resultado, termo_absoluto,  8.4667486930270974e-005);  
	    resultado = fma(resultado, termo_absoluto, -1.4570926486272249e-004);  
	    resultado = fma(resultado, termo_absoluto,  7.1877160107951816e-005);  
	    resultado = fma(resultado, termo_absoluto,  4.9486959714660115e-004);  
	    resultado = fma(resultado, termo_absoluto, -1.6221099717135142e-003);  
	    resultado = fma(resultado, termo_absoluto,  1.6425707149019371e-004);  
	    resultado = fma(resultado, termo_absoluto,  1.9148914196620626e-002);  
	    resultado = fma(resultado, termo_absoluto, -1.0277918343487556e-001);  
	    resultado = fma(resultado, termo_absoluto, -6.3661844223699315e-001);  
	    resultado = fma(resultado, termo_absoluto, -1.2837929411398119e-001);  
	    resultado = fma(resultado, termo_absoluto, -termo_absoluto         );

	    return resultado;
	}


	static double
	log_prob_normal_distribution(
		double media,
		double desvio_padrao,
		double inicio_do_intervalo,
		double final_do_intervalo,
		int n = 0
	){
		/*
		Descrição:
		    Calcula o logaritmo da probabilidade de uma variável aleatória X,
		    com distribuição normal (Gaussiana) de média e desvio padrão conhecidos,
		    estar contida no intervalo aberto definido por [inicio..., final...].

		    Isto é, retorna:
		        log( P(inicio < X < final) )

		Parâmetros:
		    Autoexplicativos.

		Retorno:
		    - O valor do logaritmo natural da probabilidade de X pertencer ao intervalo especificado.
			- Retorna um valor ≤ 0.
		*/

		double erf1_x = ( media - inicio_do_intervalo ) / ( desvio_padrao * SQRT2 );
		double erf2_x = ( media - final_do_intervalo  ) / ( desvio_padrao * SQRT2 );

		/*
		Calcular erf(erf_x1) - erf(erf_x2) é equivalente a erf(erf_x1) + erf(-erf_x2).
		Intuitivamente, a primeira forma parece mais natural.

		Computacionalmente, essa forma é mais precisa nas seguintes condições:
			- erf_x1 * erf_x2 <= 0
			- |erf_x1| < 3  ou  |erf_x2| < 3  (o número '3' é apenas uma estimativa, não é crítico)

		Problemas conhecidos:
			- erf(6.5) - erf(6.0) = 1 - 1 = 0  (valor real: 2.148e-17)
			- Mesmo usando funções com precisão de 128 bits, o problema apenas é mitigado — e é bastante comum.

		Para esses casos, é utilizada a função erf_aux(x), embora ela não seja precisa para |x| < 1.
			- erf_aux(x) permite calcular erf(6.5) - erf(6.0) com 7 dígitos de precisão
			- erf_aux(x) permite calcular erf(8.5) - erf(8.0) com 3 dígitos de precisão
			- erf(12.5) - erf(12) = 0 (o que não é desejável em comparações probabilísticas)
			- erf_aux(x) permite calcular log(erf(6.5) - erf(6.0)) com 8 dígitos de precisão
			- erf_aux(x) permite calcular log(erf(8.5) - erf(8.0)) com 5 dígitos de precisão
			- log(erf(12.5) - erf(12)) = -4647 (real: -147) — não é preciso, mas suficiente para comparações

		O algoritmo completo abaixo, que utiliza erf_aux(x), é quase tão rápido quanto o que usa erf() da math.h (aumento de apenas ~30% no tempo de execução).
		*/


		if(
			// Condições Anteriormente Citadas
			fabs( erf1_x ) < 2 || fabs( erf2_x ) < 2 || ( ( erf1_x > 0 ) ^ ( erf2_x > 0 ) )
		){

			// same but faster than log( 0.5 * (erf(erf1_x) - erf(erf2_x)) )
			return log( erf( erf1_x ) - erf( erf2_x ) ) + LOG05;
		}

		// Como as condições não foram atendidas, vamos a:
		double erf1 = erf_aux( erf1_x );
		double erf2 = erf_aux( erf2_x );

		// A operação de soma é diferente devido à erf_aux, veja a documentação.
		return ( erf1_x > 0 ) ? log( 1 - exp( erf1 - erf2 ) ) + erf2 + LOG05 : log( 1 - exp(erf2 - erf1 ) ) + erf1 + LOG05;
	}


public:
	static double 
	log_prob_r( double d, double r     ){

	    double c1 = 100.0 * ( (r-0.005) / d - 1);
	    double c2 = 100.0 * ( (r+0.005) / d - 1);

	    return log_prob_normal_distribution(0, 0.0965, c1, c2);
	}

	static double 
	log_prob_h( double h, double phi   ){

	    double c1 = phi - 0.005 - h;
	    double c2 = phi + 0.005 - h;

	    return log_prob_normal_distribution(0, 0.1225, c1, c2);
	}

	static double 
	log_prob_v( double v, double theta ){

	    double c1 = theta - 0.005 - v;
	    double c2 = theta + 0.005 - v;

	    return log_prob_normal_distribution(0, 0.1480, c1, c2);
	}
};

#endif // RUIDO_DE_CAMPO_H
