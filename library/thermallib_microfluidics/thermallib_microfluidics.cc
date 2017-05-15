#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "thermallib_microfluidics.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_MICROFLUIDICS;

thermallib_microfluidics::thermallib_microfluidics(pseudo_component_t *PseudoComponent) :
    thermal_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_MICROFLUIDICS),
    microfluidics(NULL),
    thermal_analysis(KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_UNKNOWN)
{}

thermallib_microfluidics::~thermallib_microfluidics() {
    delete microfluidics;
    floorplanID_map.clear();
}

void thermallib_microfluidics::initialize() {
    microfluidics = new microfluidics_t();

    try {    
        microfluidics->config->HC = pseudo_component->lookup_in_library("HC",true);
        microfluidics->config->Dp = pseudo_component->lookup_in_library("Dp",true);
        microfluidics->config->NW = pseudo_component->lookup_in_library("NT",true);
        microfluidics->config->NH = pseudo_component->lookup_in_library("NL",true);
        
        microfluidics->config->ThSi = pseudo_component->lookup_in_library("ThSi",true);
        microfluidics->config->ThO = pseudo_component->lookup_in_library("ThO",true);
        microfluidics->config->ThB = pseudo_component->lookup_in_library("ThB",true);
        
        microfluidics->config->PPower = pseudo_component->lookup_in_library("PPower",true);
        
        microfluidics->config->heff = pseudo_component->lookup_in_library("heff",true);
        microfluidics->config->hamb = pseudo_component->lookup_in_library("hamb",true);
        microfluidics->config->Tfin = pseudo_component->lookup_in_library("Tfin",true);
        microfluidics->config->Twini = pseudo_component->lookup_in_library("temperature",true);
        microfluidics->config->Tamb = pseudo_component->lookup_in_library("ambient_temperature",true);
        
        microfluidics->config->rho_Si = pseudo_component->lookup_in_library("rho_Si",true);
        microfluidics->config->cp_Si = pseudo_component->lookup_in_library("cp_Si",true);
        microfluidics->config->k_Si = pseudo_component->lookup_in_library("k_Si",true);
        microfluidics->config->rho_O = pseudo_component->lookup_in_library("rho_O",true);
        microfluidics->config->k_O = pseudo_component->lookup_in_library("k_O",true);
        microfluidics->config->cp_B = pseudo_component->lookup_in_library("cp_B",true);
        microfluidics->config->k_B = pseudo_component->lookup_in_library("k_B",true);
        microfluidics->config->rho_f = pseudo_component->lookup_in_library("rho_f",true);
        microfluidics->config->cp_f = pseudo_component->lookup_in_library("cp_f",true);
        microfluidics->config->k_f = pseudo_component->lookup_in_library("k_f",true);
        microfluidics->config->mu_f = pseudo_component->lookup_in_library("mu_f",true);
        
        const char *thermal_mapping_type_str = pseudo_component->lookup_in_library("grid_map_mode",true);
        if(!strcasecmp(thermal_mapping_type_str,"center"))
            microfluidics->config->thermal_mapping_type = MICROFLUIDICS_THERMAL_MAPPING_CENTER;
        else if(!strcasecmp(thermal_mapping_type_str,"average")||!strcasecmp(thermal_mapping_type_str,"avg"))
            microfluidics->config->thermal_mapping_type = MICROFLUIDICS_THERMAL_MAPPING_AVERAGE;
        else if(!strcasecmp(thermal_mapping_type_str,"max")||!strcasecmp(thermal_mapping_type_str,"maximum"))
            microfluidics->config->thermal_mapping_type = MICROFLUIDICS_THERMAL_MAPPING_MAX;
        else if(!strcasecmp(thermal_mapping_type_str,"min")||!strcasecmp(thermal_mapping_type_str,"minimum"))
            microfluidics->config->thermal_mapping_type = MICROFLUIDICS_THERMAL_MAPPING_MIN;
        else {
            pseudo_component->kitfox->error("microfluidics","%s.library.grid_map_mode = %s is unknown",pseudo_component->name.c_str(),thermal_mapping_type_str);
        }
        
        const char *thermal_analysis_str = pseudo_component->lookup_in_library("thermal_analysis");
        if(!strcasecmp(thermal_analysis_str,"transient"))
            thermal_analysis = KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_TRANSIENT;
        else if(!strcasecmp(thermal_analysis_str,"steady")||!strcasecmp(thermal_analysis_str,"steady_state"))
            thermal_analysis = KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_STEADY_STATE;
        else {
            pseudo_component->kitfox->error("microfluidics","%s.library.thermal_analysis = %s is unknown",pseudo_component->name.c_str(),thermal_analysis_str);
        }
        
        for(unsigned i = 0; i < get_floorplan_counts(); i++) {
            Comp_ID floorplan_id = get_floorplan_id(i); // 'i'th floorplan's ID
            const char *floorplan_name = get_floorplan_name(floorplan_id);

            dimension_t floorplan_dimension;
            if(pseudo_component->kitfox->pull_data(floorplan_id,0.0,UNSPECIFIED_TIME,KITFOX_DATA_DIMENSION,&floorplan_dimension) != KITFOX_QUEUE_ERROR_NONE) {
                pseudo_component->kitfox->error("microfluidics","%s.library.floorplan = %s is invalid",pseudo_component->name.c_str(),floorplan_name);
            }
            microfluidics->add_floorplan(string(floorplan_name),floorplan_dimension.left,floorplan_dimension.bottom,floorplan_dimension.width,floorplan_dimension.height,floorplan_dimension.die_index);
            
            floorplanID_map[floorplan_id] = microfluidics->get_floorplan_index(string(floorplan_name));
        }
        
        microfluidics->initialize();
        
        /*
        cout << "microfluidics->config->HC=" << microfluidics->config->HC << endl;
        cout << "microfluidics->config->Dp=" << microfluidics->config->Dp << endl;
        cout << "microfluidics->config->NT=" << microfluidics->config->NW << endl;
        cout << "microfluidics->config->NL=" << microfluidics->config->NH << endl;
    
        cout << "microfluidics->config->ThSi=" << microfluidics->config->ThSi << endl;
        cout << "microfluidics->config->ThO=" << microfluidics->config->ThO << endl;
        cout << "microfluidics->config->ThB=" << microfluidics->config->ThB << endl;
    
        cout << "microfluidics->config->PPower=" << microfluidics->config->PPower << endl;
    
        cout << "microfluidics->config->heff=" << microfluidics->config->heff << endl;
        cout << "microfluidics->config->hamb=" << microfluidics->config->hamb << endl;
        cout << "microfluidics->config->Tfin=" << microfluidics->config->Tfin << endl;
        cout << "microfluidics->config->Twini=" << microfluidics->config->Twini << endl;
        cout << "microfluidics->config->Tamb=" << microfluidics->config->Tamb << endl;
    
        cout << "microfluidics->config->rho_Si=" << microfluidics->config->rho_Si << endl;
        cout << "microfluidics->config->cp_Si=" << microfluidics->config->cp_Si << endl;
        cout << "microfluidics->config->k_Si=" << microfluidics->config->k_Si << endl;
        cout << "microfluidics->config->rho_O=" << microfluidics->config->rho_O << endl;
        cout << "microfluidics->config->k_O=" << microfluidics->config->k_O << endl;
        cout << "microfluidics->config->cp_B=" << microfluidics->config->cp_B << endl;
        cout << "microfluidics->config->k_B=" << microfluidics->config->k_B << endl;
        cout << "microfluidics->config->rho_f=" << microfluidics->config->rho_f << endl;
        cout << "microfluidics->config->cp_f=" << microfluidics->config->cp_f << endl;
        cout << "microfluidics->config->k_f=" << microfluidics->config->k_f << endl;
        cout << "microfluidics->config->mu_f=" << microfluidics->config->mu_f << endl;
        */
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

void thermallib_microfluidics::calculate_temperature(Second Time, Second Period) {
    microfluidics->convert_floorplan_to_grid_power();

    if(thermal_analysis == KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_TRANSIENT) {
        microfluidics->calculate_transient_temperature(Period);
    }
    else { // steady state
        microfluidics->calculate_steady_state_temperature();
    }
    
    microfluidics->convert_grid_to_floorplan_temperature();
}

grid_t<Kelvin> thermallib_microfluidics::get_thermal_grid() {
    grid_t<Kelvin> thermal_grid;
    
    thermal_grid.reserve(microfluidics->config->Nx,microfluidics->config->Ny,microfluidics->config->Nz);
    
    for(unsigned z = 0; z < microfluidics->config->Nz; z++) {
        for(unsigned y = 0; y < microfluidics->config->Ny; y++) {
            for(unsigned x = 0; x < microfluidics->config->Nx; x++) {
                thermal_grid(x,y,z) = microfluidics->Tw_grid[x+y*microfluidics->config->Nx+z*microfluidics->config->Nx*microfluidics->config->Ny];
            }
        }
    }
    
    return thermal_grid;
}

Kelvin thermallib_microfluidics::get_floorplan_temperature(Comp_ID ComponentID, int Type) {
    Kelvin temperature = 0.0;
    
    if(Type != KITFOX_TEMPERATURE_MAPPING_UNKNOWN) {
        if(Type == KITFOX_TEMPERATURE_MAPPING_CENTER) {
            if(microfluidics->config->thermal_mapping_type != MICROFLUIDICS_THERMAL_MAPPING_CENTER)
                microfluidics->convert_grid_to_floorplan_temperature(MICROFLUIDICS_THERMAL_MAPPING_CENTER);
        }
        else if(Type == KITFOX_TEMPERATURE_MAPPING_AVERAGE) {
            if(microfluidics->config->thermal_mapping_type != MICROFLUIDICS_THERMAL_MAPPING_AVERAGE)
                microfluidics->convert_grid_to_floorplan_temperature(MICROFLUIDICS_THERMAL_MAPPING_AVERAGE);
        }
        else if(Type == KITFOX_TEMPERATURE_MAPPING_MAX) {
            if(microfluidics->config->thermal_mapping_type != MICROFLUIDICS_THERMAL_MAPPING_MAX)
                microfluidics->convert_grid_to_floorplan_temperature(MICROFLUIDICS_THERMAL_MAPPING_MAX);
        }
        else if(Type == KITFOX_TEMPERATURE_MAPPING_MIN) {
            if(microfluidics->config->thermal_mapping_type != MICROFLUIDICS_THERMAL_MAPPING_MIN)
                microfluidics->convert_grid_to_floorplan_temperature(MICROFLUIDICS_THERMAL_MAPPING_MIN);
        }
    }
    
    assert(floorplanID_map.count(ComponentID));
    microfluidics_floorplan_t *floorplan = microfluidics->get_floorplan(floorplanID_map[ComponentID]);
    temperature = floorplan->get_temperature();
    
    return temperature;
}

Kelvin thermallib_microfluidics::get_point_temperature(Meter X, Meter Y, Index Layer) {
    double temperature = 0.0;
    
    double grid_width = microfluidics->config->ST;
    double grid_height = microfluidics->config->SL;
    
    unsigned x_index = (unsigned)(X/grid_width)+1;
    unsigned y_index = (unsigned)(Y/grid_width)+1;
    
    temperature = microfluidics->Tw_grid[x_index+y_index*microfluidics->config->Nx+Layer*microfluidics->config->Nx*microfluidics->config->Ny];
    
    return temperature;
}

void thermallib_microfluidics::map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower) {
    assert(floorplanID_map.count(ComponentID));
    microfluidics->set_floorplan_power(floorplanID_map[ComponentID],PartitionPower.get_total());
}

bool thermallib_microfluidics::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    if(IsLibraryVariable) {
        pseudo_component->kitfox->error("microfluidics","%s doesn't have library-specific variable",pseudo_component->name.c_str());
    }
    
    switch(Type) {
        case KITFOX_DATA_CLOCK_FREQUENCY:
        case KITFOX_DATA_VOLTAGE:
        case KITFOX_DATA_THRESHOLD_VOLTAGE:
        case KITFOX_DATA_TEMPERATURE:
        case KITFOX_DATA_TDP:
        case KITFOX_DATA_POWER: 
        default: {
            // Nothing to do
            break;
        }
    }
    return updated;
}

