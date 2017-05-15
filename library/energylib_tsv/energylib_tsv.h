#ifndef __ENERGYLIB_TSV_H__
#define __ENERGYLIB_TSV_H__

#include "library.h"
#include "tsv_v1.0/tsv.h"

namespace libKitFox {
namespace libKitFox_TSV {

class energylib_tsv : public energy_library_t {
public:
    energylib_tsv(pseudo_component_t *PseudoComponent);
    virtual ~energylib_tsv();

    virtual void initialize();
    virtual unit_energy_t get_unit_energy();
    virtual power_t get_tdp_power(Kelvin temperatur = 373.0);
    virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter);
    virtual MeterSquare get_area();
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);

private:
    double energy_scaling;
    double area_scaling;
    double scaling;

    tsv_t *tsv;
};

} // namespace libKitFox_TSV
} // namespace libKitFox

#endif
