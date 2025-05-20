#include <iostream>
#include "Ruido_de_Campo.h"
#include "World.h"  // Incluirá AlgLin.h
#include "LocalizerV2.h"  // Já incluirá RobovizField.h
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace std;

static LocalizerV2& loc = Singular<LocalizerV2>::obter_instancia();

void apresentar_dados_em_python(){
    /*
    Função que levará informações que serão utilizadas no ambiente C++.
    */

    static World &world = Singular<World>::obter_instancia();

    cout << "Foot Is Touching: "      << world.se_pe_esta_tocando[0] << " " << world.se_pe_esta_tocando[1] << endl;
    cout << "LFoot Pos_Rel Contato: " << world.pos_rel_do_ponto_de_contato[0].x << ", " << world.pos_rel_do_ponto_de_contato[0].y << ", " << world.pos_rel_do_ponto_de_contato[0].z << endl;
    cout << "RFoot Pos_Rel Contato: " << world.pos_rel_do_ponto_de_contato[1].x << ", " << world.pos_rel_do_ponto_de_contato[1].y << ", " << world.pos_rel_do_ponto_de_contato[1].z << endl;
    cout << "Bola Vista: "            << world.se_bola_esta_visivel << endl;
    cout << "Bola Cheat: "            << world.pos_abs_da_bola_cart_cheat.x << ", " << world.pos_abs_da_bola_cart_cheat.y << ", " << world.pos_abs_da_bola_cart_cheat.z << endl;
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

    for(int i=0; i<world.linhas_esfericas.size(); i++){

        cout << "Linha Em Coordenadas Esfericas " << i << ": " <<
        world.linhas_esfericas[i].inicio.x << " " << 
        world.linhas_esfericas[i].inicio.y << " " << 
        world.linhas_esfericas[i].inicio.z << " " << 
        world.linhas_esfericas[i].final.x << " " << 
        world.linhas_esfericas[i].final.y << " " << 
        world.linhas_esfericas[i].final.z << endl;
    }
}

py::array_t<float>
compute(
            // Não vou renomear estes pq já estão bons.
            bool lfoot_touch,
            bool rfoot_touch, 
            py::array_t<double> feet_contact,
            bool ball_seen,
            py::array_t<double> ball_pos,
            py::array_t<double> me_pos,
            py::array_t<double> landmarks,
            py::array_t<double> lines
){

    // Traduzir informações
    
    static World &world = Singular<World>::obter_instancia();
    world.se_pe_esta_tocando[0] = lfoot_touch;
    world.se_pe_esta_tocando[1] = rfoot_touch;

    /*
    Receberemos do python a estrutura feet_contact:
        {lfoot_contact_pt, rfoot_contact_pt, lfoot_contact_rel_pos, rfoot_contact_rel_pos}
    */
    py::buffer_info buffer_pos_pes = feet_contact.request();
    double *feet_contact_ptr = (double *) buffer_pos_pes.ptr;  // ponteiro para buffer

    world.pos_rel_do_ponto_de_contato[0].x = feet_contact_ptr[0];
    world.pos_rel_do_ponto_de_contato[0].y = feet_contact_ptr[1];
    world.pos_rel_do_ponto_de_contato[0].z = feet_contact_ptr[2];
    world.pos_rel_do_ponto_de_contato[1].x = feet_contact_ptr[3];
    world.pos_rel_do_ponto_de_contato[1].y = feet_contact_ptr[4];
    world.pos_rel_do_ponto_de_contato[1].z = feet_contact_ptr[5];

    world.se_bola_esta_visivel = se_bola_esta_visivel;

    // Receberemos do python a estrutura ball_pos:
    //   {pos_rel_da_bola_cart, ball_cheat_abs_cart_pos}

    py::buffer_info buffer_pos_ball = ball_pos.request();
    double *ball_pos_ptr = (double *) buffer_pos_ball.ptr;

    world.pos_rel_da_bola_cart.x       = ball_pos_ptr[0];
    world.pos_rel_da_bola_cart.y       = ball_pos_ptr[1];
    world.pos_rel_da_bola_cart.z       = ball_pos_ptr[2];
    world.pos_abs_da_bola_cart_cheat.x = ball_pos_ptr[3];
    world.pos_abs_da_bola_cart_cheat.y = ball_pos_ptr[4];
    world.pos_abs_da_bola_cart_cheat.z = ball_pos_ptr[5];
    
    py::buffer_info buffer_robo = me_pos.request();
    double *me_pos_ptr = (double *) buffer_robo.ptr;

    world.pos_abs_do_robo_cart_cheat.x = me_pos_ptr[0];
    world.pos_abs_do_robo_cart_cheat.y = me_pos_ptr[1];
    world.pos_abs_do_robo_cart_cheat.z = me_pos_ptr[2];

    py::buffer_info landmarks_buf = landmarks.request();
    double *landmarks_ptr = (double *) landmarks_buf.ptr;

    for(
        int i=0; 
            i<8; 
            i++
    ){

        world.marcadores_de_chao[i].se_esta_visivel = (bool) landmarks_ptr[0];
        world.marcadores_de_chao[i].se_eh_canto   = (bool) landmarks_ptr[1];
        world.marcadores_de_chao[i].pos_abs.x     = landmarks_ptr[2];
        world.marcadores_de_chao[i].pos_abs.y     = landmarks_ptr[3];
        world.marcadores_de_chao[i].pos_abs.z     = landmarks_ptr[4];
        world.marcadores_de_chao[i].pos_rel_esf.x = landmarks_ptr[5];
        world.marcadores_de_chao[i].pos_rel_esf.y = landmarks_ptr[6];
        world.marcadores_de_chao[i].pos_rel_esf.z = landmarks_ptr[7];

        // São 8 marcadores, logo devemos realizar esse loop 8 vezes
        // Pegando informações de cada um
        landmarks_ptr += 8; 
    }

    py::buffer_info buffer_das_linhas = lines.request();
    int lines_len = buffer_das_linhas.shape[0];  // Para sabermos quantas linhas estamos vendo.

    double *lines_ptr = (double *) buffer_das_linhas.ptr;
    world.linhas_esfericas.clear();

    for(
        int i=0; 
            i<lines_len; 
            i++
    ){

        world.linhas_esfericas.emplace_back(
            Vetor3D( lines_ptr[0],lines_ptr[1],lines_ptr[2] ),  // inicio da linhas
            Vetor3D( lines_ptr[3],lines_ptr[4],lines_ptr[5] )   // final 
        ); 

        lines_ptr += 6;  // Para cada linha
    }
    
    // Realizamos o super algoritmo.

    loc.run(); 
    
    // Preparamos data para retornar.
    
    // Prealocamos tudo que iremos retornar
    py::array_t<float> retval = py::array_t<float>(35); 
    py::buffer_info buffer = retval.request();
    float *ptr = (float *) buffer.ptr;  // Manipulação de ponteiro é algo realmente lindo.

    for(int i=0; i<16; i++){
        ptr[i] = loc.Head_to_Field_Transform.conteudo[i];
    }
    ptr += 16;

    for(int i=0; i<16; i++){
        ptr[i] = loc.Field_to_Head_Transform.conteudo[i];
    }
    ptr += 16;

    ptr[0] = (float) loc.se_esta_pronta_para_atualizacao;
    ptr[1] = loc.head_z;
    ptr[2] = (float) loc.se_head_z_esta_pronta_para_atualizacao;

    return retval;
}

void reportar_situacao(){
    loc.reportar_situacao();
}

void desenhar_elementos_visiveis(bool is_right_side){

    RobovizField& fd = Singular<RobovizField>::obter_instancia();
    fd.desenhar_visiveis(loc.Head_to_Field_Transform, is_right_side);

    return;
}


using namespace pybind11::literals; 

// Vamos manter o nome em honra aos autores
PYBIND11_MODULE(localization, m) {                      
    m.doc() = "Super Algoritmo de Localização 6D baseado em Probabilidades"; // optional module docstring

    m.def(
        "aplicar_localizer", &compute, "Calcular a posição do robô baseado em informações virtuais e retornar dados relevantes, como as matrizes de transformação. Cada argumento tem uma função crucial, atente-se.",
        "lfoot_touch"_a,
        "rfoot_touch"_a,
        "feet_contact"_a,
        "ball_seen"_a,
        "ball_pos"_a,
        "me_pos"_a,
        "landmarks"_a,
        "lines"_a);

    m.def(
        "apresentar_dados_recebidos_do_python",
        &apresentar_dados_em_python, 
        "Apresenta dados recebidos pelo C++ a partir do Python."
    );

    m.def(
        "reportar_situacao",
        &reportar_situacao,
        "Reportar informações relevantes sobre os processos calculados pelo algoritmo de localização."
    );

    m.def(
        "desenhar_elementos_visiveis", 
        &desenhar_elementos_visiveis, 
        "Desenhar todos os elementos vísiveis no Roboviz.", 
        "is_right_side"_a
    );
    
}