#ifndef __THERMALLIB_MICROFLUIDICS_H__
#define __THERMALLIB_MICROFLUIDICS_H__

#include "library.h"
#include "Microfluidics/microfluidics.h"

namespace libKitFox {
namespace libKitFox_MICROFLUIDICS {

enum KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_TYPES {
    KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_UNKNOWN = 0,
    KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_STEADY_STATE,
    KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_TRANSIENT,
    NUM_KITFOX_MICROFLUIDICS_THERMAL_ANALYSIS_TYPES
};

class thermallib_microfluidics : public thermal_library_t {
public:
    thermallib_microfluidics(pseudo_component_t *PseudoComponent);
    ~thermallib_microfluidics();
    
    virtual void initialize(void);
    virtual void calculate_temperature(Second Time, Second Period);
    virtual grid_t<Kelvin> get_thermal_grid(void);
    virtual Kelvin get_floorplan_temperature(Comp_ID ComponentID, int Type = KITFOX_TEMPERATURE_MAPPING_UNKNOWN);
    virtual Kelvin get_point_temperature(Meter X, Meter Y, Index Layer);
    virtual void map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower);
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    microfluidics_t *microfluidics;
    std::map<Comp_ID,unsigned> floorplanID_map;
    int thermal_analysis;
};

} // namespace libKitFox_MICROFLUIDICS
} // namespace libKitFox

#endif
