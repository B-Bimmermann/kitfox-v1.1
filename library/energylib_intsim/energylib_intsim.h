#ifndef __ENERGYLIB_INTSIM_H__
#define __ENERGYLIB_INTSIM_H__

#include "library.h"

#include "intsim_alpha/parameters.h"
#include "intsim_alpha/chip.h"
#include "intsim_alpha/intsim.h"

namespace libKitFox {
namespace libKitFox_INTSIM {

enum KITFOX_INTSIM_ENERGY_MODEL_TYPES {
    KITFOX_INTSIM_ENERGY_MODEL_UNKNOWN = 0,
    KITFOX_INTSIM_DEFAULT,
    KITFOX_INTSIM_LOGIC_GATE,
    KITFOX_INTSIM_REPEATER,
    KITFOX_INTSIM_INTERCONNECT,
    KITFOX_INTSIM_CLOCK,
    NUM_KITFOX_INTSIM_ENERGY_MODEL_TYPES
};

static const char *KITFOX_INTSIM_ENERGY_MODEL_STR[] = {
    "KITFOX_INTSIM_ENERGY_MODEL_UNKNOWN",
    "KITFOX_INTSIM_DEFAULT",
    "KITFOX_INTSIM_LOGIC_GATES",
    "KITFOX_INTSIM_REPEATER",
    "KITFOX_INTSIM_INTERCONNECT",
    "KITFOX_INTSIM_CLOCK",
    "NUM_KITFOX_INTSIM_ENERGY_MODEL_TYPES"
};

class energylib_intsim : public energy_library_t {
public:
    energylib_intsim(pseudo_component_t *PseudoComponent);
    ~energylib_intsim();
    
    virtual void initialize();
    virtual unit_energy_t get_unit_energy();
    virtual power_t get_tdp_power(Kelvin MaxTemperature = 373.0);
    virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter);
    virtual MeterSquare get_area();
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    int energy_model;
    
    double energy_scaling;
    double area_scaling;
    double scaling;
    
    intsim_chip_t *chip;
    intsim_param_t *param;
};

} // namespace libKitFox_INTSIM
} // namespace libKitFox

#endif
