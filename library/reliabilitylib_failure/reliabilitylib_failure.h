#ifndef __RELIABILITYLIB_FAILURE_H__
#define __RELIABILITYLIB_FAILURE_H__

#include "library.h"

#include "failure_v1.0/failure.h"

namespace libKitFox {
namespace libKitFox_FAILURE {

class reliabilitylib_failure : public reliability_library_t {
public:
    reliabilitylib_failure(pseudo_component_t *PseudoComponent);
    virtual ~reliabilitylib_failure();
    
    virtual void initialize();
    virtual Unitless get_failure_rate(Second Time, Second Period, Kelvin Temperature, Volt Vdd, Hertz ClockFrequency);
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    failure_model_config_t *model_config;
    failure_model_t *failure_model;
    bool use_models[NUM_FAILURE_MODEL_TYPES];
};

} // namesapce libKitFox_FAILURE
} // namespace libKitFox

#endif

