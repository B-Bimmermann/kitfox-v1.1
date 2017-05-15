#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "energylib_intsim.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_INTSIM;

energylib_intsim::energylib_intsim(pseudo_component_t *PseudoComponent) :
    energy_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_INTSIM),
    chip(NULL),
    param(NULL),
    energy_scaling(1.0),
    area_scaling(1.0),
    scaling(1.0),
    energy_model(KITFOX_INTSIM_ENERGY_MODEL_UNKNOWN)
{}

energylib_intsim::~energylib_intsim() {
    chip->finalize();
    delete chip;
    delete param;
}

void energylib_intsim::initialize() {
    chip = new intsim_chip_t();
    param = new intsim_param_t();
    
    param->setup(true); // init with default
    
    try {
        if(pseudo_component->exists_in_library("energy_scaling",true))
            energy_scaling = pseudo_component->lookup_in_library("energy_scaling",true);
        else
            energy_scaling = 1.0;
            
        if(pseudo_component->exists_in_library("area_scaling",true))
            area_scaling = pseudo_component->lookup_in_library("area_scaling",true);
        else
            area_scaling = 1.0;
            
        // Scaling factor applies to area and leakage energy.
        // Scaling factor is used for duplicated components.
        // For instance, if there are two components to be modeled,
        // they can be configured as component0 and component1,
        // or simply component with scaling = 2 such that area and leakage power are doubled.
        // Note that dynamic energy is not scaled, otherwise 1 counter access represents
        // simultaneous accesses to both components which may not be the intended case.
        if(pseudo_component->exists_in_library("scaling",true))
            scaling = pseudo_component->lookup_in_library("scaling",true);
        else
            scaling = 1.0;
            
        param->f = pseudo_component->lookup_in_library("clock_frequency",true);
        param->Vdd = pseudo_component->lookup_in_library("voltage",true);
        param->temp = pseudo_component->lookup_in_library("temperature",true);
        param->F = pseudo_component->lookup_in_library("feature_size",true);
        param->A = pseudo_component->lookup_in_library("area");
        param->ngates = pseudo_component->lookup_in_library("num_gates",true);
        
        if(pseudo_component->exists_in_library("energy_model")) {
            const char *energy_model_str = pseudo_component->lookup_in_library("energy_model");
            if(!strcasecmp(energy_model_str,"default"))
                energy_model = KITFOX_INTSIM_DEFAULT;
            else if(!strcasecmp(energy_model_str,"logic_gate"))
                energy_model = KITFOX_INTSIM_LOGIC_GATE;
            else if(!strcasecmp(energy_model_str,"repeater"))
                energy_model = KITFOX_INTSIM_REPEATER;
            else if(!strcasecmp(energy_model_str,"interconnect"))
                energy_model = KITFOX_INTSIM_INTERCONNECT;
            else if(!strcasecmp(energy_model_str,"clock"))
                energy_model = KITFOX_INTSIM_CLOCK;
            else {
                pseudo_component->kitfox->error("intsim","%s.library.energy_model = %s is unknown",pseudo_component->name.c_str(),energy_model_str);
            }
        }
        else
            energy_model = KITFOX_INTSIM_DEFAULT;
            
        param->Vt = pseudo_component->lookup_in_library("threshold_voltage",true);
        param->ncp = pseudo_component->lookup_in_library("critical_path_depth",true);
        if(pseudo_component->exists_in_library("rents_constant_k",true))
            param->k = pseudo_component->lookup_in_library("rents_constant_k",true);
        if(pseudo_component->exists_in_library("rents_constant_p",true))
            param->p = pseudo_component->lookup_in_library("rents_constant_p",true);
        param->a = pseudo_component->lookup_in_library("activity_factor",true);
        if(pseudo_component->exists_in_library("tdp_activity_factor",true))
            param->tdp_a = pseudo_component->lookup_in_library("tdp_activity_factor",true);
        else
            param->tdp_a = 0.2;
        if(pseudo_component->exists_in_library("voltage_spec",true))
            param->Vdd_spec = pseudo_component->lookup_in_library("voltage_spec",true);
        else
            param->Vdd_spec = param->Vdd;
        if(pseudo_component->exists_in_library("threshold_voltage_spec",true))
            param->Vt_spec = pseudo_component->lookup_in_library("threshold_voltage_spec",true);
        else
            param->Vt_spec = param->Vt;
        param->tox = pseudo_component->lookup_in_library("oxide_thickness",true);
        if(pseudo_component->exists_in_library("alpha_power_law",true))
            param->alpha = pseudo_component->lookup_in_library("alpha_power_law",true);
        param->drive_p_div_n = pseudo_component->lookup_in_library("pn_ratio",true);
        param->subvtslope_spec = pseudo_component->lookup_in_library("subvtslope_spec",true);
        if(pseudo_component->exists_in_library("dielectric_permitivity",true))
            param->er = pseudo_component->lookup_in_library("dielectric_permitivity",true);
        if(pseudo_component->exists_in_library("copper_resistivity",true))
            param->rho = pseudo_component->lookup_in_library("copper_resistivity",true);
        param->ar = pseudo_component->lookup_in_library("wiring_aspect_ratio",true);
        param->R_coeff = pseudo_component->lookup_in_library("reflectivity_coefficient",true);
        param->p_size = pseudo_component->lookup_in_library("specularity",true);
        param->npower_pads = pseudo_component->lookup_in_library("num_power_pads",true);
        param->pad_to_pad_distance = pseudo_component->lookup_in_library("power_pad_distance",true);
        param->pad_length = pseudo_component->lookup_in_library("power_pad_length",true);
        param->ir_drop_limit = pseudo_component->lookup_in_library("ir_drop_limit",true);
        param->router_eff = pseudo_component->lookup_in_library("router_efficiency",true);
        param->rep_eff = pseudo_component->lookup_in_library("repeater_efficiency",true);
        param->fo = pseudo_component->lookup_in_library("average_fanouts",true);
        param->margin = pseudo_component->lookup_in_library("clock_margin",true);
        param->D = pseudo_component->lookup_in_library("HTree_max_span",true);
        param->latches_per_buffer = pseudo_component->lookup_in_library("latches_per_buffer",true);
        param->clock_factor = pseudo_component->lookup_in_library("clock_factor",true);
        param->clock_gating_factor = pseudo_component->lookup_in_library("clock_gating_factor",true);
        if(pseudo_component->exists_in_library("max_tier",true))
            param->max_tier = pseudo_component->lookup_in_library("max_tier",true);
            
        param->setup(false);
        
        /*
        // debugging
        cout << parameters_module.name << endl;
        cout << "param->Vdd = " << param->Vdd << endl;
        cout << "param->Vt = " << param->Vt << endl;
        cout << "param->tox = " << param->tox << endl;
        cout << "param->drive_p_div_n = " << param->drive_p_div_n << endl;
        cout << "param->f = " << param->f << endl;
        cout << "param->F = " << param->F << endl;
        cout << "param->A = " << param->A << endl;
        cout << "param->ngates = " << param->ngates << endl;
        cout << "param->F1 = " << param->F1 << endl;
        cout << "param->Vdd_spec = " << param->Vdd_spec << endl;
        cout << "param->Vt_spec = " << param->Vt_spec << endl;
        cout << "param->rho = " << param->rho << endl;
        cout << "param->Ileak_spec = " << param->Ileak_spec << endl;
        cout << "param->Idsat_spec = " << param->Idsat_spec << endl;
        cout << "param->ncp = " << param->ncp << endl;
        cout << "param->er = " << param->er << endl;
        cout << "param->k = " << param->k << endl;
        cout << "param->p = " << param->p << endl;
        cout << "param->a = " << param->a << endl;
        cout << "param->tdp_a = " << param->tdp_a << endl;
        cout << "param->alpha = " << param->alpha << endl;
        cout << "param->subvtslope_spec = " << param->subvtslope_spec << endl;
        cout << "param->s = " << param->s << endl;
        cout << "param->ar = " << param->ar << endl;
        cout << "param->p_size = " << param->p_size << endl;
        cout << "param->pad_to_pad_distance = " << param->pad_to_pad_distance << endl;
        cout << "param->pad_length = " << param->pad_length << endl;
        cout << "param->ir_drop_limit = " << param->ir_drop_limit << endl;
        cout << "param->router_eff = " << param->router_eff << endl;
        cout << "param->rep_eff = " << param->rep_eff << endl;
        cout << "param->fo = " << param->fo << endl;
        cout << "param->margin = " << param->margin << endl;
        cout << "param->D = " << param->D << endl;
        cout << "param->latches_per_buffer = " << param->latches_per_buffer << endl;
        cout << "param->clock_factor = " << param->clock_factor << endl;
        cout << "param->clock_gating_factor = " << param->clock_gating_factor << endl;
        cout << "param->kp = " << param->kp << endl;
        cout << "param->kc = " << param->kc << endl;
        cout << "param->beta_clock = " << param->beta_clock << endl;
        cout << "param->ew_ground_power = " << param->ew_power_ground << endl;
        cout << "param->kai = " << param->kai << endl;
        cout << "param->alpha_wire = " << param->alpha_wire << endl;
        cout << "param->ro = " << param->ro << endl;
        cout << "param->co = " << param->co << endl;
        cout << "param->H = " << param->H << endl;
        cout << "param->W = " << param->W << endl;
        cout << "param->T = " << param->T << endl;
        cout << "param->S = " << param->S << endl;
        cout << "param->cg = " << param->cg << endl;
        cout << "param->cm = " << param->cm << endl;
        cout << "param->ew = " << param->ew << endl;
        cout << "param->H_global = " << param->H_global << endl;
        cout << "param->W_global = " << param->W_global << endl;
        cout << "param->T_global = " << param->T_global << endl;
        cout << "param->S_global = " << param->S_global << endl;
        cout << "param->c_clock = " << param->c_clock << endl;
        cout << "param->max_tier = " << param->max_tier << endl;
        */
        intsim(chip,param,"config/intsim/intsim.out");
              
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

unit_energy_t energylib_intsim::get_unit_energy() {
    unit_energy_t unit_energy;
    
    switch(energy_model) {
        case KITFOX_INTSIM_LOGIC_GATE: {
            unit_energy.switching = chip->dyn_power_logic_gates/param->f;
            unit_energy.leakage = chip->leakage_power_logic_gates/param->f;
            break;
        }
        case KITFOX_INTSIM_REPEATER: {
            unit_energy.switching = chip->dyn_power_repeaters/param->f;
            unit_energy.leakage = chip->leakage_power_repeaters/param->f;
            break;
        }
        case KITFOX_INTSIM_INTERCONNECT: {
            unit_energy.switching = chip->power_wires/param->f;
            break;
        }
        case KITFOX_INTSIM_CLOCK: {
            unit_energy.baseline = chip->clock_power/param->f;
            break;
        }
        default: { // KITFOX_INTSIM_DEFAULT
            //unit_energy.baseline = chip->clock_power/param->f;
            unit_energy.baseline = chip->dyn_power_clock/param->f;
            unit_energy.switching = (chip->dyn_power_logic_gates+chip->dyn_power_repeaters+chip->power_wires)/param->f;
            //unit_energy.leakage = (chip->leakage_power_logic_gates+chip->leakage_power_repeaters)/param->f;
            unit_energy.leakage = (chip->leakage_power_logic_gates+chip->leakage_power_repeaters+chip->leakage_power_clock)/param->f;
            
            /*cout << "logic_gates=" << chip->dyn_power_logic_gates << "," << chip->leakage_power_logic_gates << endl;
            cout << "repeaters=" << chip->dyn_power_repeaters << "," << chip->leakage_power_repeaters << endl;
            cout << "wire=" << chip->power_wires << endl;
            cout << "clock=" << chip->clock_power << endl;*/
            break;
        }
    }

    /*cout << "unit_energy.baseline=" << unit_energy.baseline << endl;
    cout << "unit_energy.switching=" << unit_energy.switching << endl;
    cout << "unit_energy.leakage=" << unit_energy.leakage << endl;*/
    
    unit_energy.baseline *= energy_scaling;
    unit_energy.switching *= energy_scaling;
    unit_energy.leakage *= (energy_scaling*scaling);
    
    return unit_energy;
}

power_t energylib_intsim::get_tdp_power(Kelvin MaxTemperature) {
    Kelvin current_temperature = param->temp;
    Kelvin max_temperature = MaxTemperature;
    update_library_variable(KITFOX_DATA_TEMPERATURE,&max_temperature,false);

    double activity_factor = param->a;
    param->a = param->tdp_a;    
    chip->update_energy(param);
    param->a = activity_factor;

    unit_energy_t unit_energy = get_unit_energy();
    energy_t tdp_energy;
    power_t tdp_power;

    tdp_energy.dynamic = unit_energy.baseline;
    tdp_energy.dynamic += unit_energy.switching;    
    tdp_energy.leakage = unit_energy.leakage;
    tdp_energy.total = tdp_energy.get_total();
    tdp_power = tdp_energy * param->f;
    
    update_library_variable(KITFOX_DATA_TEMPERATURE,&current_temperature,false);
    
    return tdp_power;
}

power_t energylib_intsim::get_runtime_power(Second Time, Second Period, counter_t Counter) {
    param->a = (double)Counter.switching/Period/param->f;
    chip->update_energy(param);
    
    unit_energy_t unit_energy = get_unit_energy();
    energy_t runtime_energy;
    power_t runtime_power;
    
    runtime_energy.dynamic = unit_energy.baseline*Period*param->f;
    runtime_energy.dynamic += unit_energy.switching*Period*param->f;
    runtime_energy.leakage = unit_energy.leakage*Period*param->f;
    runtime_energy.total = runtime_energy.get_total();
    runtime_power = runtime_energy/Period;
    
    return runtime_power;
}

MeterSquare energylib_intsim::get_area() {
    return param->A;
}

bool energylib_intsim::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    
    if(IsLibraryVariable) return updated;
    
    switch(Type) {
        case KITFOX_DATA_CLOCK_FREQUENCY: {
            param->f = *(double*)Value;
            updated = true;
            break;
        }
        case KITFOX_DATA_TEMPERATURE: {
            param->temp = *(double*)Value;
            param->setup(false);
            updated = true;
            break;
        }
        case KITFOX_DATA_VOLTAGE: {
            double voltage = *(double*)Value;
            double scaling_ratio = voltage/param->Vdd;
            param->Vdd = voltage;
            param->Vdd_spec *= scaling_ratio;
            updated = true;
            break;
        }
        default: {
            // Nothing to do
            return updated;
        }
    }
    
    chip->update_energy(param);
    
    return updated;
}
