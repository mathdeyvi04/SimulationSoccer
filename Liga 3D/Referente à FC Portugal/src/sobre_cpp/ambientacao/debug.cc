#include "LocalizerV2.h"
#include <iostream>
#include <cstdio>

using namespace std;

static LocalizerV2& loc = Singular<LocalizerV2>::obter_instancia();

void
apresentar_infos_gerais(){
	/*
    Descrição:
        Imprime no console diversos dados do ambiente (mundo_existente) em formato legível, 
        geralmente para depuração ou análise em integração com scripts Python.

        Inclui informações sobre contato dos pés, posição relativa e absoluta da 
        bola, posição do agente (“me cheat”), dados dos marcos (landmarks) e linhas 
        detectadas.

    Parâmetros:
        - Não possui parâmetros de entrada.

    Retorno:
        - Não possui retorno (void). Os dados são impressos no console padrão (stdout).
    */

    static World& mundo_existente = Singular<World>::obter_instancia();

	cout << "Pé Tocando: " << mundo_existente.pe_em_contato[0] << " " << mundo_existente.pe_em_contato[1] << endl;
    cout << "Ponto de Contato Rel (LFoot): " << mundo_existente.pos_rel_contato_pe[0].x << " " << mundo_existente.pos_rel_contato_pe[0].y << " " << mundo_existente.pos_rel_contato_pe[0].z << endl;
    cout << "Ponto de Contato Rel (RFoot): " << mundo_existente.pos_rel_contato_pe[1].x << " " << mundo_existente.pos_rel_contato_pe[1].y << " " << mundo_existente.pos_rel_contato_pe[1].z << endl;
    cout << "Bola Detectada: " << mundo_existente.bola_detectada << endl;
    cout << "Posição Relativa da Bola: " << mundo_existente.pos_rel_bola_cartesiana.x << " " << mundo_existente.pos_rel_bola_cartesiana.y << " " << mundo_existente.pos_rel_bola_cartesiana.z << endl;
    cout << "Posição Absoluta da Bola (Cheat): " << mundo_existente.pos_abs_bola_cartesiana_cheat.x << " " << mundo_existente.pos_abs_bola_cartesiana_cheat.y << " " << mundo_existente.pos_abs_bola_cartesiana_cheat.z << endl;
    cout << "Posição Absoluta do Robô (Cheat): " << mundo_existente.pos_abs_agente_cartesiana_cheat.x << " " << mundo_existente.pos_abs_agente_cartesiana_cheat.y << " " << mundo_existente.pos_abs_agente_cartesiana_cheat.z << endl;
    
    for(int i=0; i<8; i++){
        cout << "Landmark " << i << ": " <<
        mundo_existente.landmark[i].detectado << " " <<
        mundo_existente.landmark[i].eh_canto << " " <<
        mundo_existente.landmark[i].pos_absoluta.x << " " <<
        mundo_existente.landmark[i].pos_absoluta.y << " " <<
        mundo_existente.landmark[i].pos_absoluta.z << " " <<
        mundo_existente.landmark[i].pos_relativa.x << " " <<
        mundo_existente.landmark[i].pos_relativa.y << " " <<
        mundo_existente.landmark[i].pos_relativa.z << endl;
    }

    for(size_t i=0; i<mundo_existente.linhas_esfericas.size(); i++){
        cout << "Line " << i << ": " <<
        mundo_existente.linhas_esfericas[i].ponto_inicial.x << " " << 
        mundo_existente.linhas_esfericas[i].ponto_inicial.y << " " << 
        mundo_existente.linhas_esfericas[i].ponto_inicial.z << " " << 
        mundo_existente.linhas_esfericas[i].ponto_final.x << " " << 
        mundo_existente.linhas_esfericas[i].ponto_final.y << " " << 
        mundo_existente.linhas_esfericas[i].ponto_final.z << endl;
    }
}

void
reportar_situacao(){
	/*
	- Mesma descrição relativa ao método.
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

	loc.reportar_situacao();
}

void
ilustrador( bool is_right_side ){
	/*
	Descrição:
	    Responsável por ilustrar graficamente, via RobovizLogger, os elementos visíveis no campo de visão do agente.
	    Esta função desenha:
	        - Todas as linhas visíveis (segmentos observados)
	        - Segmentos identificados, com suas coordenadas absolutas corrigidas
	        - Pontos marcadores conhecidos (com nomes de referência)
	        - Pontos marcadores desconhecidos (representados por "?")
	        - A posição e orientação atual do agente (com uma seta)
	        - Um marcador para a bola (se necessário, pode ser incluído)

	    O desenho é feito no buffer identificado por "ambientacao", que é posteriormente trocado para exibição.

	    Caso o lado do campo seja invertido (`is_right_side == true`), haverá as conversões
	    necessárias.

	Parâmetros:
	    - Head_to_Field:
	        Matriz de transformação (Matriz4D) que converte coordenadas relativas do agente para coordenadas absolutas no campo.

	    - is_right_side:
	        Booleano que indica se o agente está no lado direito do campo. 

	Retorno:
	    - Nenhum valor é retornado.
	      A função realiza chamadas ao RobovizLogger para desenhar graficamente os elementos no campo.
	*/

	RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

	campo_existente.ilustrador(loc.Head_to_Field_Transform, is_right_side);
}

float*
localize_agent_pose(
    bool   lfoot_touch, 
    bool   rfoot_touch, 
    double feet_contact[],
    bool   ball_seen, 
    double ball_pos[],
    double me_pos[],
    double landmarks[],
    double lines[],
    int    lines_no,
    float* retval // somente para executarmos os testes
){
	/*
	Descrição:
		Realiza o processamento dos dados sensoriais do agente, atualiza o estado global do mundo, executa o algoritmo de localização para estimar a pose 6D (posição e orientação) e retorna as matrizes de transformação e informações auxiliares para uso externo.
		A função recebe dados de contato dos pés, informações sobre a bola, posição do agente, marcos (landmarks) e linhas visíveis, organiza esses dados para o sistema de localização e devolve os resultados principais em um vetor de floats.

	Parâmetros:
		- lfoot_touch: 
			Indica se o pé esquerdo está em contato com o solo.
		- rfoot_touch: 
			Indica se o pé direito está em contato com o solo.
		- feet_contact: 
			Vetor de 6 elementos contendo a posição relativa de contato dos pés (x, y, z para cada pé).
		- ball_seen: 
			Indica se a bola foi detectada.
		- ball_pos: 
			Vetor de 6 elementos contendo a posição relativa da bola (x, y, z) e a posição absoluta "cheat" (x, y, z).
		- me_pos: 
			Vetor de 3 elementos com a posição absoluta "cheat" do agente (x, y, z).
		- landmarks: 
			Vetor contendo os dados dos 8 marcos, cada um com 8 valores: [seen, isCorner, pos.x, pos.y, pos.z, rel_pos.x, rel_pos.y, rel_pos.z].
		- lines: 
			Vetor contendo os dados das linhas visíveis, cada linha com 6 valores: [start.x, start.y, start.z, end.x, end.y, end.z].
		- lines_no: 
			Quantidade de linhas presentes no vetor lines.

	Retorno:
		- float* : Ponteiro para um vetor de 35 floats, contendo:
		    - [0-15]: Matriz de transformação Head-To-Field (16 elementos).
		    - [16-31]: Matriz de transformação Field-To-Head (16 elementos).
		    - [32]: Indicador de atualização bem-sucedida (is_uptodate, 0 ou 1).
		    - [33]: Valor de head_z.
	    	- [34]: Indicador de atualização do head_z (is_head_z_uptodate, 0 ou 1).
	*/

	static World& mundo_existente = Singular<World>::obter_instancia();

	mundo_existente.pe_em_contato[0] = lfoot_touch;
    mundo_existente.pe_em_contato[1] = rfoot_touch;

    //Structure of feet_contact {lfoot_contact_pt, rfoot_contact_pt, lfoot_contact_rel_pos, rfoot_contact_rel_pos}

    mundo_existente.pos_rel_contato_pe[0].x = feet_contact[0];
    mundo_existente.pos_rel_contato_pe[0].y = feet_contact[1];
    mundo_existente.pos_rel_contato_pe[0].z = feet_contact[2];
    mundo_existente.pos_rel_contato_pe[1].x = feet_contact[3];
    mundo_existente.pos_rel_contato_pe[1].y = feet_contact[4];
    mundo_existente.pos_rel_contato_pe[1].z = feet_contact[5];

    mundo_existente.bola_detectada = ball_seen;

    //Structure of ball_pos {ball_rel_pos_cart, ball_cheat_abs_cart_pos}

    mundo_existente.pos_rel_bola_cartesiana.x = ball_pos[0];
    mundo_existente.pos_rel_bola_cartesiana.y = ball_pos[1];
    mundo_existente.pos_rel_bola_cartesiana.z = ball_pos[2];
    mundo_existente.pos_abs_bola_cartesiana_cheat.x = ball_pos[3];
    mundo_existente.pos_abs_bola_cartesiana_cheat.y = ball_pos[4];
    mundo_existente.pos_abs_bola_cartesiana_cheat.z = ball_pos[5];
    
    mundo_existente.pos_abs_agente_cartesiana_cheat.x = me_pos[0];
    mundo_existente.pos_abs_agente_cartesiana_cheat.y = me_pos[1];
    mundo_existente.pos_abs_agente_cartesiana_cheat.z = me_pos[2];

    for(int i=0; i<8; i++){

        mundo_existente.landmark[i].detectado = (bool) landmarks[0];
        mundo_existente.landmark[i].eh_canto  = (bool) landmarks[1];
        mundo_existente.landmark[i].pos_absoluta.x = landmarks[2];
        mundo_existente.landmark[i].pos_absoluta.y = landmarks[3];
        mundo_existente.landmark[i].pos_absoluta.z = landmarks[4];
        mundo_existente.landmark[i].pos_relativa.x = landmarks[5];
        mundo_existente.landmark[i].pos_relativa.y = landmarks[6];
        mundo_existente.landmark[i].pos_relativa.z = landmarks[7];

        landmarks += 8;
    }

    mundo_existente.linhas_esfericas.clear();

    for(int i=0; i<lines_no; i++){
        Vetor3D s(lines[0],lines[1],lines[2]);
        Vetor3D e(lines[3],lines[4],lines[5]);
        mundo_existente.linhas_esfericas.emplace_back(s, e); 
        lines += 6;
    }

    //apresentar_infos_gerais();
    
    // ================================================= 2. Compute 6D pose

    loc.run(); 
    
    // ================================================= 3. Prepare data to return
    
    float *ptr = retval;
    printf("\n-Head_to_Field\n");
    for(int i=0; i<16; i++){
        ptr[i] = loc.Head_to_Field_Transform.conteudo[i];
        printf("\n(index, valor) = (%d, %.8lf)", i, ptr[i]);
    }
    printf("\n");
    ptr += 16;
    printf("\n-Field_to_Head-\n");
    for(int i=0; i<16; i++){
        ptr[i] = loc.Field_to_Head_Transform.conteudo[i];
        printf("\n(index, valor) = (%d, %.8lf)", i, ptr[i]);
    }
    ptr += 16;
    printf("\n");

    ptr[0] = (float) loc.is_uptodate;
    ptr[1] = loc.head_z;
    ptr[2] = (float) loc.is_head_z_uptodate;

    printf("\nis_uptodate = %lf", ptr[0]);
    printf("\nhead_z = %lf", ptr[1]);
    printf("\nis_head_z_uptodate = %lf\n", ptr[2]);

    return retval;
}

int main(){

	double feet_contact[] = {0.02668597,  0.055     , -0.49031584,  0.02668597, -0.055     , -0.49031584};
    double ball_pos[] =     {22.3917517 ,  4.91904904, -0.44419865, -0.        , -0.        , 0.04 };
    double me_pos[] =       {-22.8 ,  -2.44,   0.48};
    double landmarks[] =    { 1.  ,   1.  , -15.  , -10.  ,   0.  ,  10.88, -37.74,  -2.42,
                              0.  ,   1.  , -15.  ,  10.  ,   0.  ,   0.  ,   0.  ,   0.  ,
                              1.  ,   1.  ,  15.  , -10.  ,   0.  ,  38.56,  -4.9 ,  -0.66,
                              1.  ,   1.  ,  15.  ,  10.  ,   0.  ,  39.75,  24.4 ,  -0.7 ,
                              1.  ,   0.  , -15.  ,  -1.05,   0.8 ,   7.94,  16.31,   2.42,
                              1.  ,   0.  , -15.  ,   1.05,   0.8 ,   8.55,  30.15,   2.11,
                              1.  ,   0.  ,  15.  ,  -1.05,   0.8 ,  37.82,   8.16,   0.5 ,
                              1.  ,   0.  ,  15.  ,   1.05,   0.8 ,  37.94,  11.77,   0.44 };
    double lines[] =        { 25.95,  35.02,  -1.14,  24.02, -12.26,  -1.12,
                              13.18,  59.93,  -2.11,  10.87, -37.8 ,  -2.69,
                              39.78,  24.32,  -0.75,  38.64,  -5.05,  -0.67,
                              10.89, -37.56,  -2.6 ,  38.52,  -5.24,  -0.68,
                              15.44,  59.85,  -1.87,  39.76,  24.77,  -0.88,
                               9.62,   3.24,  -2.67,  11.02,  36.02,  -2.54,
                               9.63,   2.82,  -3.16,   7.82,   2.14,  -3.67,
                              11.02,  36.09,  -2.61,   9.51,  41.19,  -2.94,
                              36.03,   5.33,  -0.66,  36.46,  14.9 ,  -0.74,
                              35.94,   5.43,  -0.72,  37.81,   5.26,  -0.73,
                              36.42,  14.72,  -0.83,  38.16,  14.68,  -0.85,
                              20.93,  13.26,  -1.33,  21.25,   9.66,  -1.15,
                              21.21,   9.75,  -1.6 ,  22.18,   7.95,  -1.19,
                              22.21,   7.94,  -1.17,  23.43,   7.82,  -1.11,
                              23.38,   7.55,  -1.18,  24.42,   9.47,  -1.16,
                              24.43,   9.37,  -1.25,  24.9 ,  11.72,  -0.98,
                              24.89,  11.73,  -1.2 ,  24.68,  14.54,  -1.05,
                              24.7 ,  14.85,  -1.06,  23.85,  16.63,  -1.1 ,
                              23.82,  16.53,  -1.14,  22.61,  17.14,  -1.32,
                              22.65,  17.53,  -1.23,  21.5 ,  16.19,  -1.34,
                              21.49,  15.92,  -1.32,  20.95,  13.07,  -1.32 };

    int lines_no = sizeof(lines)/sizeof(lines[0])/6;

    float retval[35];

    localize_agent_pose(true, // lfoot_touch
            true, // rfoot_touch
            feet_contact,
            true, // ball_seen
            ball_pos, 
            me_pos,
            landmarks,
            lines,
            lines_no,
            retval
    );                         

	return 0;
}
