#include "library.h"
#include "component.h"

using namespace std;
using namespace libKitFox;

model_library_t::model_library_t(pseudo_component_t *PseudoComponent, int Type) :
    pseudo_component(PseudoComponent),
    type(Type)
{
}





energy_library_t::energy_library_t(pseudo_component_t *PseudoComponent, int Type) :
    model_library_t(PseudoComponent, KITFOX_LIBRARY_ENERGY_MODEL),
    type(Type)
{
}





thermal_library_t::thermal_library_t(pseudo_component_t *PseudoComponent, int Type) :
    model_library_t(PseudoComponent, KITFOX_LIBRARY_THERMAL_MODEL),
    type(Type),
    generate_thermal_grid(false)
{
}

void thermal_library_t::add_floorplan(string ComponentName, Comp_ID ComponentID)
{
    floorplan_name_map.insert(pair<string,Comp_ID>(ComponentName,ComponentID));
    floorplan_id_map.insert(pair<Comp_ID,string>(ComponentID,ComponentName));
}

const Index thermal_library_t::get_floorplan_counts(void)
{
    return floorplan_name_map.size();
}

const Comp_ID thermal_library_t::get_floorplan_id(string ComponentName)
{
    if(floorplan_name_map.count(ComponentName)) { return floorplan_name_map[ComponentName]; }
    return INVALID_COMP_ID;
}

const char* thermal_library_t::get_floorplan_name(Comp_ID ComponentID)
{
    if(floorplan_id_map.count(ComponentID)) { return floorplan_id_map[ComponentID].c_str(); }
    return NULL;
}

const Comp_ID thermal_library_t::get_floorplan_id(unsigned i)
{
    if(floorplan_name_map.size() == 0) { return INVALID_COMP_ID; }
    
    map<string,Comp_ID>::iterator it = floorplan_name_map.begin();
    for(unsigned p = 0; p < i; p++, it++) {
        if(it == floorplan_name_map.end()) { return INVALID_COMP_ID; }
    }
    return it->second;
}





reliability_library_t::reliability_library_t(pseudo_component_t *PseudoComponent, int Type) :
model_library_t(PseudoComponent, KITFOX_LIBRARY_RELIABILITY_MODEL),
type(Type)
{
}





sensor_library_t::sensor_library_t(pseudo_component_t *PseudoComponent, int Type) :
model_library_t(PseudoComponent, KITFOX_LIBRARY_SENSOR_MODEL),
type(Type)
{
}
