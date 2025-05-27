#include <iostream>
#include "Ruido_de_Campo.h"
#include "World.h"  // Incluirá AlgLin.h
#include "LocalizerV2.h"  // Já incluirá RobovizField.h
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cstdio>

namespace py = pybind11;
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

py::array_t<float>
localize_agent_pose(
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

    world.se_bola_esta_visivel = ball_seen;

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
    /*
    Tive um super erro aqui. Por exemplo, ao passar um array de floats
    para a função, por exemplo, 120 floats, teremos apenas 20 linhas,
    pois para cada linha haverá 6 floats.

    O código original não levava isso em consideração e acessava
    elementos indefinidos.
    */
    int lines_len = buffer_das_linhas.shape[0] / 6;  
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
    
    //// Realizamos o super algoritmo. //// 

    loc.run(); 
    
    // Preparamos data para retornar.  ////
    
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

void report_calculation_status(bool for_debugging = false){

    loc.reportar_situacao(for_debugging);
    return;
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


using namespace pybind11::literals; 

// Vamos manter o nome em honra aos autores
PYBIND11_MODULE(
    localization,
    m
) {                      
    m.doc() = R"pbdoc(
    This module provides a set of functions that enable real-time
    estimation and visualization of an agent's 3D pose based on 
    visual information, including lines, field markers, and object 
    detections. It integrates low-level localization logic with 
    high-level visualization and data inspection tools for debugging
    and analysis.
    )pbdoc";

    m.def(
        "localize_agent_pose",
        &localize_agent_pose, 
        R"pbdoc(
        Description:
            This function estimates the 3D pose of an agent's head in the environment using
            visual and geometric cues such as field lines, ground markers, and goalposts.

            It follows a multi-stage process involving minimum data requirements, orientation 
            vector estimation (via SVD or relative geometry), Z-coordinate translation, and 
            full transformation matrix assembly. 

            It includes logic for ambiguity resolution, probabilistic validation, and refinement 
            based on prior position and measurement quality. When visual input is partial or
            ambiguous, the system may choose not to update the global transformation matrix, 
            preserving consistency. The resulting head pose is always updated, while the world 
            matrix is only updated under strict confidence conditions.

        Parameters:
            - if_lfoot_touch_the_ground: Whether the agent's **left foot** is currently in contact with the ground.
            - if_rfoot_touch_the_ground: Whether the agent's **right foot** is currently in contact with the ground.
            - relative_position_of_feet_contact: The estimated **position(s)** where the agent's feet make contact with the ground, relative to the head or body.
            - if_ball_seen: Whether the **ball is currently visible** in the agent's field of view.
            - ball_pos: The **3D position** of the ball in the agent's relative coordinate frame.
            - me_pos: The current **estimated pose** (position and orientation) of the agent in the world.
            - landmarks: A set of **ground reference markers** (e.g., field corners, penalty marks) visible in the scene.
            - lines: A set of **line segments** detected from the field markings used for geometric pose estimation.

        Returns:
            A flat array of 35 float values containing:
            - [0–15]   : The 4x4 transformation matrix from head to world coordinates (row-major order).
            - [16–31]  : The 4x4 inverse transformation matrix from world to head coordinates.
            - [32]     : Flag indicating whether the pose is ready to update the global transformation matrix (1.0 or 0.0).
            - [33]     : The estimated Z-coordinate (height) of the agent's head.
            - [34]     : Flag indicating whether the head Z estimation is considered valid (1.0 or 0.0).
        )pbdoc",
        "if_lfoot_touch_the_ground"_a,
        "if_rfoot_touch_the_ground"_a,
        "relative_position_of_feet_contact"_a,
        "if_ball_seen"_a,
        "ball_pos"_a,
        "me_pos"_a,
        "landmarks"_a,
        "lines"_a
    );

    m.def(
        "expose_runtime_data",
        &expose_runtime_data, 
        R"pbdoc(
        Description:
            Exposes internal world state and calculated values to Python
            in real-time for inspection or debugging purposes. Useful for
            monitoring agent state during runtime.

        Parameters:
            None

        Return:
            None, but the visual information given.
        )pbdoc"
    );

    m.def(
        "report_calculation_status",
        &report_calculation_status,
        R"pbdoc(
        Description:
            Prints diagnostic information about the most recent pose calculation, 
            including convergence status, error metrics, and internal decisions 
            made by the optimizer.

        Parameters:
            None

        Return:
            None, but the visual information given.
        )pbdoc"
    );

    m.def(
        "illustrator", 
        &illustrator, 
        R"pbdoc(
        Description:
            Triggers visualization of currently detected field elements
            (lines, markers, goalposts) and pose estimation results, 
            typically using a debug rendering layer or graphical interface.

        Parameters:
            - is_right_side: if field side is switch

        Return:
            None, but the visual lines and markers.
        )pbdoc",
        "is_right_side"_a
    );
}
