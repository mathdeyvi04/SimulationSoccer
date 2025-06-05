#include "LocalizerV2.h"
#include <cstdio>
static World& mundo_existente = Singular<World>::obter_instancia();

///////////////////////////////////////////////////////////////////////////////////////////////////////////

int
LocalizerV2::estimar_erro_posicional(
    const Vetor3D  posicao_estimativa,
    const Vetor3D& posicao_real,
    	  double    error_placeholder[]
){
	/*
	  Descrição:
	  Calcula e acumula as diferenças de erro entre a posição estimada do agente e a posição real informada pelo servidor.
	  Para cada chamada, incrementa os valores dos erros em x, y, z, o erro euclidiano 2D e 3D, além de seus quadrados, no vetor de saída.
	  Retorna 1 se o cálculo foi realizado, ou 0 caso a posição de referência do servidor seja (0,0,0).

	  Parâmetros:
	    - posicao_estimativa: posição estimada do agente.
	    - posicao_real: posição real fornecida pelo servidor.
	    - error_placeholder: vetor onde os resultados dos erros serão acumulados.

	  Retorno:
	    - 1 se o erro foi computado e acumulado; 0 caso contrário.
	*/
    if (mundo_existente.pos_abs_agente_cartesiana_cheat == Vetor3D(0,0,0) ) { return 0; }

    double erro_x  = posicao_estimativa.x - posicao_real.x;
    double erro_y  = posicao_estimativa.y - posicao_real.y;
    double erro_z  = posicao_estimativa.z - posicao_real.z;

    double erro_x2 = erro_x * erro_x;
    double erro_y2 = erro_y * erro_y;
    double erro_z2 = erro_z * erro_z;

    error_placeholder[0] += erro_x;
    error_placeholder[1] += erro_y;
    error_placeholder[2] += erro_z;

    double erro_quadratico_2d = erro_x2 + erro_y2;
    error_placeholder[3] += sqrt(erro_quadratico_2d);
    error_placeholder[4] += erro_quadratico_2d;

    double erro_quadratico_3d = erro_x2 + erro_y2 + erro_z2;
    error_placeholder[5] += sqrt(erro_quadratico_3d);
    error_placeholder[6] += erro_quadratico_3d;

    return 1;
}

bool 
LocalizerV2::fine_tune(
    float initial_angle, 
    float initial_x, 
    float initial_y
){
    /*
    Descrição:
        Aplica um ajuste fino diretamente na matriz _Head_to_Field_Prelim, 
        refinando o alinhamento do mapa por etapas.
        
        O processo consiste em:
        1. Ajustar o encaixe do mapa com base nos parâmetros iniciais.
        2. Identificar segmentos de linha e seus pontos finais.
        3. Realizar um novo ajuste fino utilizando marcadores conhecidos.
        Após cada etapa, são coletadas estatísticas sobre o erro de posicionamento.

    Parâmetros:
        - initial_angle: ângulo inicial de Xvec em torno de Zvec.
        - initial_x: translação inicial no eixo x.
        - initial_y: translação inicial no eixo y.

    Retorno:
        - true  — se o ajuste fino foi realizado com sucesso.
        - false — se falhou em alguma etapa do processo.
    */

    RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

    // Estatísticas antes do ajuste fino
    counter_fineTune += estimar_erro_posicional(
                                                Vetor3D(initial_x, initial_y, _Head_to_Field_Prelim.obter(11)),
                                                mundo_existente.pos_abs_agente_cartesiana_cheat,
                                                errorSum_fineTune_before
                                                );

    // Ajuste fino, alterando diretamente os parâmetros iniciais
    if( !fine_tune_aux(initial_angle, initial_x, initial_y, false) ) { return false; }
    
    // Estatísticas para o primeiro ajuste fino
    estimar_erro_posicional(
                            Vetor3D(initial_x, initial_y, _Head_to_Field_Prelim.obter(11)),
                            mundo_existente.pos_abs_agente_cartesiana_cheat,
                            errorSum_fineTune_euclidianDist
                            );

    // Identifica novos marcadores
    campo_existente.atualizar_marcadores_por_transformacao(_Head_to_Field_Prelim);

    // Ajuste fino probabilístico
    fine_tune_aux(initial_angle, initial_x, initial_y, true);

    // Estatísticas para o segundo ajuste fino
    estimar_erro_posicional(
                           _Head_to_Field_Prelim.obter_vetor_de_translacao(),
                           mundo_existente.pos_abs_agente_cartesiana_cheat,
                           errorSum_fineTune_probabilistic
                           );

    // Atualiza a posição absoluta dos marcadores desconhecidos com base na matriz refinada de transformação
    campo_existente.atualizar_marcadores_desconhecidos_por_transformacao(_Head_to_Field_Prelim);

    return true;
}

void 
LocalizerV2::run(){
    /*
    Descrição:
        Executa o ciclo incipal de localização, computando a posição e orientação 3D do agente no campo.
        
        O método segue um fluxo de processamento dividido em etapas:
        - Reinicializa variáveis e matriz de transformação preliminar.
        - Atualiza as coleções de linhas e marcos visíveis.
        - Verifica se há dados suficientes para prosseguir (pelo menos dois elementos entre linhas e marcos).
        - Determina o vetor de orientação do eixo Z.
        - Encontra a rotação e translação XY, utilizando um método de estimação ou adivinhação conforme necessário.
        - Atualiza as variáveis públicas e o estado interno.
        - Coleta estatísticas sobre a posição da bola, caso ela seja visível.

    Parâmetros:
        None

    Retorno:
        Resultado da localização é armazenado em variáveis da classe e do ambiente.
    */

    RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

    atualizar_estado_do_sistema(RUNNING);

    // FLUXO DE TRABALHO: 0

    _is_uptodate = false;
    _is_head_z_uptodate = false;
    _steps_since_last_update++;

    // reseta a matriz de transformação preliminar
    resetar_matriz_preliminar(); 

    campo_existente.atualizar_marcadores(); 

    int lines_no     = campo_existente.list_segments.size();
    int landmarks_no = campo_existente.list_landmarks.size();

    if ((landmarks_no == 0 && lines_no < 2) || (lines_no == 0)) {

        atualizar_estado_do_sistema( (lines_no == 0 && landmarks_no == 0) ? BLIND : MINFAIL );
        return;
    }

    // FLUXO DE TRABALHO: 1-2

    if( !calcular_orientacao_eixo_z() ){ return; }

    // FLUXO DE TRABALHO: 3-4

    if( !(  landmarks_no > 1 ? calcular_translacao_rotacao_xy() : estimar_translacao_rotacao_xy()  ) ){ return; }

    // Atualiza variáveis públicas
    commit_system();

    atualizar_estado_do_sistema(DONE);

    printf("--------------------------------------");

    // Estatísticas da posição da bola
    if(
        mundo_existente.bola_detectada
    ){
        // counter_ball é definido no header
        counter_ball += estimar_erro_posicional(
                                                _Head_to_Field_Prelim * mundo_existente.pos_rel_bola_cartesiana,
                                                mundo_existente.pos_abs_bola_cartesiana_cheat,
                                                errorSum_ball
                                                );
    }

    // reportar_situacao(); // descomente para habilitar o relatório (erros médios + análise da solução)
}
