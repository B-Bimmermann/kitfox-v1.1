#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "energylib_tsv.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_TSV;

energylib_tsv::energylib_tsv(pseudo_component_t *PseudoComponent) :
    energy_library_t(PseudoComponent, KITFOX_LIBRARY_MODEL_TSV),
    energy_scaling(1.0),
    area_scaling(1.0),
    scaling(1.0),
    tsv(NULL)
{
}

energylib_tsv::~energylib_tsv()
{
    delete tsv;
}

void energylib_tsv::initialize() {
    tsv = new tsv_t();

    try {
        tsv->clock_frequency = pseudo_component->exists_in_library("clock_frequency", true);
        if(pseudo_component->exists_in_library("voltage"))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("voltage", true);

        if(pseudo_component->exists_in_library("dielectric_constant", true))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("dielectric_constant", true);
        if(pseudo_component->exists_in_library("relative_dielectric_constant", true))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("relative_dielectric_constant", true);
        if(pseudo_component->exists_in_library("tsv_height"))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("tsv_height");
        if(pseudo_component->exists_in_library("tsv_radius"))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("tsv_radius");
        if(pseudo_component->exists_in_library("sidewall_thickness"))
            tsv->dielectric_constant = pseudo_component->lookup_in_library("sidewall_thickness");

        if(pseudo_component->exists_in_library("scaling"))
            scaling = pseudo_component->lookup_in_library("scaling");
        if(pseudo_component->exists_in_library("energy_scaling"))
            energy_scaling = pseudo_component->lookup_in_library("energy_scaling");
        if(pseudo_component->exists_in_library("area_scaling"))
            area_scaling = pseudo_component->lookup_in_library("area_scaling");

        double data_width = pseudo_component->lookup_in_library("data_width");
        area_scaling *= data_width;
        energy_scaling *= data_width;
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

unit_energy_t energylib_tsv::get_unit_energy() {
    unit_energy_t unit_energy;

    unit_energy.switching = tsv->get_capacitance() * tsv->voltage * tsv->voltage;
    unit_energy.switching *= energy_scaling;

    return unit_energy;
}

power_t energylib_tsv::get_tdp_power(Kelvin Temperature) {
    unit_energy_t unit_energy = get_unit_energy();
    energy_t tdp_energy;
    power_t tdp_power;

    tdp_energy.dynamic = unit_energy.switching;
    tdp_power = tdp_energy * tsv->clock_frequency;

    return tdp_power;
}

power_t energylib_tsv::get_runtime_power(Second Time, Second Period, counter_t Counter) {
    unit_energy_t unit_energy = get_unit_energy();
    energy_t runtime_energy;
    power_t runtime_power;

    runtime_energy.dynamic = unit_energy.switching * (double)Counter.switching;
    runtime_power = runtime_energy / Period;
    
    return runtime_power;
}

MeterSquare energylib_tsv::get_area() {
    return tsv->get_area();
}

bool energylib_tsv::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;

    if(IsLibraryVariable) return updated;

    switch(Type) {
        case KITFOX_DATA_CLOCK_FREQUENCY: {
            tsv->clock_frequency = *(Hertz*)Value;
            updated = true;
            break;
        }
        case KITFOX_DATA_VOLTAGE: {
            tsv->voltage = *(Volt*)Value;
            updated = true;
            break;
        }
        case KITFOX_DATA_TEMPERATURE: {
            updated = true;
            break;
        }
        default: { break; }
    }

    return updated;
}
