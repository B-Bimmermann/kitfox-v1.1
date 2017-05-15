#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "energylib_dramsim.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_DRAMSIM;

energylib_dramsim::energylib_dramsim(pseudo_component_t *PseudoComponent) :
    energy_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_DRAMSIM),
    energy_scaling(1.0),
    area_scaling(1.0),
    scaling(1.0)
{}

energylib_dramsim::~energylib_dramsim() {}

void energylib_dramsim::initialize() {
    try {
        dram.IDD0 = pseudo_component->lookup_in_library("IDD0",true);
        dram.IDD1 = pseudo_component->lookup_in_library("IDD1",true);
        dram.IDD2P = pseudo_component->lookup_in_library("IDD2P",true);
        dram.IDD2Q = pseudo_component->lookup_in_library("IDD2Q",true);
        dram.IDD2N = pseudo_component->lookup_in_library("IDD2N",true);
        dram.IDD3Pf = pseudo_component->lookup_in_library("IDD3Pf",true);
        dram.IDD3Ps = pseudo_component->lookup_in_library("IDD3Ps",true);
        dram.IDD3N = pseudo_component->lookup_in_library("IDD3N",true);
        dram.IDD4W = pseudo_component->lookup_in_library("IDD4W",true);
        dram.IDD4R = pseudo_component->lookup_in_library("IDD4R",true);
        dram.IDD5 = pseudo_component->lookup_in_library("IDD5",true);
        dram.IDD6 = pseudo_component->lookup_in_library("IDD6",true);
        dram.IDD6L = pseudo_component->lookup_in_library("IDD6L",true);
        dram.IDD7 = pseudo_component->lookup_in_library("IDD7",true);
        
        dram.BL = pseudo_component->lookup_in_library("BL",true);
        dram.DEVICE_WIDTH = pseudo_component->lookup_in_library("DEVICE_WIDTH",true);
        dram.DATA_BUS_BITS = pseudo_component->lookup_in_library("DATA_BUS_BITS",true);
        dram.NUM_DEVICES = dram.DATA_BUS_BITS/dram.DEVICE_WIDTH;
        assert(dram.NUM_DEVICES);
        
        dram.tRC = pseudo_component->lookup_in_library("tRC",true);
        dram.tRAS = pseudo_component->lookup_in_library("tRAS",true);
        dram.tRFC = pseudo_component->lookup_in_library("tRFC",true);
        
        voltage = pseudo_component->lookup_in_library("voltage",true);
        clock_frequency = pseudo_component->lookup_in_library("clock_frequency",true);
        
        const char *page_policy_str = pseudo_component->lookup_in_library("page_policy",true);
        if(!strcasecmp(page_policy_str,"close_page")||!strcasecmp(page_policy_str,"closed_page"))
            dram.page_policy = KITFOX_DRAMSIM_CLOSE_PAGE_POLICY;
        else if(!strcasecmp(page_policy_str,"open_page"))
            dram.page_policy = KITFOX_DRAMSIM_OPEN_PAGE_POLICY;
        else
            pseudo_component->kitfox->error("dramsim","%s.library.page_policy = %s is unknown",pseudo_component->name.c_str(),page_policy_str);
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

unit_energy_t energylib_dramsim::get_unit_energy() {
    unit_energy_t unit_energy;
    
    unit_energy.background_open_page_high = 1e-3*(double)(dram.IDD3N * dram.NUM_DEVICES);
    unit_energy.background_closed_page_high = 1e-3*(double)(dram.IDD2N * dram.NUM_DEVICES);
    //unit_energy.background_open_page_low = 1e-3*(double)(dram.IDD3P * dram.NUM_DEVICES);
    unit_energy.background_closed_page_low = 1e-3*(double)(dram.IDD2P * dram.NUM_DEVICES);
    unit_energy.read = 1e-3*(double)((dram.IDD4R - dram.IDD3N) * dram.BL/2 * dram.NUM_DEVICES);
    unit_energy.write = 1e-3*(double)((dram.IDD4W - dram.IDD3N) * dram.BL/2 * dram.NUM_DEVICES);
    unit_energy.precharge = 1e-3*(double)(((dram.IDD0 * dram.tRC) - ((dram.IDD3N * dram.tRAS) + (dram.IDD2N * (dram.tRC - dram.tRAS)))) * dram.NUM_DEVICES);
    unit_energy.refresh = 1e-3*(double)((dram.IDD5 - dram.IDD3N) * dram.tRFC * dram.NUM_DEVICES);
    
    return unit_energy;
}

power_t energylib_dramsim::get_tdp_power(Kelvin MaxTemperature) {
    //Kelvin max_temperature = MaxTemperature;
    //update_library_variable(KITFOX_DATA_TEMPERATURE,&max_temperature,false);
    
    unit_energy_t unit_energy = get_unit_energy();
    energy_t tdp_energy;
    power_t tdp_power;
    
    if(dram.page_policy == KITFOX_DRAMSIM_CLOSE_PAGE_POLICY) {
    }
    else { // KITFOX_DRAMSIM_OPEN_PAGE_POLICY
    }
    
    //update_library_variable(KITFOX_DATA_TEMPERATURE,current_temperature,false);
    
    return tdp_power;
}

power_t energylib_dramsim::get_runtime_power(Second Time, Second Period, counter_t Counter) {
    unit_energy_t unit_energy = get_unit_energy();
    energy_t runtime_energy;
    power_t runtime_power;

    runtime_energy.dynamic += unit_energy.background_open_page_high * (double)Counter.background_open_page_high;
    runtime_energy.dynamic += unit_energy.background_closed_page_high * (double)Counter.background_closed_page_high;
    runtime_energy.dynamic += unit_energy.background_open_page_low * (double)Counter.background_open_page_low;
    runtime_energy.dynamic += unit_energy.background_closed_page_low * (double)Counter.background_closed_page_low;
    runtime_energy.dynamic += unit_energy.read * (double)Counter.read;
    runtime_energy.dynamic += unit_energy.write * (double)Counter.write;
    runtime_energy.dynamic += unit_energy.precharge * (double)Counter.precharge;
    runtime_energy.dynamic += unit_energy.refresh * (double)Counter.refresh;
    runtime_energy.total = runtime_energy.get_total();
    runtime_energy = (runtime_energy*voltage)/1.0;
    
    runtime_power = runtime_energy / (Period*clock_frequency);
    
    return runtime_power;
}

MeterSquare energylib_dramsim::get_area() {
    MeterSquare area = 0.0;
    
    return area;
}

bool energylib_dramsim::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    
    if(IsLibraryVariable) return updated;
    
    switch(Type) {
        case KITFOX_DATA_VOLTAGE: {
            voltage = *(double*)Value;
            updated = true;
            break;
        }
        default: {
            // Nothing to do
            break;
        }
    }
    
    return updated;
}
