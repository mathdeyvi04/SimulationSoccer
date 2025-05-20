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

py::array_t<float> compute(
            bool lfoot_touch, bool rfoot_touch, 
            py::array_t<double> feet_contact,
            bool ball_seen, py::array_t<double> ball_pos,
            py::array_t<double> me_pos,
            py::array_t<double> landmarks,
            py::array_t<double> lines){

    // ================================================= 1. Parse data
    
    static World &world = SWorld::getInstance();
    world.foot_touch[0] = lfoot_touch;
    world.foot_touch[1] = rfoot_touch;

    //Structure of feet_contact {lfoot_contact_pt, rfoot_contact_pt, lfoot_contact_rel_pos, rfoot_contact_rel_pos}

    py::buffer_info feet_contact_buf = feet_contact.request();
    double *feet_contact_ptr = (double *) feet_contact_buf.ptr;
    world.foot_contact_rel_pos[0].x = feet_contact_ptr[0];
    world.foot_contact_rel_pos[0].y = feet_contact_ptr[1];
    world.foot_contact_rel_pos[0].z = feet_contact_ptr[2];
    world.foot_contact_rel_pos[1].x = feet_contact_ptr[3];
    world.foot_contact_rel_pos[1].y = feet_contact_ptr[4];
    world.foot_contact_rel_pos[1].z = feet_contact_ptr[5];

    world.ball_seen = ball_seen;

    //Structure of ball_pos {ball_rel_pos_cart, ball_cheat_abs_cart_pos}

    py::buffer_info ball_pos_buf = ball_pos.request();
    double *ball_pos_ptr = (double *) ball_pos_buf.ptr;
    world.ball_rel_pos_cart.x = ball_pos_ptr[0];
    world.ball_rel_pos_cart.y = ball_pos_ptr[1];
    world.ball_rel_pos_cart.z = ball_pos_ptr[2];
    world.ball_cheat_abs_cart_pos.x = ball_pos_ptr[3];
    world.ball_cheat_abs_cart_pos.y = ball_pos_ptr[4];
    world.ball_cheat_abs_cart_pos.z = ball_pos_ptr[5];
    
    py::buffer_info me_pos_buf = me_pos.request();
    double *me_pos_ptr = (double *) me_pos_buf.ptr;
    world.my_cheat_abs_cart_pos.x = me_pos_ptr[0];
    world.my_cheat_abs_cart_pos.y = me_pos_ptr[1];
    world.my_cheat_abs_cart_pos.z = me_pos_ptr[2];

    py::buffer_info landmarks_buf = landmarks.request();
    double *landmarks_ptr = (double *) landmarks_buf.ptr;

    for(int i=0; i<8; i++){
        world.landmark[i].seen = (bool) landmarks_ptr[0];
        world.landmark[i].isCorner = (bool) landmarks_ptr[1];
        world.landmark[i].pos.x = landmarks_ptr[2];
        world.landmark[i].pos.y = landmarks_ptr[3];
        world.landmark[i].pos.z = landmarks_ptr[4];
        world.landmark[i].rel_pos.x = landmarks_ptr[5];
        world.landmark[i].rel_pos.y = landmarks_ptr[6];
        world.landmark[i].rel_pos.z = landmarks_ptr[7];
        landmarks_ptr += 8;
    }

    py::buffer_info lines_buf = lines.request();
    int lines_len = lines_buf.shape[0];
    double *lines_ptr = (double *) lines_buf.ptr;
    world.lines_polar.clear();

    for(int i=0; i<lines_len; i++){
        Vector3f s(lines_ptr[0],lines_ptr[1],lines_ptr[2]);
        Vector3f e(lines_ptr[3],lines_ptr[4],lines_ptr[5]);
        world.lines_polar.emplace_back(s, e); 
        lines_ptr += 6;
    }
    
    // ================================================= 2. Compute 6D pose

    loc.run(); 
    
    // ================================================= 3. Prepare data to return
    
    py::array_t<float> retval = py::array_t<float>(35); //allocate
    py::buffer_info buff = retval.request();
    float *ptr = (float *) buff.ptr;

    for(int i=0; i<16; i++){
        ptr[i] = loc.headTofieldTransform.content[i];
    }
    ptr += 16;
    for(int i=0; i<16; i++){
        ptr[i] = loc.fieldToheadTransform.content[i];
    }
    ptr += 16;

    ptr[0] = (float) loc.is_uptodate;
    ptr[1] = loc.head_z;
    ptr[2] = (float) loc.is_head_z_uptodate;


    return retval;
}

void print_report(){
    loc.print_report();
}

void draw_visible_elements(bool is_right_side){
    Field& fd = SField::getInstance();
    fd.draw_visible(loc.headTofieldTransform, is_right_side);
}


using namespace pybind11::literals; //to add informative argument names as -> "argname"_a

PYBIND11_MODULE(localization, m) { //the python module name, m is the interface to create bindings
    m.doc() = "Probabilistic 6D localization algorithm"; // optional module docstring

    //optional arguments names
    m.def("compute", &compute, "Compute the 6D pose based on visual information and return transformation matrices and other relevant data",
        "lfoot_touch"_a,
        "rfoot_touch"_a,
        "feet_contact"_a,
        "ball_seen"_a,
        "ball_pos"_a,
        "me_pos"_a,
        "landmarks"_a,
        "lines"_a);

    m.def("print_python_data", &print_python_data, "Print data received from Python");
    m.def("print_report", &print_report, "Print localization report");
    m.def("draw_visible_elements", &draw_visible_elements, "Draw all visible elements in RoboViz", "is_right_side"_a);
    
}