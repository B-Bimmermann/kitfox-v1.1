#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

struct thermal_component {
    Comp_ID id;
    power_t power;
    Kelvin temperature;
};

int main(int argc, char **argv) {
    printf("Processing HotSpot test using config/hotspot.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/hotspot.config");

    thermal_component package, L2_left, L2, L2_right, Icache, Dcache, Bpred_0, Bpred_1, Bpred_2, DTB_0, DTB_1, DTB_2, FPAdd_0, FPAdd_1, FPReg_0, FPReg_1, FPReg_2, FPReg_3, FPMul_0, FPMul_1, FPMap_0, FPMap_1, IntMap, IntQ, IntReg_0, IntReg_1, IntExec, FPQ, LdStQ, ITB_0, ITB_1;
  
    // Following are the power numbers from a HotSpot example
    L2_left.power.total = 1.44;
    L2.power.total = 7.37;
    L2_right.power.total = 1.44;
    Icache.power.total = 8.27;
    Dcache.power.total = 14.3;
    Bpred_0.power.total = 1.51666666666667;
    Bpred_1.power.total = 1.51666666666667;
    Bpred_2.power.total = 1.51666666666667;
    DTB_0.power.total = 0.0596666666666667;
    DTB_1.power.total = 0.0596666666666667;
    DTB_2.power.total = 0.0596666666666667;
    FPAdd_0.power.total = 0.62;
    FPAdd_1.power.total = 0.62;
    FPReg_0.power.total = 0.19375;
    FPReg_1.power.total = 0.19375;
    FPReg_2.power.total = 0.19375;
    FPReg_3.power.total = 0.19375;
    FPMul_0.power.total = 0.665;
    FPMul_1.power.total = 0.665;
    FPMap_0.power.total = 0.02355;
    FPMap_1.power.total = 0.02355;
    IntMap.power.total = 1.07;
    IntQ.power.total = 0.365;
    IntReg_0.power.total = 2.585;
    IntReg_1.power.total = 2.585;
    IntExec.power.total = 7.7;
    FPQ.power.total = 0.0354;
    LdStQ.power.total = 3.46;
    ITB_0.power.total = 0.2;
    ITB_1.power.total = 0.2;
  
    Second sampling_interval = 3.333e-6; // Sampling interval
    Second current_time = sampling_interval; // Only one sampling point

    // Get pseudo component IDs
    package.id = kitfox->get_component_id("package");
    L2_left.id = kitfox->get_component_id("package.L2_left");
    L2.id = kitfox->get_component_id("package.L2");
    L2_right.id = kitfox->get_component_id("package.L2_right");
    Icache.id = kitfox->get_component_id("package.Icache");
    Dcache.id = kitfox->get_component_id("package.Dcache");
    Bpred_0.id = kitfox->get_component_id("package.Bpred_0");
    Bpred_1.id = kitfox->get_component_id("package.Bpred_1");
    Bpred_2.id = kitfox->get_component_id("package.Bpred_2");
    DTB_0.id = kitfox->get_component_id("package.DTB_0");
    DTB_1.id = kitfox->get_component_id("package.DTB_1");
    DTB_2.id = kitfox->get_component_id("package.DTB_2");
    FPAdd_0.id = kitfox->get_component_id("package.FPAdd_0");
    FPAdd_1.id = kitfox->get_component_id("package.FPAdd_1");
    FPReg_0.id = kitfox->get_component_id("package.FPReg_0");
    FPReg_1.id = kitfox->get_component_id("package.FPReg_1");
    FPReg_2.id = kitfox->get_component_id("package.FPReg_2");
    FPReg_3.id = kitfox->get_component_id("package.FPReg_3");
    FPMul_0.id = kitfox->get_component_id("package.FPMul_0");
    FPMul_1.id = kitfox->get_component_id("package.FPMul_1");
    FPMap_0.id = kitfox->get_component_id("package.FPMap_0");
    FPMap_1.id = kitfox->get_component_id("package.FPMap_1");
    IntMap.id = kitfox->get_component_id("package.IntMap");
    IntQ.id = kitfox->get_component_id("package.IntQ");
    IntReg_0.id = kitfox->get_component_id("package.IntReg_0");
    IntReg_1.id = kitfox->get_component_id("package.IntReg_1");
    IntExec.id = kitfox->get_component_id("package.IntExec");
    FPQ.id = kitfox->get_component_id("package.FPQ");
    LdStQ.id = kitfox->get_component_id("package.LdStQ");
    ITB_0.id = kitfox->get_component_id("package.ITB_0");
    ITB_1.id = kitfox->get_component_id("package.ITB_1");

    assert(package.id != INVALID_COMP_ID);
    assert(L2_left.id != INVALID_COMP_ID);
    assert(L2.id != INVALID_COMP_ID);
    assert(L2_right.id != INVALID_COMP_ID);
    assert(Icache.id != INVALID_COMP_ID);
    assert(Dcache.id != INVALID_COMP_ID);
    assert(Bpred_0.id != INVALID_COMP_ID);
    assert(Bpred_1.id != INVALID_COMP_ID);
    assert(Bpred_2.id != INVALID_COMP_ID);
    assert(DTB_0.id != INVALID_COMP_ID);
    assert(DTB_1.id != INVALID_COMP_ID);
    assert(DTB_2.id != INVALID_COMP_ID);
    assert(FPAdd_0.id != INVALID_COMP_ID);
    assert(FPAdd_1.id != INVALID_COMP_ID);
    assert(FPReg_0.id != INVALID_COMP_ID);
    assert(FPReg_1.id != INVALID_COMP_ID);
    assert(FPReg_2.id != INVALID_COMP_ID);
    assert(FPReg_3.id != INVALID_COMP_ID);
    assert(FPMul_0.id != INVALID_COMP_ID);
    assert(FPMul_1.id != INVALID_COMP_ID);
    assert(FPMap_0.id != INVALID_COMP_ID);
    assert(FPMap_1.id != INVALID_COMP_ID);
    assert(IntMap.id != INVALID_COMP_ID);
    assert(IntQ.id != INVALID_COMP_ID);
    assert(IntReg_0.id != INVALID_COMP_ID);
    assert(IntReg_1.id != INVALID_COMP_ID);
    assert(IntExec.id != INVALID_COMP_ID);
    assert(FPQ.id != INVALID_COMP_ID);
    assert(LdStQ.id != INVALID_COMP_ID);
    assert(ITB_0.id != INVALID_COMP_ID);
    assert(ITB_1.id != INVALID_COMP_ID);

    // Manually insert floorplan powers
    assert(kitfox->push_data(L2_left.id, current_time, sampling_interval, KITFOX_DATA_POWER, &L2_left.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(L2.id, current_time, sampling_interval, KITFOX_DATA_POWER, &L2.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(L2_right.id, current_time, sampling_interval, KITFOX_DATA_POWER, &L2_right.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(Icache.id, current_time, sampling_interval, KITFOX_DATA_POWER, &Icache.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(Dcache.id, current_time, sampling_interval, KITFOX_DATA_POWER, &Dcache.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(Bpred_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &Bpred_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(Bpred_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &Bpred_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(Bpred_2.id, current_time, sampling_interval, KITFOX_DATA_POWER, &Bpred_2.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(DTB_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &DTB_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(DTB_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &DTB_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(DTB_2.id, current_time, sampling_interval, KITFOX_DATA_POWER, &DTB_2.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPAdd_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPAdd_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPAdd_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPAdd_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPReg_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPReg_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPReg_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPReg_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPReg_2.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPReg_2.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPReg_3.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPReg_3.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPMul_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPMul_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPMul_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPMul_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPMap_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPMap_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPMap_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPMap_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(IntMap.id, current_time, sampling_interval, KITFOX_DATA_POWER, &IntMap.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(IntQ.id, current_time, sampling_interval, KITFOX_DATA_POWER, &IntQ.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(IntReg_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &IntReg_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(IntReg_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &IntReg_1.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(IntExec.id, current_time, sampling_interval, KITFOX_DATA_POWER, &IntExec.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(FPQ.id, current_time, sampling_interval, KITFOX_DATA_POWER, &FPQ.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(LdStQ.id, current_time, sampling_interval, KITFOX_DATA_POWER, &LdStQ.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(ITB_0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &ITB_0.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(ITB_1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &ITB_1.power) == KITFOX_QUEUE_ERROR_NONE);

    // Calculate temperature
    kitfox->calculate_temperature(package.id, current_time, sampling_interval);

    // After synchronizing the pseudo component hierarchy, any pseudo components can be probed.
    assert(kitfox->pull_data(package.id, current_time, sampling_interval, KITFOX_DATA_POWER, &package.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(L2_left.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &L2_left.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(L2.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &L2.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(L2_right.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &L2_right.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(Icache.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &Icache.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(Dcache.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &Dcache.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(Bpred_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &Bpred_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(Bpred_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &Bpred_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(Bpred_2.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &Bpred_2.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(DTB_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &DTB_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(DTB_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &DTB_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(DTB_2.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &DTB_2.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPAdd_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPAdd_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPAdd_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPAdd_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPReg_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPReg_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPReg_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPReg_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPReg_2.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPReg_2.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPReg_3.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPReg_3.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPMul_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPMul_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPMul_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPMul_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPMap_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPMap_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPMap_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPMap_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(IntMap.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &IntMap.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(IntQ.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &IntQ.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(IntReg_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &IntReg_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(IntReg_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &IntReg_1.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(IntExec.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &IntExec.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(FPQ.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &FPQ.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(LdStQ.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &LdStQ.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(ITB_0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &ITB_0.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(ITB_1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &ITB_1.temperature) == KITFOX_QUEUE_ERROR_NONE);

    printf("package.power.total = %3.3lfW\n", package.power.get_total());
    printf("package.L2_left.temperature = %3.3lfK\n", L2_left.temperature);
    printf("package.L2.temperature = %3.3lfK\n", L2.temperature);
    printf("package.L2_right.temperature = %3.3lfK\n", L2_right.temperature);
    printf("package.Icache.temperature = %3.3lfK\n", Icache.temperature);
    printf("package.Dcache.temperature = %3.3lfK\n", Dcache.temperature);
    printf("package.Bpred_0.temperature = %3.3lfK\n", Bpred_0.temperature);
    printf("package.Bpred_1.temperature = %3.3lfK\n", Bpred_1.temperature);
    printf("package.Bpred_2.temperature = %3.3lfK\n", Bpred_2.temperature);
    printf("package.DTB_0.temperature = %3.3lfK\n", DTB_0.temperature);
    printf("package.DTB_1.temperature = %3.3lfK\n", DTB_1.temperature);
    printf("package.DTB_2.temperature = %3.3lfK\n", DTB_2.temperature);
    printf("package.FPAdd_0.temperature = %3.3lfK\n", FPAdd_0.temperature);
    printf("package.FPAdd_1.temperature = %3.3lfK\n", FPAdd_1.temperature);
    printf("package.FPReg_0.temperature = %3.3lfK\n", FPReg_0.temperature);
    printf("package.FPReg_1.temperature = %3.3lfK\n", FPReg_1.temperature);
    printf("package.FPReg_2.temperature = %3.3lfK\n", FPReg_2.temperature);
    printf("package.FPReg_3.temperature = %3.3lfK\n", FPReg_3.temperature);
    printf("package.FPMul_0.temperature = %3.3lfK\n", FPMul_0.temperature);
    printf("package.FPMul_1.temperature = %3.3lfK\n", FPMul_1.temperature);
    printf("package.FPMap_0.temperature = %3.3lfK\n", FPMap_0.temperature);
    printf("package.FPMap_1.temperature = %3.3lfK\n", FPMap_1.temperature);
    printf("package.IntMap.temperature = %3.3lfK\n", IntMap.temperature);
    printf("package.IntQ.temperature = %3.3lfK\n", IntQ.temperature);
    printf("package.IntReg_0.temperature = %3.3lfK\n", IntReg_0.temperature);
    printf("package.IntReg_1.temperature = %3.3lfK\n", IntReg_1.temperature);
    printf("package.IntExec.temperature = %3.3lfK\n", IntExec.temperature);
    printf("package.FPQ.temperature = %3.3lfK\n", FPQ.temperature);
    printf("package.LdStQ.temperature = %3.3lfK\n", LdStQ.temperature);
    printf("package.ITB_0.temperature = %3.3lfK\n", ITB_0.temperature);
    printf("package.ITB_1.temperature = %3.3lfK\n", ITB_1.temperature);

    delete kitfox;

    return 0;
}
