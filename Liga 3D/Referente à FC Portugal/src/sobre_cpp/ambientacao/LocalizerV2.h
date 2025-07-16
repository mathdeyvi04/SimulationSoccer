/*
Créditos ao Miguel Abreu!

Devido ao não uso, houve remoção das seguintes funcionalidades:

Vetor3D relativeToAbsoluteCoordinates(const Vetor3D relativeCoordinates) const;
Vetor3D absoluteToRelativeCoordinates(const Vetor3D absoluteCoordinates) const;
Vetor3D get_velocity(unsigned int n) const;
void add_gsl_regression_sample(gsl_matrix* m, gsl_vector* v, int sample_no, const Vetor3D& relativeCoord, double absoluteCoord, double translCoeffMult=1);

Atributos de Localizerv2:

std::array<Vetor3D, 10> position_history;
unsigned int position_history_ptr = 0;
float get_last_head_z() const;
headTofieldRotate e sua inversa
*/

#ifndef LOCALIZERV2_H
#define LOCALIZERV2_H

#include "Singular.h"
#include "Ruido_de_Campo.h"
#include "RobovizField.h"
#include <cstdio>

using namespace std;

/*
Fornece funções para ajuste linear de mínimos quadrados. É ideal para ajustar modelos lineares a
dados experimentais, minimizando a soma dos quadrados dos resíduos entre os pontos observados
e os valores previstos.
*/
#include <gsl/gsl_multifit.h>  

/*
Implementa algoritmos de álgebra linear, como decomposição em valores singulares (SVD),
fatorações LU e QR, resolução de sistemas lineares e cálculo de determinantes. Essencial para
operações robustas com matrizes.
*/
#include <gsl/gsl_linalg.h>   

/*
Disponibiliza algoritmos para minimização de funções multivariadas, com ou sem derivadas.
É útil para encontrar o ponto de mínimo (ótimo) de funções complexas em espaços de múltiplas variáveis.
*/
#include <gsl/gsl_multimin.h> 


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Nunca se sabe o dia de amanha, mas aparentemente isso não é usado em canto algum.
// void add_gsl_regression_sample(
// 	gsl_matrix*     matriz_de_entrada,
// 	gsl_vector*     vetor_resultado,
// 	int             indice_amostra,
// 	const Vetor3D& coordenada_relativa,
// 	double          coordenada_absoluta,
// 	double          multiplicador_translacao = 1
// ){
// 	/*
// 	Descrição:
// 	    Insere uma amostra em uma matriz e vetor utilizados para regressão linear com a biblioteca GSL.
// 	    Cada amostra associa um ponto 3D (coordenadas relativas) a um valor escalar (coordenada absoluta),
// 	    permitindo ajustar modelos lineares por mínimos quadrados. Um coeficiente extra pode ser usado
// 	    para representar deslocamentos/termos de viés.

// 	Parâmetros:
// 	- matriz_de_entrada:
// 	    ponteiro para a matriz GSL (designada para armazenar variáveis independentes da regressão).

// 	- vetor_resultado:
// 	    ponteiro para o vetor GSL (designado para armazenar os valores observados ou dependentes).

// 	- indice_amostra:
// 	    índice da linha onde a nova amostra será inserida (linha da matriz e posição do vetor).

// 	- coordenada_relativa:
// 	    vetor 3D contendo os valores (x, y, z) relativos que representam a entrada da regressão.

// 	- coordenada_absoluta:
// 	    valor escalar correspondente à variável dependente para esta amostra.

// 	- multiplicador_translacao:
// 	    coeficiente adicional (opcional, padrão = 1), útil para modelar deslocamentos constantes.

// 	Retorno:
// 	    Nenhum valor de retorno. 
// 	    A matriz e o vetor são modificados diretamente.
// 	*/

//     gsl_matrix_set(matriz_de_entrada, indice_amostra, 0, coordenada_relativa.x);
//     gsl_matrix_set(matriz_de_entrada, indice_amostra, 1, coordenada_relativa.y);
//     gsl_matrix_set(matriz_de_entrada, indice_amostra, 2, coordenada_relativa.z); 
//     gsl_matrix_set(matriz_de_entrada, indice_amostra, 3, multiplicador_translacao); 

//     gsl_vector_set(vetor_resultado, indice_amostra, coordenada_absoluta);
// }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalizerV2 {
	/*
	Descrição:
	    Classe responsável por realizar a localização tridimensional do agente em campo, com base
	    em dados visuais e modelagem geométrica. Ela estima a posição e orientação da cabeça do robô
	    utilizando transformações homogêneas (matrizes 4x4), extraídas a partir de observações do
	    ambiente (linhas, marcadores e contatos com o solo).

	    A classe também realiza o rastreamento temporal da posição da cabeça para permitir a
	    estimativa de velocidade, além de fornecer a altura da cabeça separadamente — útil em casos
	    onde a localização completa falha, mas a posição vertical ainda é confiável.

	Principais Métodos:
	    - void run():
	        Atualiza a posição e orientação da cabeça com base nos dados visuais disponíveis.
	        Se não houver dados novos, mantém a última estimativa.

	    - void print_report() const:
	        Imprime um relatório com os erros médios e análise da solução da localização.

	    - Vetor3D relativeToAbsoluteCoordinates(const Vetor3D):
	        Converte um vetor do sistema relativo (cabeça) para o sistema absoluto (campo).

	    - Vetor3D absoluteToRelativeCoordinates(const Vetor3D):
	        Converte um vetor do sistema absoluto (campo) para o sistema relativo (cabeça).

	    - float get_last_head_z() const:
	        Retorna o último valor conhecido da coordenada Z da cabeça, mesmo quando não foi possível
	        realizar a localização completa.

	Observações Plausíveis:
	    - O atributo `is_uptodate` informa se as estimativas atuais são confiáveis. Quando falso,
	      significa que não houve dados visuais suficientes ou válidos para uma nova estimativa.

	    - A classe mantém um histórico das últimas 10 posições para cálculo de velocidade.
	*/
	friend class Singular<LocalizerV2>;

private:

	LocalizerV2() {}

	template<std::size_t TAMANHO> static gsl_vector* 
	criar_vetor_gsl(
		const array<double, TAMANHO> &conteudo
	){
		/*
		Descrição:
		    Cria e inicializa um vetor GSL (`gsl_vector`) a partir de um `array` de valores do tipo `double`.
		    A função é útil para converter dados estáticos ou computados previamente em estruturas compatíveis com
		    algoritmos da biblioteca GSL.

		Parâmetros:
		- conteudo:
		    array padrão da STL contendo os valores a serem copiados para o vetor GSL.
		    O tamanho do vetor é definido pelo tamanho do array passado como argumento.

		Retorno:
		    Retorna um ponteiro para um vetor GSL alocado dinamicamente (`gsl_vector*`) contendo os dados copiados.
		    É responsabilidade do usuário liberar a memória posteriormente com `gsl_vector_free()`.
		*/

	    gsl_vector* vetor_gsl = gsl_vector_alloc(TAMANHO);
	    
	    for(
	    	size_t i = 0; 
	    		i < TAMANHO; 
	    		i++
	    ){
	        gsl_vector_set(vetor_gsl, i, conteudo[i]);
	    }

	    return vetor_gsl;
	}

	static Vetor2D
	obter_vetor_unitario_perpendicular_ao_chao( const Vetor3D& vec ) {
		/*
		  Descrição:
		  	Calcula e retorna um vetor unitário no plano Z=0 (XY), perpendicular à projeção do vetor fornecido.

		  	Dada a projeção do vetor `vec` no plano XY, retorna um vetor 2D unitário perpendicular a essa projeção,
		 	ou seja, o vetor normalizado resultante do produto vetorial (0,0,1) x vec.
		  	Se a projeção for nula (vec paralelo ao eixo Z), retorna (1,0) por padrão.

		  Parâmetros:
		    - vec: Vetor 3D de entrada.

		  Retorno:
		    - Vetor 2D unitário perpendicular à projeção de `vec` no plano Z=0.
		*/

	    float gx = 1, gy = 0; // vetor padrão (caso projeção seja nula)
	    const float aux = sqrtf( vec.x*vec.x + vec.y*vec.y ); // comprimento da projeção no plano XY

	    if(aux > 0) {
	        gx = -vec.y / aux;
	        gy =  vec.x / aux;
	    }

	    return Vetor2D(gx, gy);
	}

	static Vetor3D 
	rotacao_rapida_em_torno_eixo_solo( Vetor3D vetor, Vetor3D normal_solo ) {
	    /*
		  Descrição:
		  Rotaciona um vetor 3D em torno de um eixo no plano do solo definido como u = (0,0,1) x normal_solo / |normal_solo|.  
		  A direção da rotação é do vetor normal_solo para (0,0,1).  
		  Para inverter o sentido da rotação, inverta os sinais de normal_solo.x e normal_solo.y.

		  Parâmetros:
		    - vetor: vetor a ser rotacionado.
		    - normal_solo: vetor normal unitário do plano do solo rotacionado.

		  Retorno:
		    - Vetor 3D resultante da rotação.
		*/

	    // Obtém o eixo de rotação no plano do solo: u = (0,0,1) x normal_solo / |normal_solo|
	    Vetor2D eixo_rotacao = obter_vetor_unitario_perpendicular_ao_chao(normal_solo);
	    
	    // Ângulo entre o vetor normal do plano original e o vetor normal do plano rotacionado:
	    // cos(ângulo) = (normal_original . normal_rotacionado) / (|normal_original| * |normal_rotacionado|)
	    //             = ((0,0,1) . (nx, ny, nz)) / (1 * 1) = nz
	    float& cosseno_angulo = normal_solo.z;

	    // Correção: ocorre raramente, causa ainda não identificada, acredito que erro acumulado
	    if( cosseno_angulo > 1 ) { cosseno_angulo = 1; } 

	    // inverte o seno para inverter a direção (conforme descrição do método, veja .md)
	    float seno_angulo = - sqrtf(1.0f - cosseno_angulo * cosseno_angulo);

	    // Auxiliares para o cálculo da matriz de rotação
	    const float um_menos_cosseno = 1.0f - cosseno_angulo;
		const float eixo_x2_um_menos_cosseno = eixo_rotacao.x * eixo_rotacao.x * um_menos_cosseno;
		const float eixo_y2_um_menos_cosseno = eixo_rotacao.y * eixo_rotacao.y * um_menos_cosseno;
		const float eixo_xy_um_menos_cosseno = eixo_rotacao.x * eixo_rotacao.y * um_menos_cosseno;
		const float eixo_x_seno              = eixo_rotacao.x * seno_angulo;
		const float eixo_y_seno              = eixo_rotacao.y * seno_angulo;

		// Cálculo dos componentes do vetor rotacionado
		float x = (cosseno_angulo + eixo_x2_um_menos_cosseno) * vetor.x
		        + eixo_xy_um_menos_cosseno * vetor.y
		        + eixo_y_seno * vetor.z;

		float y = eixo_xy_um_menos_cosseno * vetor.x
		        + (cosseno_angulo + eixo_y2_um_menos_cosseno) * vetor.y
		        - eixo_x_seno * vetor.z;

		float z = -eixo_y_seno * vetor.x
		        +  eixo_x_seno * vetor.y
		        +  cosseno_angulo * vetor.z;

	    return Vetor3D(x, y, z);
	}

	static void 
	calcular_eixos_XY_a_partir_de_Z(
		const Vetor3D& normal_solo, 
		float angulo_em_torno_de_z, 
		Vetor3D& vetor_x, 
		Vetor3D& vetor_y
	){
		/*
		Descrição:
		    Calcula os vetores de orientação dos eixos X e Y a partir de um vetor de orientação do eixo Z
		    e de um ângulo de rotação em torno desse eixo. O vetor Z define a direção do "cima" (por exemplo,
		    a orientação de um agente), e o ângulo define a rotação do eixo X em torno dele. O vetor Y é
		    automaticamente calculado como o produto vetorial entre Z e X para manter um sistema ortonormal.

		Parâmetros:
		    - vetor_z:
		        Vetor unitário de entrada representando a direção do eixo Z.

		    - angulo_em_torno_de_z:
		        Ângulo (em radianos) de rotação do eixo X em torno do vetor Z. Define a orientação horizontal
		        em relação ao plano perpendicular a Z.

		    - vetor_x:
		        Vetor de saída representando a direção do eixo X após a rotação. Calculado com base no vetor Z
		        e no ângulo fornecido.

		    - vetor_y:
		        Vetor de saída representando a direção do eixo Y. Calculado automaticamente como o produto vetorial
		        entre vetor_z e vetor_x para manter a ortogonalidade.

		Retorno:
		    Os vetores vetor_x e vetor_y são modificados por referência.
		*/

	    Vetor2D vetor_base_chao = obter_vetor_unitario_perpendicular_ao_chao(normal_solo);

	    const float& cosseno_z = normal_solo.z; 
	    const float  seno_z    = sqrtf(1 - cosseno_z * cosseno_z);
	    const float  produto_xy_base = vetor_base_chao.x * vetor_base_chao.y;
	    const float  angulo_x  = -angulo_em_torno_de_z; // Ângulo do eixo X ao redor de Z
	    const float  cosseno_x = cosf(angulo_x);
	    const float  seno_x    = sinf(angulo_x);
	    const float  inverso_cosseno_z = 1 - cosseno_z;

	    vetor_x.x = cosseno_z * cosseno_x + inverso_cosseno_z * (vetor_base_chao.x * vetor_base_chao.x * cosseno_x + produto_xy_base * seno_x);
	    vetor_x.y = cosseno_z * seno_x    + inverso_cosseno_z * (vetor_base_chao.y * vetor_base_chao.y * seno_x    + produto_xy_base * cosseno_x);
	    vetor_x.z = seno_z * (vetor_base_chao.x * seno_x - vetor_base_chao.y * cosseno_x);

	    // Define o vetor Y usando o produto vetorial
	    vetor_y = normal_solo.CrossProduct(vetor_x); 
	}

	Matriz4D _Head_to_Field_Prelim = Matriz4D();

	void 
	resetar_matriz_preliminar(){
    	/*
		Descrição:
			Vamos resetar a matriz de transformação para uma versão preliminar.
    	
    	Parâmetros:
    		None

    	Retorno:
    		Nada além de setar os elementos da matriz preliminar.
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

	// MATRIZ _final - o usuário tem acesso a uma referência pública constante das variáveis abaixo
	Matriz4D _final_Head_to_Field_Transform; // Rotação + Translação
	//Matriz4D _final_Head_to_Field_Rotate;    // Rotação

	Matriz4D _final_Field_to_Head_Transform; // Rotação + Translação
	//Matriz4D _final_Field_to_Head_Rotate;    // Rotação

	// Parte referente à translação de _final_Head_to_Field_Transform
	Vetor3D _final_vetor_de_translacao; 
	float   _final_z;

	float _last_z = 0.5; 
    bool  _is_uptodate = false;
    bool  _is_head_z_uptodate = false;
    unsigned int _steps_since_last_update = 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static double 
	map_error_logprob(
		const gsl_vector *v,
		void *params
	){
		/*
		  Descrição:
			  Calcula o erro de mapeamento utilizando probabilidades de distâncias.
			  Retorna o logaritmo negativo da "probabilidade normalizada" = (p1*p2*p3*...*pn)^(1/n). 

		  Parâmetros:
		    - v: vetor de otimização do GSL.
		    - params: parâmetros auxiliares (pode fornecer o ângulo fixo).

		  Retorno:
		    - Logaritmo negativo da probabilidade normalizada, utilizado como função de perda para otimização.
		*/

	    float angle;
	    RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

	    // Obtém o ângulo do vetor de otimização, ou dos parâmetros (como constante)
	    if(
	    	v->size == 3
	    ){

	        angle = gsl_vector_get(v, 2);
	    }else{

	        angle = *(float *)params;
	    }

	    Matriz4D& transfMat = Singular<LocalizerV2>::obter_instancia()._Head_to_Field_Prelim;
	    Vetor3D Zvec(transfMat.obter(2,0), transfMat.obter(2,1), transfMat.obter(2,2));
	    
	    Vetor3D Xvec, Yvec;
	    calcular_eixos_XY_a_partir_de_Z(Zvec, angle, Xvec, Yvec );

	    // Estes são os coeficientes de transformação que estão sendo otimizados
	    transfMat.setar(0,0,Xvec.x);
	    transfMat.setar(0,1,Xvec.y);
	    transfMat.setar(0,2,Xvec.z);
	    transfMat.setar(0,3,gsl_vector_get(v, 0));
	    transfMat.setar(1,0,Yvec.x);
	    transfMat.setar(1,1,Yvec.y);
	    transfMat.setar(1,2,Yvec.z);
	    transfMat.setar(1,3,gsl_vector_get(v, 1));

	    Matriz4D inverseTransMat = transfMat.criar_transformacao_inversa();

	    double total_logprob = 0;
	    int total_err_cnt =0;
	    
	    // Adiciona a log-probabilidade dos marcadores desconhecidos (com segmento de campo correspondente conhecido)
	    for(
	    	const auto& mkr : campo_existente.list_unknown_markers
	    ){

	        // Sabemos o segmento de campo mais próximo, então podemos trazê-lo para o referencial do agente
	        Vetor3D rel_field_s_start = inverseTransMat * mkr.segm->pt[0]->obter_vetor();
	        Vetor3D rel_field_s_end   = inverseTransMat * mkr.segm->pt[1]->obter_vetor();

	        Linha6D rel_field_s(rel_field_s_start, rel_field_s_end, mkr.segm->comprimento);

	        Vetor3D closest_polar_pt = rel_field_s.ponto_mais_proximo_na_reta_para_ponto_cartesiano(mkr.pos_rel_cart).to_esfe();

	        total_logprob += Ruido_de_Campo::log_prob_r(closest_polar_pt.x, mkr.pos_rel_esf.x);
	        total_logprob += Ruido_de_Campo::log_prob_h(closest_polar_pt.y, mkr.pos_rel_esf.y);
	        total_logprob += Ruido_de_Campo::log_prob_v(closest_polar_pt.z, mkr.pos_rel_esf.z);
	        total_err_cnt++;
	    }

	    // Adiciona a log-probabilidade dos marcadores conhecidos
	    for(
	    	const auto& mkr : campo_existente.list_known_markers
	    ){

	        // Traz o marcador para o referencial do agente
	        Vetor3D rel_k = (inverseTransMat * mkr.pos_abs.obter_vetor()).to_esfe();

	        total_logprob += Ruido_de_Campo::log_prob_r(rel_k.x, mkr.pos_rel_esf.x);
	        total_logprob += Ruido_de_Campo::log_prob_h(rel_k.y, mkr.pos_rel_esf.y);
	        total_logprob += Ruido_de_Campo::log_prob_v(rel_k.z, mkr.pos_rel_esf.z);
	        total_err_cnt++;

	    }

	    // retorna o log da "probabilidade normalizada" = (p1*p2*p3*...*pn)^(1/n)
	    /*
		Como a probabilidade de um evento desejado é maximizada, o logaritmo dessa probabilidade normalmente seria maximizado. 
		No entanto, como o otimizador busca o menor valor possível, multiplicamos por -1, convertendo o problema de maximização
		da probabilidade em um problema de minimização do logaritmo negativo da probabilidade. 
		Assim, o otimizador encontra a solução mais provável ao minimizar essa função de perda.
	    */
	    double logNormProb = - total_logprob / total_err_cnt; 

	    return (!gsl_finite(logNormProb)) ? 1e6 : logNormProb;
	}
	
 	static double 
 	map_error_euclidian_distance(
		const gsl_vector *v,
		void *params
	){
 		/*
		  Descrição:
		  Calcula o erro de mapeamento utilizando distâncias euclidianas em 2D.
		  Para cada segmento de linha e marcador observado, determina o desvio em relação às linhas e marcadores do campo de referência,
		  levando em consideração tolerâncias angulares específicas para linhas grandes e pequenas.
		  O resultado é a média dos erros calculados para todos os elementos avaliados.

		  Parâmetros:
		    - v: vetor de otimização do GSL.
		    - params: parâmetros auxiliares (pode fornecer o ângulo fixo).

		  Retorno:
		    - Média das distâncias (erro) calculadas.
		*/
	    RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

	    // Obtém o ângulo do vetor de otimização, ou dos parâmetros (como constante)
	    float angle = 0;
		if(
			v->size == 3
		){

			angle = gsl_vector_get(v, 2);
		}
		else{

			angle = *(float *) params;
		}

	    Matriz4D& transfMat = Singular<LocalizerV2>::obter_instancia()._Head_to_Field_Prelim;
	    Vetor3D Zvec(transfMat.obter(2,0), transfMat.obter(2,1), transfMat.obter(2,2));
	    
	    Vetor3D Xvec, Yvec;
	    calcular_eixos_XY_a_partir_de_Z( Zvec, angle, Xvec, Yvec );

	    // Estes são os coeficientes de transformação que estão sendo otimizados
	    transfMat.setar(0,0,Xvec.x);
	    transfMat.setar(0,1,Xvec.y);
	    transfMat.setar(0,2,Xvec.z);
	    transfMat.setar(0,3,gsl_vector_get(v, 0));
	    transfMat.setar(1,0,Yvec.x);
	    transfMat.setar(1,1,Yvec.y);
	    transfMat.setar(1,2,Yvec.z);
	    transfMat.setar(1,3,gsl_vector_get(v, 1));

	    float total_err = 0;
	    int total_err_cnt =0;
	    for(
	    	const Linha6D& linha_qualquer : campo_existente.list_segments
	    ){
	 
	        // Calcula as coordenadas absolutas da linha de acordo com a transformação atual
	        Vetor3D ponto_inicial_da_linha_abs = transfMat * linha_qualquer.ponto_inicial_cartesiano; 
	        Vetor3D ponto_final_da_linha_abs   = transfMat * linha_qualquer.ponto_final_cartesiano; 

	        // Calcula o ângulo da linha e estabelece uma tolerância
	        float l_angle = 0;

	        // tolerância padrão (não faz sentido ser maior que pi/2, mas não há problema se for)
	        float l_angle_tolerance = 10; 

	        if(
	        	linha_qualquer.comprimento > 0.8
	        ){

	            // Caso mais fácil: encontra o ângulo e aplica tolerância pequena (permite pequenas rotações visuais)
	            l_angle = atan2f( 
	            				ponto_final_da_linha_abs.y - ponto_inicial_da_linha_abs.y,
	            				ponto_final_da_linha_abs.x - ponto_inicial_da_linha_abs.x
	            			 	);
	        	// é uma linha, não um vetor, então ângulos positivos bastam
	            if(l_angle < 0) { l_angle += 3.14159265f; } 
	            
	            // 20 graus
	            l_angle_tolerance = 0.35f; 

	        } else if(
	        	campo_existente.list_segments.size() <= 3
	        ) {
	            // Chega um momento em que o custo/benefício não compensa. Se há muitas linhas (>3),
	            // as pequenas não são tão decisivas para o erro de mapeamento. Caso contrário, prossegue:

	            // Se a linha pequena está tocando uma grande, elas têm orientações diferentes (característica das linhas do campo) 

	            for(
	            	const Linha6D& lbig : campo_existente.list_segments
	            ){
	                if(lbig.comprimento < 2 || &lbig == &linha_qualquer ) { continue; }// verifica se a linha é grande e diferente da atual

	                if( 
	                	lbig.distancia_entre_segmentos(linha_qualquer) < 0.5 
	                ){
	                    // isso só geraria falsos positivos com a linha central e linhas dos anéis (se houver muito erro de visão)
	                    // mas mesmo assim, suas orientações seriam bem diferentes, então o método se mantém

	                    // pega o ângulo perpendicular à linha grande (que é o ângulo da pequena, ou pelo menos próximo)

	                    // pega o ângulo da linha grande
	                    Vetor3D lbigs = transfMat * lbig.ponto_inicial_cartesiano; 
	                    Vetor3D lbige = transfMat * lbig.ponto_final_cartesiano; 
	                    l_angle = atan2f(
	                    				lbige.y - lbigs.y,
	                    				lbige.x - lbigs.x
	                    				);

	                    //  adiciona 90 graus mantendo o ângulo entre 0-180 graus (mesma lógica do caso linha_qualquer.comprimento > 0.8)
	                    if     (l_angle < -1.57079632f){ l_angle += 4.71238898f; } // Q3 -> soma pi*3/2
	                    else if(l_angle < 0           ){ l_angle += 1.57079632f; } // Q4 -> soma pi/2
	                    else if(l_angle < 1.57079632f ){ l_angle += 1.57079632f; } // Q1 -> soma pi/2
	                    else                           { l_angle -= 1.57079632f; } // Q2 -> subtrai pi/2

	                    // Essa tolerância grande permite que a linha pequena seja casada com quase tudo, exceto linhas perpendiculares
	                    l_angle_tolerance = 1.22f; // tolerância de 70 graus
	                    break; // fim da busca por linhas grandes próximas
	                }
	            }
	        }

	        // Procura a distância para a linha de campo mais próxima
	        // Erro padrão de 1e6f é aplicado quando não há correspondência (matriz de transf. Xvec/Yvec errada)
	        float min_err = 1e6f;
	        for(
	        	const auto& segm : RobovizField::cSegmentos::list
	        ){ 

	            // Ignora linha de campo se a linha observada for substancialmente maior
	            if( linha_qualquer.comprimento > (segm.comprimento + 0.7) ){ continue; }

	            // Ignora linha de campo se a orientação não coincidir
	            float angle_difference = fabsf(l_angle - segm.ang);
	            if(angle_difference > 1.57079632f)       { angle_difference = 3.14159265f - angle_difference; }
	            if(angle_difference > l_angle_tolerance) { continue; }
	            
	            // Erro é a soma das distâncias de um segmento de linha para ambos os extremos da linha observada
	            float err = RobovizField::calcular_dist_segm_para_pt2D_c(segm,ponto_inicial_da_linha_abs.to_2d());
	            if(err < min_err) { err += RobovizField::calcular_dist_segm_para_pt2D_c(segm,ponto_final_da_linha_abs.to_2d()); }

	            if(err < min_err) { min_err = err; }
	        }

	        total_err += min_err;
	        total_err_cnt += 2; // uma linha tem 2 pontos, peso dobrado comparado a um único marco
	    }

	    for(
	    	const RobovizField::sMkr& m : campo_existente.list_landmarks
	    ){

	    	// calcula coordenadas absolutas conforme a transformação
	        Vetor3D lpt = transfMat * m.pos_rel_cart; 

	        float err = lpt.to_2d().obter_distancia_para(Vetor2D(m.pos_abs.x, m.pos_abs.y));
	        total_err += err > 0.5 ? err * 100 : err;
	        total_err_cnt++;
	    }

	    double media_de_error = total_err / total_err_cnt;

	    return (!gsl_finite(media_de_error)) ? 1e6 : media_de_error;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool 
	calcular_orientacao_eixo_z(){
		/*
		Descrição:
			Determina o vetor de orientação do eixo Z (normal ao solo) do agente, com base em marcadores e linhas detectadas no campo.
			O método utiliza diferentes estratégias dependendo da quantidade e qualidade dos marcadores e segmentos de linha disponíveis,
			buscando sempre aplicar a solução mais robusta e confiável para estimar a orientação vertical do agente.

		Parâmetros:
			- Não possui parâmetros de entrada.

		Retorno:
			- true  — se foi possível determinar e atualizar o vetor de orientação do eixo Z.
			- false — se não foi possível encontrar uma solução adequada
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		const int goalNo = campo_existente.list_landmarks_goalposts.size();

		if(
			campo_existente.marcadores_nao_colineares >= 3
		){

			estimar_plano_solo_e_altura();
			return true;
		}

		// Neste ponto temos mais que 0 linhas. Ter 1 linha é o caso mais comum.
		// Também é possível ter 2 linhas se pelo menos uma delas for muito curta para criar 2 pontos de referência.
		// Essas 2 linhas podem ser ambas muito curtas, o que não é ideal para os algoritmos abaixo.
		
		// Passos: encontrar a maior linha, verificar se ela é longa o suficiente
		const Linha6D* linha_qualquer = &campo_existente.list_segments.front();
		if( 
			campo_existente.list_segments.size() > 1
		){
			for(
				const auto& outra_linha: campo_existente.list_segments
			){

				if( outra_linha.comprimento > linha_qualquer->comprimento ) { linha_qualquer = &outra_linha; }
			}
		}

		// A maior linha é muito curta
		if( linha_qualquer->comprimento < 1 ){ 

			atualizar_estado_do_sistema(FAILzLine);
			return false;
		}

		// Exemplo de caso: 1 canto e 1 linha (ou 2 linhas curtas)
		if(goalNo == 0){ 
			atualizar_estado_do_sistema(FAILzNOgoal);
			return false;
		}
			
		// Prepararamos variáveis para as soluções A e B

		// Pega qualquer ponto do travessão
		Vetor3D crossbar_pt;
		vector<RobovizField::sMkr>& glist = campo_existente.list_landmarks_goalposts;
		if(
			goalNo == 1
		){

			crossbar_pt = glist.front().pos_rel_cart;
		}
		else{

			if(
				// Caso haja 2 traves do mesmo  gol
				// Comparamos x pois o campo está na horizontal.
				glist[0].pos_abs.x == glist[1].pos_abs.x
			){

				crossbar_pt = Vetor3D::obter_ponto_medio( glist[0].pos_rel_cart, glist[1].pos_rel_cart );
			}
			else{

				// Extremamente raro: há outras soluções quando goalNo>2, mas custo/benefício não compensa
				crossbar_pt = glist.front().pos_rel_cart; 
			}
		}

		// Identificar e aplicar as soluções A e B, descritas no markdown

		// Pega o ponto da linha mais próximo ao ponto do travessão
		Vetor3D ponto = linha_qualquer->ponto_mais_proximo_na_reta_para_ponto_cartesiano( crossbar_pt );
		Vetor3D possibleZvec = crossbar_pt - ponto;
		float possibleZveccomprimento = possibleZvec.obter_modulo();

		// Soluções A e B
		if(
			fabsf(possibleZveccomprimento - 0.8) < 0.05
		){

			Vetor3D unit_zvec = possibleZvec / possibleZveccomprimento;

			// Salva como o novo vetor de orientação do eixo z
			_Head_to_Field_Prelim.setar(2,0,unit_zvec.x);
			_Head_to_Field_Prelim.setar(2,1,unit_zvec.y);
			_Head_to_Field_Prelim.setar(2,2,unit_zvec.z);

			estimar_altura(unit_zvec);
			return true;
		}

		// Resta-nos a solução C

		// Este vetor do travessão aponta para a esquerda se visto do meio de campo (isso é importante para o produto vetorial)
		Vetor3D crossbar_left_vec, crossbar_midp;

		const auto& goal_mm = RobovizField::gMkrs::goal_mm;
		const auto& goal_mp = RobovizField::gMkrs::goal_mp;
		const auto& goal_pm = RobovizField::gMkrs::goal_pm;
		const auto& goal_pp = RobovizField::gMkrs::goal_pp;

		if(                     goal_mm.detectado   && goal_mp.detectado){

			crossbar_left_vec = goal_mm.pos_rel_cart - goal_mp.pos_rel_cart;
			crossbar_midp =    (goal_mm.pos_rel_cart + goal_mp.pos_rel_cart)/2;
		}else if(               goal_pp.detectado   && goal_pm.detectado){

			crossbar_left_vec = goal_pp.pos_rel_cart - goal_pm.pos_rel_cart;
			crossbar_midp =    (goal_pp.pos_rel_cart + goal_pm.pos_rel_cart)/2;
		}

		// Verifica se o ângulo entre a linha e o travessão está entre 45° e 135°
		// 45° < acos(line.crossbar / |line||crossbar|) < 135°
		// | line.crossbar | < cos(45°) * |line| * ~2.1
		// | line.crossbar | < 1.485 * |line|
		Vetor3D lvec = linha_qualquer->ponto_final_cartesiano - linha_qualquer->ponto_inicial_cartesiano;
		if( 
			goalNo > 1 && fabsf(lvec.InnerProduct(crossbar_left_vec)) < 1.485 * linha_qualquer->comprimento 
		){
			Vetor3D Zvec;
			if(
				linha_qualquer->ponto_inicial_cartesiano.obter_distancia_para(crossbar_midp) > linha_qualquer->ponto_final_cartesiano.obter_distancia_para(crossbar_midp)
			){

				Zvec = lvec.CrossProduct(crossbar_left_vec);
			}else{

				Zvec = crossbar_left_vec.CrossProduct(lvec);
			}

			// Obtém vetor unitário
			Zvec = Zvec.normalize(); 

			// Salva como o novo vetor de orientação do eixo z
			_Head_to_Field_Prelim.setar(2,0,Zvec.x);
			_Head_to_Field_Prelim.setar(2,1,Zvec.y);
			_Head_to_Field_Prelim.setar(2,2,Zvec.z);

			estimar_altura(Zvec);
			return true;
		}

		atualizar_estado_do_sistema(FAILz);
		return false; // Nenhuma solução foi encontrada
	}

	void 
	estimar_plano_solo_e_altura(){
		/*
		Descrição:
			Encontra o vetor normal do plano do solo que melhor se ajusta aos marcadores do chão utilizando 
			Decomposição em Valores Singulares (SVD).
			
			Também calcula a altura do agente com base no centróide dos marcadores de referência no chão.
				- Requer pelo menos 3 referências de solo para funcionar corretamente.

		Parâmetros:
			None

		Retorno:
			Sem valor de retorno.
			- Atualiza o vetor normal ao plano do solo na matriz preliminar.
			- Não necessarimente, atualiza a altura da cabeça do agente.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		const auto& ground_markers = campo_existente.list_weighted_ground_markers;
		const int   ground_m_size  = ground_markers.size();

		// Calcula o plano dos marcadores do solo (se houver ao menos 3 marcadores)

		gsl_matrix *matriz_marcadores_solo     = gsl_matrix_alloc(ground_m_size, 3); 
		gsl_matrix *matriz_vetores_singulares  = gsl_matrix_alloc(3, 3); 
		gsl_vector *valores_singulares         = gsl_vector_alloc(3); 
		gsl_vector *vetor_trabalho_svd         = gsl_vector_alloc(3);

		// Calcula o centróide dos marcadores
		Vetor3D centroid(0,0,0);
		for(
			const auto& gmkr : ground_markers
		){ 

			centroid += gmkr.pos_rel_cart;
		}
		centroid /= (float) ground_m_size;

		// Insere todos os marcadores na matriz após subtrair o centróide
		for(
			int i = 0; 
				i < ground_m_size; 
				i++
		){ 

			gsl_matrix_set(matriz_marcadores_solo,i,0, ground_markers[i].pos_rel_cart.x - centroid.x );
			gsl_matrix_set(matriz_marcadores_solo,i,1, ground_markers[i].pos_rel_cart.y - centroid.y );
			gsl_matrix_set(matriz_marcadores_solo,i,2, ground_markers[i].pos_rel_cart.z - centroid.z );
		} 

		// Decomposição em valores singulares para encontrar o plano que melhor se ajusta
		gsl_linalg_SV_decomp(
							matriz_marcadores_solo,
							matriz_vetores_singulares,
							valores_singulares,
							vetor_trabalho_svd
							);

		// plano: ax + by + cz = d
		double a = gsl_matrix_get(matriz_vetores_singulares,0,2);
		double b = gsl_matrix_get(matriz_vetores_singulares,1,2);
		double c = gsl_matrix_get(matriz_vetores_singulares,2,2);

		/*
		 * Equação do plano: ax + by + cz = d
		 * Portanto, considerando o centróide(x,y,z) e o vetor normal(a,b,c):
		 *  d = ax + by + cz
		 *    = normalvec . centroid
		 */
		double estimativa_da_altura = a * centroid.x + b * centroid.y + c * centroid.z;

		// Nota: |d| é uma estimativa da altura do agente, mas é possível melhorar incluindo referências aéreas posteriormente

		gsl_matrix_free (matriz_marcadores_solo);
		gsl_matrix_free (matriz_vetores_singulares);
		gsl_vector_free (valores_singulares);
		gsl_vector_free (vetor_trabalho_svd);

		/*
		Infelizmente, o vetor normal nem sempre aponta para cima
		O plano é definido por (ax + by + cz - d = 0)
		Substituindo [x,y,z] por um ponto aleatório p, o sinal de (ax + by + cz - d) indica de que lado p está
		Prova de que o sinal de (ax + by + cz - d) é positivo para (ponto_aleatorio(i,j,k) + vetor normal(a,b,c)):
				ax + by + cz - d  =  
				a(i+a) + b(j+b) + c(k+c) - (a*i + b*j + c*k)  =
				ai + aa + bj + bb + ck + cc - ai - bj - ck  =
				aa + bb + cc  (que é sempre positivo e igual a 1 pois é um vetor unitário) 
		Como o agente está sempre acima do campo, só precisamos garantir que ele esteja do mesmo lado que o vetor normal
		Para isso, verificamos se a origem do sistema de coordenadas relativo (cabeça do agente) gera sinal positivo para (ax + by + cz - d)
				ax + by + cz - d  > 0   (substituindo x=0, y=0, z=0)
				d < 0

		No entanto, se o agente estiver deitado, o plano otimizado pode estar ligeiramente acima da cabeça devido a erros de visão
		Então, se tivermos um ponto de referência melhor, como uma trave, utilizamos ele
		*/
		if(
			// Se há traves visíveis
			!campo_existente.list_landmarks_goalposts.empty()
		){

			// ponto aéreo aleatório (trave)
			const Vetor3D& aerialpt = campo_existente.list_landmarks_goalposts.front().pos_rel_cart; 

			// a trave está do lado negativo, então invertemos o vetor normal
			if( 
				(a * aerialpt.x + b * aerialpt.y + c * aerialpt.z) < estimativa_da_altura 
			){ 

				a = -a; b = -b; c = -c;
			}
		}
		else{ 
			// Se não há traves visíveis, usamos a cabeça do agente como referência

			// o vetor normal aponta para baixo, então invertemos
			if( estimativa_da_altura > 0 ){ a = -a; b = -b; c = -c; }
		}

		// Salva o vetor normal do plano do solo como o novo vetor de orientação do eixo z
		_Head_to_Field_Prelim.setar(2,0,a);
		_Head_to_Field_Prelim.setar(2,1,b);
		_Head_to_Field_Prelim.setar(2,2,c);

		// Calcula a altura do agente
		float h = max( - centroid.x * a - centroid.y * b - centroid.z * c, 0.064 );
		
		_Head_to_Field_Prelim.setar(2,3, h ); 

		// Define o valor público independente de coordenada z
		// (pode ficar fora de sincronia com a matriz de transformação)
		_last_z = _final_z;
		_final_z = h;
		_is_head_z_uptodate = true;

		return;
	}

	void 
	estimar_altura( const Vetor3D& Zvec ){
		/*
		Descrição:
			Calcula a translação no eixo z (altura) do agente utilizando os marcadores do chão visíveis.
			Observação: Não há real benefício em considerar os postes do gol (com ou sem peso), exceto 
			quando o número de objetos visíveis é menor que 5 ou 6, e mesmo nesses casos a diferença é mínima.

		Parâmetros:
			- Zvec: 
				vetor de orientação do eixo z (direção vertical) a ser utilizado para projetar os marcadores do chão.

		Retorno:
			- Não possui valor de retorno.
			- Atualiza o valor da altura.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		Vetor3D zsum;
		for(
			const auto& gmkr : campo_existente.list_weighted_ground_markers
		){

			zsum += gmkr.pos_rel_cart;
		}

		// Altura mínima: 0.064m
		float z =  max( 
					  -( zsum / campo_existente.list_weighted_ground_markers.size() ).InnerProduct(Zvec)  ,
					  0.064f
					  );

		_Head_to_Field_Prelim.setar( 2, 3, z ); 

		// Atualiza a coordenada pública independente z (Pode ficar fora de sincronia com a matriz de transformação, mas não é um problema)
		_last_z = _final_z;
		_final_z = z;
		_is_head_z_uptodate = true;
	}

	bool 
	fine_tune(  // Não achei nome melhor
		float initial_angle, 
		float initial_x, 
		float initial_y
	);

	bool 
	fine_tune_aux(
	    float &initial_angle, 
	    float &initial_x, 
	    float &initial_y, 
	    bool   use_probabilities
	){
		/*
		Descrição:
			Aplica o ajuste fino (fine tuning) à estimativa de orientação e posição do agente.
		    O ajuste pode ser feito de duas formas:
		    - Diretamente nos valores iniciais de ângulo, x e y se use_probabilities == false.
		    - Diretamente na matriz _Head_to_Field_Prelim utilizando probabilidades (se use_probabilities == true).

		    Utiliza o algoritmo de minimizar simplex do GSL para encontrar a melhor configuração dos parâmetros,
		    otimizando a função de erro escolhida (distância euclidiana 2D ou log-probabilidade).

		Parâmetros:
			- initial_angle: referência para o ângulo inicial de Xvec ao redor de Zvec (pode ser atualizado pela função).
		    - initial_x: referência para a translação inicial em x (pode ser atualizado pela função).
		    - initial_y: referência para a translação inicial em y (pode ser atualizado pela função).
		    - use_probabilities: se true, usa função de erro baseada em probabilidade; se false, usa erro euclidiano 2D.

		Retorno:
			- true  —> se o ajuste fino foi realizado com sucesso e os parâmetros/matriz foram atualizados.
		    - false —> se o ajuste fino falhou (por exemplo, erro de mapeamento acima do limite permitido).
		*/

		int status, iter = 0;
		gsl_vector* x =  criar_vetor_gsl<3>({initial_x, initial_y, initial_angle});    // Transformação inicial
		gsl_vector* ss = criar_vetor_gsl<3>({0.02, 0.02, 0.03});                       // Define os tamanhos de passo iniciais
		gsl_multimin_function minex_func = {map_error_euclidian_distance, 3, nullptr}; // Função de erro, número de variáveis, params
		if(use_probabilities) { minex_func.f = map_error_logprob; } 		           // Usa função de erro baseada em probabilidade

		const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;   // Tipo de algoritmo
		gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc (T, 3);            // Aloca espaço de trabalho
	  	gsl_multimin_fminimizer_set (s, &minex_func, x, ss);                          // Configura o espaço de trabalho

		float best_x, best_y, best_ang;

	  	do{
			iter++;
			status = gsl_multimin_fminimizer_iterate(s);

			// *s armazena a melhor solução, não necessariamente a última
			best_x = gsl_vector_get (s->x, 0);
			best_y = gsl_vector_get (s->x, 1);
			best_ang = gsl_vector_get (s->x, 2);

			if (status) { break; }

			// Tamanho característico específico do minimizador
			double size = gsl_multimin_fminimizer_size (s); 

			// Critério de parada: simplex contrai próximo ao mínimo
			status      = gsl_multimin_test_size (size, 1e-3);   
	    }
		while ((status == GSL_CONTINUE || use_probabilities) && iter < 40);

		float best_map_error = s->fval;

		gsl_vector_free(x);
		gsl_vector_free(ss);
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

		// Neste ponto, use_probabilities é true
		// Nota: as transformações são testadas diretamente em _Head_to_Field_Prelim, mas ao final
		// ela guarda o último teste, então configuramos manualmente aqui com a melhor solução encontrada

		// Converte o ângulo em Xvec e Yvec
		Vetor3D Zvec(_Head_to_Field_Prelim.obter(2,0), _Head_to_Field_Prelim.obter(2,1), _Head_to_Field_Prelim.obter(2,2));
		Vetor3D Xvec, Yvec;
		calcular_eixos_XY_a_partir_de_Z(Zvec, best_ang, Xvec, Yvec );

		_Head_to_Field_Prelim.setar(0,0, Xvec.x);
		_Head_to_Field_Prelim.setar(0,1, Xvec.y);
		_Head_to_Field_Prelim.setar(0,2, Xvec.z);
		_Head_to_Field_Prelim.setar(0,3, best_x);
		_Head_to_Field_Prelim.setar(1,0, Yvec.x);
		_Head_to_Field_Prelim.setar(1,1, Yvec.y);
		_Head_to_Field_Prelim.setar(1,2, Yvec.z);
		_Head_to_Field_Prelim.setar(1,3, best_y);

		return true;
	}

	bool 
	calcular_translacao_rotacao_xy(){
		/*
		Descrição:
			Encontra a translação e rotação XY do agente no campo com base em dois marcos (landmarks) visíveis.
			Uma solução única é garantida se o vetor Z (Zvec) estiver apontando na direção correta.
			Utiliza dois pontos de referência para estimar o ângulo do agente e a translação, e então aplica ajuste fino.

		Parâmetros:
			None

		Retorno:
			- true  — se a translação e rotação XY foram determinadas com sucesso.
			- false — caso contrário (por exemplo, se o ajuste fino falhar).
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		Vetor3D Zvec(_Head_to_Field_Prelim.obter(2,0), _Head_to_Field_Prelim.obter(2,1), _Head_to_Field_Prelim.obter(2,2));

		RobovizField::sMkr *m1 = nullptr, *m2 = nullptr;

		// Obtém o maior número possível de cantos
		if(campo_existente.list_landmarks_corners.size() > 1){

			m1 = &campo_existente.list_landmarks_corners[0];
			m2 = &campo_existente.list_landmarks_corners[1];
		}else if(campo_existente.list_landmarks_corners.size()==1){

			m1 = &campo_existente.list_landmarks_corners  [0];
			m2 = &campo_existente.list_landmarks_goalposts[0];
		}else{

			m1 = &campo_existente.list_landmarks_goalposts[0];
			m2 = &campo_existente.list_landmarks_goalposts[1];
		}

		Vetor3D realVec(m2->pos_abs.x - m1->pos_abs.x,  m2->pos_abs.y - m1->pos_abs.y,  m2->pos_abs.z - m1->pos_abs.z);
		float real_angle = atan2f(realVec.y, realVec.x); // ângulo do vetor real

		Vetor3D seenVec(m2->pos_rel_cart - m1->pos_rel_cart);
		Vetor3D rotated_abs_vec = rotacao_rapida_em_torno_eixo_solo(seenVec, Zvec);
		float seen_angle = atan2f(rotated_abs_vec.y, rotated_abs_vec.x); // ângulo do vetor observado (após rotação)

		// não é necessário normalizar
		float AgentAngle = real_angle - seen_angle; 
		
		Vetor3D Xvec, Yvec;
		calcular_eixos_XY_a_partir_de_Z(Zvec, AgentAngle, Xvec, Yvec );

		/*
		Seja m um marco, rel:(mx,my,mz), abs:(mabsx, mabsy, mabsz)
			XvecX*mx + XvecY*my + XvecZ*mz + AgentX = mabsx
			AgentX = mabsx - (XvecX*mx + XvecY*my + XvecZ*mz)
			AgentX = mabsx - (XvecX . m)

		Generalizando para N estimativas:
			AgentX = soma( mabsx - (XvecX . m) )/N
		*/
		float initial_x = 0, initial_y = 0;
		for(
			const RobovizField::sMkr& m : campo_existente.list_landmarks
		){

			initial_x += m.pos_abs.x - Xvec.InnerProduct(m.pos_rel_cart);
			initial_y += m.pos_abs.y - Yvec.InnerProduct(m.pos_rel_cart);
		}

		initial_x /= campo_existente.list_landmarks.size();
		initial_y /= campo_existente.list_landmarks.size();

		return fine_tune(AgentAngle, initial_x, initial_y);
	}

	bool 
	estimar_translacao_rotacao_xy(){
		/*
		Descrição:
		    Estima a posição e orientação XY do agente no campo usando a maior linha detectada 
		    (segmento) e, se disponível, um marco (landmark) visível.
		    
		    O algoritmo testa as quatro possíveis orientações da linha (X, -X, Y, -Y) e otimiza 
		    a translação XY para cada caso, buscando a configuração mais plausível.
		    Caso não haja marcos, utiliza a última posição conhecida como referência.

		    O processo seleciona a solução com menor erro de mapeamento e proximidade à última
		    posição conhecida, aplicando critérios adicionais para garantir plausibilidade.

		Parâmetros:
		    - Não possui parâmetros de entrada.

		Retorno:
		    - true  — caso uma solução plausível seja encontrada e o ajuste fino (fine tune) seja bem-sucedido.
		    - false — se não for possível encontrar uma solução plausível ou se o ajuste fino falhar.
		*/

		RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

		// Obtém Zvec dos passos anteriores
		Vetor3D Zvec(_Head_to_Field_Prelim.obter(2,0), _Head_to_Field_Prelim.obter(2,1), _Head_to_Field_Prelim.obter(2,2));
		Vetor2D last_known_position(head_position.x, head_position.y);

		// Obtém a maior linha e a utiliza como vetor X ou Y
		const Linha6D* maior_linha = &campo_existente.list_segments.front();
		for(const Linha6D& l : campo_existente.list_segments){ if( l.comprimento > maior_linha->comprimento ) { maior_linha = &l; } }

		if(
			maior_linha->comprimento < 1.6
		){
			// A maior linha é muito curta, pode ser confundida com uma linha de anel

			atualizar_estado_do_sistema(FAILguessLine);
			return false; 
		}

		// Gira a linha para o plano do solo real, eliminando a terceira dimensão
		Vetor3D maior_linhaVec = maior_linha->ponto_final_cartesiano - maior_linha->ponto_inicial_cartesiano;
		Vetor3D rotated_abs_line = rotacao_rapida_em_torno_eixo_solo(maior_linhaVec, Zvec);

		// A linha pode estar alinhada com X ou Y, positiva ou negativamente (esses ângulos não precisam ser normalizados)
		float fixed_angle[4];
		fixed_angle[0] = -atan2f( rotated_abs_line.y,rotated_abs_line.x ); // se maior_linhaVec é Xvec
		fixed_angle[1] = fixed_angle[0] + 3.14159265f; // se maior_linhaVec é -Xvec
		fixed_angle[2] = fixed_angle[0] + 1.57079633f; // se maior_linhaVec é Yvec
		fixed_angle[3] = fixed_angle[0] - 1.57079633f; // se maior_linhaVec é -Yvec

		// Obtém translação inicial

		// Se vemos 1 marco, usamos ele; caso contrário, utilizamos a última posição conhecida

		float initial_x[4], initial_y[4];
		bool noLandmarks = campo_existente.list_landmarks.empty();

		if(
			noLandmarks
		){

			for(int i=0; i<4; i++){
				initial_x[i] = last_known_position.x;
				initial_y[i] = last_known_position.y;
			}
		} else {

			Vetor3D Xvec = maior_linhaVec / maior_linha->comprimento;
			Vetor3D Yvec(Zvec.CrossProduct(Xvec));

			/*
			Seja m um marco, rel:(mx,my,mz), abs:(mabsx, mabsy, mabsz)
				XvecX*mx + XvecY*my + XvecZ*mz + AgentX = mabsx
				AgentX = mabsx - (XvecX*mx + XvecY*my + XvecZ*mz)
				AgentX = mabsx - (XvecX . m)
			*/

			const RobovizField::sMkr& m = campo_existente.list_landmarks.front();
			const float x_aux = Xvec.InnerProduct(m.pos_rel_cart);
			const float y_aux = Yvec.InnerProduct(m.pos_rel_cart);

			initial_x[0] = m.pos_abs.x - x_aux;
			initial_y[0] = m.pos_abs.y - y_aux;
			initial_x[1] = m.pos_abs.x + x_aux; // 2ª versão: X invertido
			initial_y[1] = m.pos_abs.y + y_aux; // 2ª versão: Y invertido
			initial_x[2] = m.pos_abs.x + y_aux; // 3ª versão: X invertido Y
			initial_y[2] = m.pos_abs.y - x_aux; // 3ª versão: Y é X
			initial_x[3] = m.pos_abs.x - y_aux; // 4ª versão: X é Y
			initial_y[3] = m.pos_abs.y + x_aux; // 4ª versão: Y é X invertido
		}
		
		// ------------------------------------------------------------ Otimiza rotação XY para cada orientação possível

		const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
		gsl_multimin_fminimizer *s[4] = {nullptr,nullptr,nullptr,nullptr};
		gsl_vector *ss[4], *x[4];
		gsl_multimin_function minex_func[4];

		size_t iter = 0;
		int status;
		double size;

		for(int i=0; i<4; i++){

			x[i]  = criar_vetor_gsl<2>({initial_x[i], initial_y[i]}); // Transformação inicial
			ss[i] = criar_vetor_gsl<2>({1, 1});						  // Passos iniciais em 1

			// Inicializa método
			minex_func[i].n = 2;
			minex_func[i].f = map_error_euclidian_distance;
			minex_func[i].params = &fixed_angle[i];	

			s[i] = gsl_multimin_fminimizer_alloc (T, 2);
	  		gsl_multimin_fminimizer_set (s[i], &minex_func[i], x[i], ss[i]);
		}

		// Inicia iteração de minimização para cada caso possível
		bool running[4]        = {true,true,true,true};
		float current_error[4] = {1e6,1e6,1e6,1e6};
		float lowest_error     = 1e6;
		Vetor2D best_xy[4]; 
		const int maximum_iterations = 50;
		bool plausible_solution[4] = {false,false,false,false};
	  	do{
			iter++;
			for(int i=0; i<4; i++){
				if(!running[i]) { continue; }

				status = gsl_multimin_fminimizer_iterate(s[i]);

				current_error[i] = s[i]->fval;
				if(current_error[i] < lowest_error) lowest_error = current_error[i];

				// Possíveis erros: tamanhos incompatíveis, contração falhou, etc.
				if (status) {  

					running[i] = false; // Esta não é uma solução válida
					continue; 
				}

				// Critério de parada da minimização: simplex se contrai próximo ao mínimo
				size = gsl_multimin_fminimizer_size (s[i]);
				status = gsl_multimin_test_size (size, 1e-2);

				if(status != GSL_CONTINUE || (lowest_error * 50 < current_error[i])) { // finalizou ou abortou
					
					best_xy[i].x = gsl_vector_get (s[i]->x, 0);
					best_xy[i].y = gsl_vector_get (s[i]->x, 1);
					running[i] = false; 
					plausible_solution[i]=(status == GSL_SUCCESS); // somente válido se convergiu para mínimo local
					continue; 
				} 
			}	
	    } while (iter < maximum_iterations && (running[0] || running[1] || running[2] || running[3]));

		for(int i=0; i<4; i++){
			gsl_vector_free(x[i]);
			gsl_vector_free(ss[i]);
			gsl_multimin_fminimizer_free (s[i]);
		}

		// Neste ponto, uma solução é plausível se convergiu para um mínimo local
		// Aplica critérios adicionais de plausibilidade (erro de mapeamento, distância à última posição)
		int plausible_count = 0;
		int last_i;
		for(int i=0; i<4; i++){
			if(!plausible_solution[i]) continue;
			bool isDistanceOk = (!noLandmarks) || last_known_position.obter_distancia_para(best_xy[i]) < 0.5; // distância à última posição conhecida
			if(current_error[i] < 0.12 && isDistanceOk){ // erro de mapeamento
				plausible_count++; 
				last_i = i;
			}
		}

		// Se houver mais de uma opção, usa distância à última posição para eliminar candidatos
		if(!noLandmarks && plausible_count>1){
			plausible_count = 0;
			for(int i=0; i<4; i++){
				if(plausible_solution[i] && last_known_position.obter_distancia_para(best_xy[i]) < 0.5){
					plausible_count++; 
					last_i = i;
				}
			}
		}

		// Se nenhuma solução for plausível, retorna falha
		if(plausible_count==0){

			atualizar_estado_do_sistema(FAILguessNone);
			return false; 

			// Se houver mais de uma solução plausível, retorna falha
		}else if(plausible_count>1){

			atualizar_estado_do_sistema(FAILguessMany);
			return false;

			// Se o erro de mapeamento for alto ou a distância à última posição for grande, retorna falha
		}else if(current_error[last_i] > 0.06 || (noLandmarks && last_known_position.obter_distancia_para(best_xy[last_i]) > 0.3)){

			atualizar_estado_do_sistema(FAILguessTest);
			return false;
		}

		// Caso contrário, faz ajuste fino com a melhor solução encontrada
		return fine_tune(fixed_angle[last_i],best_xy[last_i].x, best_xy[last_i].y);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void
	commit_system(){
		/*
	 	Descrição:
			Atualiza todas as variáveis públicas relacionadas à transformação do localizador, incluindo rotação e translação.
			Esta função sincroniza a transformação preliminar com a final, calcula a matriz inversa, atualiza as matrizes de rotação direta e inversa,
			armazena a nova translação, marca o estado como atualizado, reinicia o contador de passos desde a última atualização
			e adiciona a posição 3D corrente ao histórico de posições.

		Parâmetros:
			None

		Retorno:
			None
		*/

		_final_Head_to_Field_Transform = _Head_to_Field_Prelim;

		_final_Head_to_Field_Transform.popular_transformacao_inversa( _final_Field_to_Head_Transform );

		_final_vetor_de_translacao = _final_Head_to_Field_Transform.obter_vetor_de_translacao();

		_is_uptodate = true;

		_steps_since_last_update = 0;
	}

    int
	estimar_erro_posicional(
	    const Vetor3D  posicao_estimativa,
	    const Vetor3D& posicao_real,
	    	  double    error_placeholder[]
	);

    void 
    stats_reset(){
    	/* 
    	Descrição:
    		Apenas resetará os valores de erro e de sistema obtidos após os cálculos

		Parâmetros:
			None

		Retorno:
			Nada além do reset dos counters.
    	*/

    	counter_fineTune = 0;

    	for(
    		int i = 0;
    			// i < sizeof(errorSum_fineTune_before) / sizeof(errorSum_fineTune_before[0]);
    			/*
				Estes valores são alterados dentro da função de estimar_error_posicional()
				que consta em LocalizerV2.cpp, entretanto, não consigo imaginar uma situação,
				sem ser race-condition, que esta função será chamada no momento. 
				Logo, acredito que seja possível isso.
    			*/
    			i < 7;
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
    
    // [0,1,2]- xyz err sum, [3]-2D err sum, [4]-2D err sq sum, [5]-3D err sum, [6]-3D err sq sum
    double errorSum_fineTune_before[7]        = {0};        
    double errorSum_fineTune_euclidianDist[7] = {0}; 
    double errorSum_fineTune_probabilistic[7] = {0}; 
    double errorSum_ball[7]                   = {0};                   
    int    counter_fineTune                   =  0;
    int    counter_ball                       =  0;

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

    void 
    atualizar_estado_do_sistema(enum STATE novo_estado){

    	contador_de_estados_do_sistema[novo_estado]++;
    	estado_do_sistema = novo_estado;
    }

    int contador_de_estados_do_sistema[STATE::ENUMSIZE] = {0};

public:

	const Matriz4D& Head_to_Field_Transform = _final_Head_to_Field_Transform; // rotação + translação
	//const Matriz4D& Head_to_Field_Rotate    = _final_Head_to_Field_Rotate;    // rotação
	const Matriz4D& Field_to_Head_Transform = _final_Field_to_Head_Transform; // rotação + translação
	//const Matriz4D& Field_to_Head_Rotate    = _final_Field_to_Head_Rotate;    // rotação

	// Parte referente à translação de _final_Head_to_Field_Transform
	const Vetor3D& head_position = _final_vetor_de_translacao;

	/*
	Coordenada Z da cabeça:

	- Frequentemente igual a head_position.z, mas pode diferir quando não é possível calcular a rotação e translação completas.
	- Pode ser estimada por visão mesmo quando outras transformações 3D falham.
	- Útil para detectar quedas ou como observação em aprendizado de máquina.
	- Não deve ser usada em transformações tridimensionais completas.
	*/
	const float& head_z = _final_z;

	/*
	Indicador de atualização da coordenada Z da cabeça:

	- head_z pode ser calculada mesmo quando a localização do próprio agente não é possível.
	- Esta variável é verdadeira quando head_z está atualizada.
	*/
	const bool &is_head_z_uptodate = _is_head_z_uptodate;

	/**
     * True if head_position and the transformation matrices are up to date
     * (false if this is not a visual step, or not enough elements are visible)
     */
	const bool &is_uptodate = _is_uptodate;

	/*
	Número de passos de simulação desde a última atualização:

	- Conta quantos passos ocorreram desde a última atualização.
	- Quando is_uptodate for true, esse valor é zero.
	*/
	const unsigned int &steps_since_last_update = _steps_since_last_update;

	void 
    reportar_situacao( bool for_debugging = false ) const {
		/*
		Descrição:
		    Gera um relatório estatístico de desempenho do algoritmo, avaliando erros e estágios do sistema.
		    Usada para diagnóstico e análise da qualidade dos ajustes de localização, exibindo métricas
		    como erro médio absoluto (MAE), desvio padrão (STD) e erro médio viés (MBE) em diferentes etapas.

		Parâmetros:
		    - for_debugging
		        Indica se o relatório é para depuração detalhada.

		        Se falso, o relatório é gerado somente após certo número de refinamentos.
		        Se verdadeiro, ignora essas restrições e sempre gera o relatório.

		Retorno:
		    Nenhum retorno.
		    Os dados são impressos diretamente no console via printf.
		*/


		if(!for_debugging){

			if(
				// Se for primeira vez
				counter_fineTune == 0
			){

				printf("LocalizerV2 reportando -> Verifique se o servidor está provendo dados privados (cheat data).\n");
				printf("-> Primeira execução: counter_fineTune = 0.\n");
				return;
			}

			if(
				counter_fineTune < 2
			){

				return;
			}
		}

		const int &c  = counter_fineTune;
		const int &cb =     counter_ball;
		const int c1  =            c - 1;
		const int cb1 =           cb - 1;

		/* Cada index dos vetores de erro significam algo específico,
  		   veja no local de declaração para mais informações.
		*/
		const double* ptr = errorSum_fineTune_before;
		float e1_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e1_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		double e1[] = { ptr[3]/c, sqrt(e1_2d_var), ptr[5]/c, sqrt(e1_3d_var), ptr[0]/c, ptr[1]/c, ptr[2]/c };


		ptr = errorSum_fineTune_euclidianDist;
		float e2_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e2_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		double e2[] = { ptr[3]/c, sqrt(e2_2d_var), ptr[5]/c, sqrt(e2_3d_var), ptr[0]/c, ptr[1]/c, ptr[2]/c };

		ptr = errorSum_fineTune_probabilistic;
		float e3_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / c) / c1;
		float e3_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / c) / c1;
		double e3[] = { ptr[3]/c, sqrt(e3_2d_var), ptr[5]/c, sqrt(e3_3d_var), ptr[0]/c, ptr[1]/c, ptr[2]/c };

		ptr = errorSum_ball;
		float e4_2d_var=0, e4_3d_var=0;
		if(cb1 > 0){
			e4_2d_var = (ptr[4] - (ptr[3]*ptr[3]) / cb) / cb1;
			e4_3d_var = (ptr[6] - (ptr[5]*ptr[5]) / cb) / cb1;
		}
		double e4[] = { ptr[3]/cb, sqrt(e4_2d_var), ptr[5]/cb, sqrt(e4_3d_var), ptr[0]/cb, ptr[1]/cb, ptr[2]/cb };

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

	void 
	run();










































};

#endif // LOCALIZERV2_H
