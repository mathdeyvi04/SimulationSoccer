#include <iostream>
#include "Ruido_de_Campo.h"
#include "World.h"
#include "LocalizerV2.h"  // Já incluirá RobovizField

using namespace std;

static LocalizerV2& loc = Singular<LocalizerV2>::obter_instancia();

void expose_runtime_data(){
    /*
    Função que apresentará informações que serão utilizadas no ambiente C++.
    */

    static World &world = Singular<World>::obter_instancia();

    cout << "Foot Is Touching: "      << world.se_pe_esta_tocando[0] << " " << world.se_pe_esta_tocando[1] << endl;
    cout << "LFoot Pos_Rel Contato: " << world.pos_rel_do_ponto_de_contato[0].x << ", " << world.pos_rel_do_ponto_de_contato[0].y << ", " << world.pos_rel_do_ponto_de_contato[0].z << endl;
    cout << "RFoot Pos_Rel Contato: " << world.pos_rel_do_ponto_de_contato[1].x << ", " << world.pos_rel_do_ponto_de_contato[1].y << ", " << world.pos_rel_do_ponto_de_contato[1].z << endl;
    cout << "If Ball Seen: "          << world.se_bola_esta_visivel << endl;
    cout << "Ball Pos Abs Cheat: "    << world.pos_abs_da_bola_cart_cheat.x << ", " << world.pos_abs_da_bola_cart_cheat.y << ", " << world.pos_abs_da_bola_cart_cheat.z << endl;
    cout << "Eu, Robo:   "            << world.pos_abs_do_robo_cart_cheat.x << ", " << world.pos_abs_do_robo_cart_cheat.y << ", " << world.pos_abs_do_robo_cart_cheat.z << endl;
               
    for(int i=0; i<8; i++){

        cout << "Marcador de Chão " << i << ": " <<
        world.marcadores_de_chao[i].se_esta_visivel << " " <<
        world.marcadores_de_chao[i].se_eh_canto   << " " <<
        world.marcadores_de_chao[i].pos_abs.x     << " " <<
        world.marcadores_de_chao[i].pos_abs.y     << " " <<
        world.marcadores_de_chao[i].pos_abs.z     << " " <<
        world.marcadores_de_chao[i].pos_rel_esf.x << " " <<
        world.marcadores_de_chao[i].pos_rel_esf.y << " " <<
        world.marcadores_de_chao[i].pos_rel_esf.z << endl;
    }

    for(size_t i=0; i<world.linhas_esfericas.size(); i++){

        cout << "Linha Em Coordenadas Esfericas " << i << ": " <<
        world.linhas_esfericas[i].inicio.x << " " << 
        world.linhas_esfericas[i].inicio.y << " " << 
        world.linhas_esfericas[i].inicio.z << " " << 
        world.linhas_esfericas[i].final.x << " " << 
        world.linhas_esfericas[i].final.y << " " << 
        world.linhas_esfericas[i].final.z << endl;
    }
}

float *localize_agent_pose(
    bool lfoot_touch, bool rfoot_touch, 
    double feet_contact[],
    bool ball_seen, double ball_pos[],
    double me_pos[],
    double landmarks[],
    double lines[],
    int lines_no,
    float *retval  // Esse parâmetro é apenas para conseguirmos retornar sem alocar.
){

    // ================================================= 1. Traduzir informações para o Python
    
    static World &world = Singular<World>::obter_instancia();
    world.se_pe_esta_tocando[0] = lfoot_touch;
    world.se_pe_esta_tocando[1] = rfoot_touch;

    world.pos_rel_do_ponto_de_contato[0].x = feet_contact[0];
    world.pos_rel_do_ponto_de_contato[0].y = feet_contact[1];
    world.pos_rel_do_ponto_de_contato[0].z = feet_contact[2];
    world.pos_rel_do_ponto_de_contato[1].x = feet_contact[3];
    world.pos_rel_do_ponto_de_contato[1].y = feet_contact[4];
    world.pos_rel_do_ponto_de_contato[1].z = feet_contact[5];

    world.se_bola_esta_visivel = ball_seen;

    world.pos_rel_da_bola_cart.x       = ball_pos[0];
    world.pos_rel_da_bola_cart.y       = ball_pos[1];
    world.pos_rel_da_bola_cart.z       = ball_pos[2];
    world.pos_abs_da_bola_cart_cheat.x = ball_pos[3];
    world.pos_abs_da_bola_cart_cheat.y = ball_pos[4];
    world.pos_abs_da_bola_cart_cheat.z = ball_pos[5];
    
    world.pos_abs_do_robo_cart_cheat.x = me_pos[0];
    world.pos_abs_do_robo_cart_cheat.y = me_pos[1];
    world.pos_abs_do_robo_cart_cheat.z = me_pos[2];

    for(int i=0; i<8; i++){

        world.marcadores_de_chao[i].se_esta_visivel = (bool) landmarks[0];
        world.marcadores_de_chao[i].se_eh_canto     = (bool) landmarks[1];
        world.marcadores_de_chao[i].pos_abs.x       = landmarks[2];
        world.marcadores_de_chao[i].pos_abs.y       = landmarks[3];
        world.marcadores_de_chao[i].pos_abs.z       = landmarks[4];
        world.marcadores_de_chao[i].pos_rel_esf.x   = landmarks[5];
        world.marcadores_de_chao[i].pos_rel_esf.y   = landmarks[6];
        world.marcadores_de_chao[i].pos_rel_esf.z   = landmarks[7];
        landmarks += 8;  // Aritmética de ponteiro é linda demais
    }


    world.linhas_esfericas.clear();

    for(int i=0; i<lines_no; i++){

        Vetor3D inicio(lines[0],lines[1],lines[2]);
        Vetor3D final(lines[3],lines[4],lines[5]);
        world.linhas_esfericas.emplace_back(inicio, final); 
        lines += 6;
    }

    expose_runtime_data();
    
    // ================================================= 2. Aplicar Algoritmo de LocalizerV2

    loc.run(); 

    loc.reportar_situacao();
    
    // ================================================= 3. Retornar valores para Python
    
    float *ptr = retval;

    // Observe que não há como entregarmos uma matriz, temos que devolver como um vetor!
    for(int i=0; i<16; i++){

        ptr[i] = loc.Head_to_Field_Transform.conteudo[i];
    }
    ptr += 16;

    for(int i=0; i<16; i++){

        ptr[i] = loc.Head_to_Field_Transform.conteudo[i];
    }
    ptr += 16;

    ptr[0] = (float) loc.se_esta_pronta_para_atualizacao;
    ptr[1] = loc.head_z;
    ptr[2] = (float) loc.se_head_z_esta_pronta_para_atualizacao;

    return retval;
}

void report_calculation_status(){
    loc.reportar_situacao();
}

// Não ache estranho, é apenas ilustrador em inglês.
void illustrator(bool is_right_side){

    RobovizField& campo_existente = Singular<RobovizField>::obter_instancia();

    campo_existente.ilustrador(
                                loc.Head_to_Field_Transform,
                                (is_right_side) ? -1 : 1
                              );

    return;
}

int main(){

    /* Esses valores são os valores padrão do início de jogo, não se assuste. */
    
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


    cout << "\n\nCompilado com sucesso.\n\n";

}