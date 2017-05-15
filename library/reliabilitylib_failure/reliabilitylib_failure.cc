#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "reliabilitylib_failure.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_FAILURE;

reliabilitylib_failure::reliabilitylib_failure(pseudo_component_t *PseudoComponent) :
    reliability_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_FAILURE),
    model_config(NULL),
    failure_model(NULL)
{}

reliabilitylib_failure::~reliabilitylib_failure() {
    delete model_config;
    delete failure_model;
}

void reliabilitylib_failure::initialize() {
    failure_model = new failure_model_t();
    model_config = new failure_model_config_t();
    assert(failure_model); assert(model_config);

    memset(use_models,false,sizeof(bool)*NUM_FAILURE_MODEL_TYPES);    
    
    try {
        Setting &model_setting = pseudo_component->lookup_in_library("failure_model");
        for(unsigned i = 0; i < model_setting.getLength(); i++) {
            const char *failure_model_str = model_setting[i].getName();
            if(!strcasecmp(failure_model_str,"hci")||
               !strcasecmp(failure_model_str,"hot_carrier_injection")) {
                if(model_setting[i].exists("n"))
                    model_config->HCI.n = model_setting[i]["n"];
                if(model_setting[i].exists("Ea"))
                    model_config->HCI.Ea = model_setting[i]["Ea"];
                model_config->HCI.used = true;
                use_models[FAILURE_MODEL_HCI] = true;                
            }
            else if(!strcasecmp(failure_model_str,"em")||
                    !strcasecmp(failure_model_str,"electromigration")) {
                if(model_setting[i].exists("n"))
                    model_config->EM.n = model_setting[i]["n"];
                if(model_setting[i].exists("Ea"))
                    model_config->EM.Ea = model_setting[i]["Ea"];
                model_config->EM.used = true;
                use_models[FAILURE_MODEL_EM] = true;                
            }
            else if(!strcasecmp(failure_model_str,"nbti")||
                    !strcasecmp(failure_model_str,"negative_bias_temperature_instability")) {
                if(model_setting[i].exists("n"))
                    model_config->NBTI.n = model_setting[i]["n"];
                if(model_setting[i].exists("Ea"))
                    model_config->NBTI.Ea = model_setting[i]["Ea"];
                model_config->NBTI.used = true;
                use_models[FAILURE_MODEL_NBTI] = true;
            }
            else if(!strcasecmp(failure_model_str,"pbti")||
                    !strcasecmp(failure_model_str,"positive_bias_temperature_instability")) {
                // TODO
                model_config->PBTI.used = false;
                use_models[FAILURE_MODEL_PBTI] = false;
            }
            else if(!strcasecmp(failure_model_str,"sm")||
                    !strcasecmp(failure_model_str,"stress_migration")) {
                if(model_setting[i].exists("T0"))
                    model_config->SM.T0 = model_setting[i]["T0"];
                if(model_setting[i].exists("n"))
                    model_config->SM.n = model_setting[i]["n"];
                if(model_setting[i].exists("Ea"))
                    model_config->SM.Ea = model_setting[i]["Ea"];
                model_config->SM.used = true;
                use_models[FAILURE_MODEL_SM] = true;
            }
            else if(!strcasecmp(failure_model_str,"tc")||
                    !strcasecmp(failure_model_str,"thermal_cycling")) {
                if(model_setting[i].exists("n"))
                    model_config->TC.n = model_setting[i]["n"];
                model_config->TC.used = true;
                use_models[FAILURE_MODEL_TC] = true;
            }
            else if(!strcasecmp(failure_model_str,"tddb")||
                    !strcasecmp(failure_model_str,"time_dependent_dielectric_breakdown")) {
                if(model_setting[i].exists("a"))
                    model_config->TDDB.a = model_setting[i]["a"];
                if(model_setting[i].exists("b"))
                    model_config->TDDB.b = model_setting[i]["b"];
                if(model_setting[i].exists("c"))
                    model_config->TDDB.c = model_setting[i]["c"];
                if(model_setting[i].exists("x"))
                    model_config->TDDB.x = model_setting[i]["x"];
                if(model_setting[i].exists("y"))
                    model_config->TDDB.y = model_setting[i]["y"];
                if(model_setting[i].exists("z"))
                    model_config->TDDB.z = model_setting[i]["z"];
                model_config->TDDB.used = true;
                use_models[FAILURE_MODEL_TDDB] = true;
            }
            else {
                pseudo_component->kitfox->error("failure","%s.library.failure_model[%u] = %s is unknown",pseudo_component->name.c_str(),i,failure_model_str);
            }
        }
        
        model_config->baseline.Temp = pseudo_component->lookup_in_library("target_temperature");
        model_config->baseline.Vdd = pseudo_component->lookup_in_library("target_voltage");
        model_config->baseline.Freq = pseudo_component->lookup_in_library("target_frequency");
        model_config->baseline.MTTF = pseudo_component->lookup_in_library("target_MTTF");
        model_config->Temp_ambient = pseudo_component->lookup_in_library("ambient_temperature",true);
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
    
    failure_model->initialize(model_config);
    
    /* INPUT DEBUG
    cout << "HCI: {coeff=" << model_config->HCI.coeff << ", n=" << model_config->HCI.n << ", Ea=" << model_config->HCI.Ea << ", used=" << model_config->HCI.used << "}" << endl;
    cout << "EM: {coeff=" << model_config->EM.coeff << ", J=" << model_config->EM.J << ", n=" << model_config->EM.n << ", Ea=" << model_config->EM.Ea << ", used=" << model_config->EM.used << "}" << endl;
    cout << "NBTI: {coeff=" << model_config->NBTI.coeff << ", n=" << model_config->NBTI.n << ", Ea=" << model_config->NBTI.Ea << ", used=" << model_config->NBTI.used << "}" << endl;
    cout << "PBTI: {coeff=" << model_config->PBTI.coeff << ", used=" << model_config->PBTI.used << "}" << endl;
    cout << "SM: {coeff=" << model_config->SM.coeff << ", T0=" << model_config->SM.T0 << ", n=" << model_config->SM.n << ", Ea=" << model_config->SM.Ea << ", used=" << model_config->SM.used << "}" << endl;
    cout << "TC: {coeff=" << model_config->TC.coeff << ", n=" << model_config->TC.n << ", used=" << model_config->TC.used << "}" << endl;
    cout << "TDDB: {coeff=" << model_config->TDDB.coeff << ", a=" << model_config->TDDB.a << ", b=" << model_config->TDDB.b << ", c=" << model_config->TDDB.c << ", x=" << model_config->TDDB.x << ", y=" << model_config->TDDB.y << ", z=" << model_config->TDDB.z << ", used=" << model_config->TDDB.used << "}" << endl;
    cout << "baseline: {Temp=" << model_config->baseline.Temp << ", Vdd=" << model_config->baseline.Vdd << ", Freq=" << model_config->baseline.Freq << ", MTTF=" << model_config->baseline.MTTF << "}" << endl;
    cout << "k=" << model_config->k << endl;
    cout << "Temp=" << model_config->Temp << endl;
    cout << "Temp_ambient=" << model_config->Temp_ambient << endl;
    cout << "Vdd=" << model_config->Vdd << endl;
    cout << "Freq=" << model_config->Freq << endl;
    cout << "MTTF=" << 1.0/get_failure_rate(1.0,1.0,model_config->Temp,model_config->Vdd,model_config->Freq) << endl;
    */
    
    // Confirm MTTF matches with baseline inputs.
    // Operating conditions should have been set to baseline inputs.
    assert(fabs(1.0/get_failure_rate(1.0,1.0,model_config->Temp,model_config->Vdd,model_config->Freq)-model_config->baseline.MTTF) < 1.0);
}

Unitless reliabilitylib_failure::get_failure_rate(Second Time, Second Period, Kelvin Temperature, Volt Vdd, Hertz ClockFrequency) {
    Unitless failure_rate = 0.0;
    
    // Set operating conditions
    model_config->Temp = Temperature;
    // Limit the temperature range between 300~400K.
    if(model_config->Temp < 300.0) { model_config->Temp = 300.0; }
    else if(model_config->Temp > 400.0) { model_config->Temp = 400.0; }
    
    model_config->Vdd = Vdd;
    model_config->Freq = ClockFrequency;
    
    for(unsigned type = FAILURE_MODEL_NONE+1; type < NUM_FAILURE_MODEL_TYPES; type++) {
        if(use_models[type]) {
            // Failure rate is the sum of the failure rates of all modeled failure models.
            // This basically assumes failure rate model is exponential, Weibull, or a similar kind.
            failure_rate += failure_model->get_failure_rate(type);
        }
    }
    return failure_rate;
}

bool reliabilitylib_failure::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    if(IsLibraryVariable) {
        pseudo_component->kitfox->error("failure","%s doesn't have library-specific variable",pseudo_component->name.c_str());
    }
    
    switch(Type) {
        case KITFOX_DATA_VOLTAGE: { 
            model_config->Vdd = *(double*)Value; 
                updated = true; break; 
        }
        case KITFOX_DATA_TEMPERATURE: { 
            model_config->Temp = *(double*)Value; 
            updated = true; break;
        }
        case KITFOX_DATA_AMBIENT_TEMPERATURE: { 
            model_config->Temp_ambient = *(double*)Value; 
            updated = true; break;
        }
        case KITFOX_DATA_CLOCK_FREQUENCY: { 
            model_config->Freq = *(double*)Value; 
            updated = true; break;
        }
        default: { break; } // Nothing to do
    }
    
    return updated;
}

