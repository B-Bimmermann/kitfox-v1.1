#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

#define TRACE_LENGTH 5

struct thermal_component {
    Comp_ID id;
    power_t power[TRACE_LENGTH];
    Kelvin temperature[TRACE_LENGTH];
};

int main(int argc, char **argv) {
    printf("Processing 3D-ICE test using config/3d-ice.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/3d-ice.config");

    thermal_component package, memory_die, core_die;
    thermal_component mem_background0, mem_background1, mem_hotspot0, mem_hotspot1; 
    thermal_component core0, core1, core2, core3, core4, core5, core6, core7, cache0, cache1, clk, fpu, crossbar;

    // Following are the power numbers from a 3D-ICE example
    mem_background0.power[0].total = 12.5; mem_background1.power[0].total = 12.5; 
    mem_background0.power[1].total = 14.0; mem_background1.power[1].total = 14.0;
    mem_background0.power[2].total = 10.5; mem_background1.power[2].total = 10.5;
    mem_background0.power[3].total = 13.5; mem_background1.power[3].total = 13.5;
    mem_background0.power[4].total = 12.5; mem_background1.power[4].total = 12.5; 

    mem_hotspot0.power[0].total = 25.0; mem_hotspot1.power[0].total = 25.0;
    mem_hotspot0.power[1].total = 22.0; mem_hotspot1.power[1].total = 22.0;
    mem_hotspot0.power[2].total = 24.0; mem_hotspot1.power[2].total = 24.0;
    mem_hotspot0.power[3].total = 28.5; mem_hotspot1.power[3].total = 28.5;
    mem_hotspot0.power[4].total = 25.0; mem_hotspot1.power[4].total = 25.0;

    core0.power[0].total = 12.5; core1.power[0].total = 12.5;
    core0.power[1].total = 14.0; core1.power[1].total = 14.0;
    core0.power[2].total = 10.5; core1.power[2].total = 10.5;
    core0.power[3].total = 13.5; core1.power[3].total = 13.5;
    core0.power[4].total = 12.5; core1.power[4].total = 12.5;

    core2.power[0].total = 12.5; core3.power[0].total = 12.5;
    core2.power[1].total = 14.0; core3.power[1].total = 14.0;
    core2.power[2].total = 10.5; core3.power[2].total = 10.5;
    core2.power[3].total = 13.5; core3.power[3].total = 13.5;
    core2.power[4].total = 12.5; core3.power[4].total = 12.5;

    core4.power[0].total = 12.5; core5.power[0].total = 12.5;
    core4.power[1].total = 14.0; core5.power[1].total = 14.0;
    core4.power[2].total = 10.5; core5.power[2].total = 10.5;
    core4.power[3].total = 13.5; core5.power[3].total = 13.5;
    core4.power[4].total = 12.5; core5.power[4].total = 12.5;

    core6.power[0].total = 12.5; core7.power[0].total = 12.5;
    core6.power[1].total = 14.0; core7.power[1].total = 14.0;
    core6.power[2].total = 10.5; core7.power[2].total = 10.5;
    core6.power[3].total = 13.5; core7.power[3].total = 13.5;
    core6.power[4].total = 12.5; core7.power[4].total = 12.5;

    cache0.power[0].total = 12.5; cache1.power[0].total = 12.5;
    cache0.power[1].total = 14.0; cache1.power[1].total = 14.0;
    cache0.power[2].total = 10.5; cache1.power[2].total = 10.5;
    cache0.power[3].total = 13.5; cache1.power[3].total = 13.5;
    cache0.power[4].total = 12.5; cache1.power[4].total = 12.5;

    clk.power[0].total = 12.5;
    clk.power[1].total = 14.0;
    clk.power[2].total = 10.5;
    clk.power[3].total = 13.5;
    clk.power[4].total = 12.5;

    fpu.power[0].total = 12.5;
    fpu.power[1].total = 14.0;
    fpu.power[2].total = 10.5;
    fpu.power[3].total = 13.5;
    fpu.power[4].total = 12.5;

    crossbar.power[0].total = 12.5;
    crossbar.power[1].total = 14.0;
    crossbar.power[2].total = 10.5;
    crossbar.power[3].total = 13.5;
    crossbar.power[4].total = 12.5;

    // Get pseudo component IDs
    package.id = kitfox->get_component_id("package");
    memory_die.id = kitfox->get_component_id("package.memory_die");
    core_die.id = kitfox->get_component_id("package.core_die");
    mem_background0.id = kitfox->get_component_id("package.memory_die.mem_background0");
    mem_background1.id = kitfox->get_component_id("package.memory_die.mem_background1");
    mem_hotspot0.id = kitfox->get_component_id("package.memory_die.mem_hotspot0");
    mem_hotspot1.id = kitfox->get_component_id("package.memory_die.mem_hotspot1");
    core0.id = kitfox->get_component_id("package.core_die.core0");
    core1.id = kitfox->get_component_id("package.core_die.core1");
    core2.id = kitfox->get_component_id("package.core_die.core2");
    core3.id = kitfox->get_component_id("package.core_die.core3");
    core4.id = kitfox->get_component_id("package.core_die.core4");
    core5.id = kitfox->get_component_id("package.core_die.core5");
    core6.id = kitfox->get_component_id("package.core_die.core6");
    core7.id = kitfox->get_component_id("package.core_die.core7");
    cache0.id = kitfox->get_component_id("package.core_die.cache0");
    cache1.id = kitfox->get_component_id("package.core_die.cache1");
    clk.id = kitfox->get_component_id("package.core_die.clk");
    fpu.id = kitfox->get_component_id("package.core_die.fpu");
    crossbar.id = kitfox->get_component_id("package.core_die.crossbar");

    assert(package.id != INVALID_COMP_ID);
    assert(memory_die.id != INVALID_COMP_ID);
    assert(core_die.id != INVALID_COMP_ID);
    assert(mem_background0.id != INVALID_COMP_ID);
    assert(mem_background1.id != INVALID_COMP_ID);
    assert(mem_hotspot0.id != INVALID_COMP_ID);
    assert(mem_hotspot1.id != INVALID_COMP_ID);
    assert(core0.id != INVALID_COMP_ID);
    assert(core1.id != INVALID_COMP_ID);
    assert(core2.id != INVALID_COMP_ID);
    assert(core3.id != INVALID_COMP_ID);
    assert(core4.id != INVALID_COMP_ID);
    assert(core5.id != INVALID_COMP_ID);
    assert(core6.id != INVALID_COMP_ID);
    assert(core7.id != INVALID_COMP_ID);
    assert(cache0.id != INVALID_COMP_ID);
    assert(cache1.id != INVALID_COMP_ID);
    assert(clk.id != INVALID_COMP_ID);
    assert(fpu.id != INVALID_COMP_ID);
    assert(crossbar.id != INVALID_COMP_ID);

    for(unsigned i = 0; i < TRACE_LENGTH; i++) {
        Second sampling_interval = 0.02;
        Second current_time = sampling_interval*(Second)(i+1);
        printf("current time = %2.0lfms, sampling interval = %2.0lfms:\n", current_time*1e3, sampling_interval*1e3);
        // Manually insert floorplan powers
        kitfox->push_data(mem_background0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_background0.power[i]);
        kitfox->push_data(mem_background1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_background1.power[i]);
        kitfox->push_data(mem_hotspot0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_hotspot0.power[i]);
        kitfox->push_data(mem_hotspot1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_hotspot1.power[i]);
        kitfox->push_data(core0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core0.power[i]);
        kitfox->push_data(core1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core1.power[i]);
        kitfox->push_data(core2.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core2.power[i]);
        kitfox->push_data(core3.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core3.power[i]);
        kitfox->push_data(core4.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core4.power[i]);
        kitfox->push_data(core5.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core5.power[i]);
        kitfox->push_data(core6.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core6.power[i]);
        kitfox->push_data(core7.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core7.power[i]);
        kitfox->push_data(cache0.id, current_time, sampling_interval, KITFOX_DATA_POWER, &cache0.power[i]);
        kitfox->push_data(cache1.id, current_time, sampling_interval, KITFOX_DATA_POWER, &cache1.power[i]);
        kitfox->push_data(clk.id, current_time, sampling_interval, KITFOX_DATA_POWER, &clk.power[i]);
        kitfox->push_data(fpu.id, current_time, sampling_interval, KITFOX_DATA_POWER, &fpu.power[i]);
        kitfox->push_data(crossbar.id, current_time, sampling_interval, KITFOX_DATA_POWER, &crossbar.power[i]);

        // Calculate temperature
        kitfox->calculate_temperature(package.id, current_time, sampling_interval);

        // After synchronizing the pseudo component hierarchy, any pseudo components can be probed.
        assert(kitfox->pull_data(package.id, current_time, sampling_interval, KITFOX_DATA_POWER, &package.power[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(memory_die.id, current_time, sampling_interval, KITFOX_DATA_POWER, &memory_die.power[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core_die.id, current_time, sampling_interval, KITFOX_DATA_POWER, &core_die.power[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(mem_background0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_background0.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(mem_background1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_background1.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(mem_hotspot0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_hotspot0.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(mem_hotspot1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_hotspot1.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core0.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core1.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core2.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core2.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core3.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core3.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core4.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core4.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core5.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core5.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core6.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core6.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(core7.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &core7.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(cache0.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &cache0.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(cache1.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &cache1.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(clk.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &clk.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(fpu.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &fpu.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);
        assert(kitfox->pull_data(crossbar.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &crossbar.temperature[i]) == KITFOX_QUEUE_ERROR_NONE);

        printf("package.power.total = %3.2lfW\n", package.power[i].get_total());
        printf("package.memory_die.power.total = %3.3lfW\n", memory_die.power[i].get_total());
        printf("package.core_die.power.total = %3.3lfW\n", core_die.power[i].get_total());
        printf("package.memory_die.mem_background0.temperature = %3.2lfK\n", mem_background0.temperature[i]);
        printf("package.memory_die.mem_background1.temperature = %3.2lfK\n", mem_background1.temperature[i]);
        printf("package.memory_die.mem_hotspot0.temperature = %3.2lfK\n", mem_hotspot0.temperature[i]);
        printf("package.memory_die.mem_hotspot1.temperature = %3.2lfK\n", mem_hotspot1.temperature[i]);
        printf("package.core_die.core0.temperature = %3.2lfK\n", core0.temperature[i]);
        printf("package.core_die.core1.temperature = %3.2lfK\n", core1.temperature[i]);
        printf("package.core_die.core2.temperature = %3.2lfK\n", core2.temperature[i]);
        printf("package.core_die.core3.temperature = %3.2lfK\n", core3.temperature[i]);
        printf("package.core_die.core4.temperature = %3.2lfK\n", core4.temperature[i]);
        printf("package.core_die.core5.temperature = %3.2lfK\n", core5.temperature[i]);
        printf("package.core_die.core6.temperature = %3.2lfK\n", core6.temperature[i]);
        printf("package.core_die.core7.temperature = %3.2lfK\n", core7.temperature[i]);
        printf("package.core_die.cache0.temperature = %3.2lfK\n", cache0.temperature[i]);
        printf("package.core_die.cache1.temperature = %3.2lfK\n", cache1.temperature[i]);
        printf("package.core_die.clk.temperature = %3.2lfK\n", clk.temperature[i]);
        printf("package.core_die.fpu.temperature = %3.2lfK\n", fpu.temperature[i]);
        printf("package.core_die.crossbar.temperature = %3.2lfK\n", crossbar.temperature[i]);
        
        if(i < TRACE_LENGTH-1) printf("\n");
        
        /*
        // Print thermal grid
        grid_t<Kelvin> thermal_grid;
        assert(kitfox->pull_data(package.id, current_time, sampling_interval, KITFOX_DATA_THERMAL_GRID, &thermal_grid) == KITFOX_QUEUE_ERROR_NONE);

        for(unsigned z = 0; z < thermal_grid.dies(); z++) {
            printf("die %d:\n", z);
            for(unsigned x = 0; x < thermal_grid.rows(); x++) {
                for(unsigned y = 0; y < thermal_grid.cols(); y++) {
                    printf("%3.2lf ", thermal_grid(x, y, z));
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("\n");
        */
    }

    delete kitfox;

    return 0;
}
