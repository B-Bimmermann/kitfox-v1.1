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
    printf("Processing Compact Microfluidics test using config/microfluidics.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/microfluidics.config");

    thermal_component package;
    thermal_component proc_left_bottom, proc_right_bottom, proc_left_top, proc_right_top;
    thermal_component mem_left_bottom, mem_right_bottom, mem_left_top, mem_right_top;

    // Following are ther power numbers from a Compact Microfluidics example
    proc_left_bottom.power.total = 10.0;
    proc_right_bottom.power.total = 20.0;
    proc_left_top.power.total = 30.0;
    proc_right_top.power.total = 40.0;
    
    mem_left_bottom.power.total = 6.75;
    mem_right_bottom.power.total = 6.75;
    mem_left_top.power.total = 6.75;
    mem_right_top.power.total = 6.75;

    Second sampling_interval = 1.0; // Sampling interval
    Second current_time = sampling_interval;

    // Get pseudo component IDs
    package.id = kitfox->get_component_id("package");
    proc_left_bottom.id = kitfox->get_component_id("package.proc.left_bottom");
    proc_right_bottom.id = kitfox->get_component_id("package.proc.right_bottom");
    proc_left_top.id = kitfox->get_component_id("package.proc.left_top");
    proc_right_top.id = kitfox->get_component_id("package.proc.right_top");
    mem_left_bottom.id = kitfox->get_component_id("package.mem.left_bottom");
    mem_right_bottom.id = kitfox->get_component_id("package.mem.right_bottom");
    mem_left_top.id = kitfox->get_component_id("package.mem.left_top");
    mem_right_top.id = kitfox->get_component_id("package.mem.right_top");

    assert(package.id != INVALID_COMP_ID);
    assert(proc_left_bottom.id != INVALID_COMP_ID);
    assert(proc_right_bottom.id != INVALID_COMP_ID);
    assert(proc_left_top.id != INVALID_COMP_ID);
    assert(proc_right_top.id != INVALID_COMP_ID);
    assert(mem_left_bottom.id != INVALID_COMP_ID);
    assert(mem_right_bottom.id != INVALID_COMP_ID);
    assert(mem_left_top.id != INVALID_COMP_ID);
    assert(mem_right_top.id != INVALID_COMP_ID);

    // Manually insert floorplan powers
    assert(kitfox->push_data(proc_left_bottom.id, current_time, sampling_interval, KITFOX_DATA_POWER, &proc_left_bottom.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(proc_right_bottom.id, current_time, sampling_interval, KITFOX_DATA_POWER, &proc_right_bottom.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(proc_left_top.id, current_time, sampling_interval, KITFOX_DATA_POWER, &proc_left_top.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(proc_right_top.id, current_time, sampling_interval, KITFOX_DATA_POWER, &proc_right_top.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(mem_left_bottom.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_left_bottom.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(mem_right_bottom.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_right_bottom.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(mem_left_top.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_left_top.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(mem_right_top.id, current_time, sampling_interval, KITFOX_DATA_POWER, &mem_right_top.power) == KITFOX_QUEUE_ERROR_NONE);

    // Calculate temperature
    kitfox->calculate_temperature(package.id, current_time, sampling_interval);

    // After synchronizing the pseudo component hierarchy, any pseudo components can be probed.
    assert(kitfox->pull_data(package.id, current_time, sampling_interval, KITFOX_DATA_POWER, &package.power) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(proc_left_bottom.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &proc_left_bottom.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(proc_right_bottom.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &proc_right_bottom.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(proc_left_top.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &proc_left_top.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(proc_right_top.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &proc_right_top.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(mem_left_bottom.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_left_bottom.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(mem_right_bottom.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_right_bottom.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(mem_left_top.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_left_top.temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->pull_data(mem_right_top.id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &mem_right_top.temperature) == KITFOX_QUEUE_ERROR_NONE);

    printf("package.power.total = %3.2lfW\n", package.power.get_total());
    printf("package.proc.left_bottom.temperature = %3.3lfK\n", proc_left_bottom.temperature);
    printf("package.proc.right_bottom.temperature = %3.3lfK\n", proc_right_bottom.temperature);
    printf("package.proc.left_top.temperature = %3.3lfK\n", proc_left_top.temperature);
    printf("package.proc.right_top.temperature = %3.3lfK\n", proc_right_top.temperature);
    printf("package.mem.left_bottom.temperature = %3.3lfK\n", mem_left_bottom.temperature);
    printf("package.mem.right_bottom.temperature = %3.3lfK\n", mem_right_bottom.temperature);
    printf("package.mem.left_top.temperature = %3.3lfK\n", mem_left_top.temperature);
    printf("package.mem.right_top.temperature = %3.3lfK\n", mem_right_top.temperature);

    delete kitfox;
    
    return 0;
}

