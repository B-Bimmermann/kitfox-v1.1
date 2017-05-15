#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

int main(int argc, char** argv) {
    printf("Processing IntSim test using config/intsim.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/intsim.config");

    // Get pseudo component ID
    Comp_ID chip_id = kitfox->get_component_id("chip");
    assert(chip_id != INVALID_COMP_ID);

    // Some fake counters for testing
    Hertz clock_frequency = 2e9;
    double activity_factor = 0.1;
    
    counter_t chip_counter;
    chip_counter.switching = activity_factor*(double)clock_frequency;

    Second sampling_interval = 1.0;
    Second current_time = sampling_interval;

    // Retrieve area information
    dimension_t chip_dimension;
    assert(kitfox->pull_data(chip_id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_DIMENSION, &chip_dimension) == KITFOX_QUEUE_ERROR_NONE);
    printf("Chip: area = %3.3lfmm^2\n", chip_dimension.get_area()*1e6);
    
    // Calculate power
    power_t chip_power;
    kitfox->calculate_power(chip_id, current_time, sampling_interval, chip_counter);
    assert(kitfox->pull_data(chip_id, current_time, sampling_interval, KITFOX_DATA_POWER, &chip_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Chip: dynamic power = %3.2lfW, leakage power = %2.2lfW\n", chip_power.dynamic, chip_power.leakage);
    
    // Voltage scaling
    Volt chip_voltage = 1.2;
    assert(kitfox->push_data(chip_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &chip_voltage) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchronize_data(chip_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &chip_voltage) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->update_library_variable(chip_id, KITFOX_DATA_VOLTAGE &chip_voltage));

    current_time += sampling_interval;
    kitfox->calculate_power(chip_id, current_time, sampling_interval, chip_counter);
    assert(kitfox->pull_data(chip_id, current_time, sampling_interval, KITFOX_DATA_POWER, &chip_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Chip: dynamic power = %3.2lfW, leakage power = %2.2lfW afer voltage scaling\n", chip_power.dynamic, chip_power.leakage);
    
    // Frequency scaling
    Hertz chip_frequency = 4.0e9;
    //assert(kitfox->push_data(chip_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &chip_frequency) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_and_synchronize_data(chip_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &chip_frequency) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->update_library_variable(chip_id, KITFOX_DATA_CLOCK_FREQUENCY, &chip_frequency));

    current_time += sampling_interval;
    chip_counter.switching = chip_frequency*activity_factor;
    kitfox->calculate_power(chip_id, current_time, sampling_interval, chip_counter);
    assert(kitfox->pull_data(chip_id, current_time, sampling_interval, KITFOX_DATA_POWER, &chip_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Chip: dynamic power = %3.2lfW, leakage power = %2.2lfW after frequency scaling\n", chip_power.dynamic, chip_power.leakage);

    // Temperature feedback
    Kelvin chip_temperature = 380.0;
    //assert(kitfox->push_data(chip_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &chip_temperature) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchronize_data(chip_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &chip_temperature) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->update_library_variable(chip_id, KITFOX_DATA_TEMPERATURE, &chip_temperature));

    current_time += sampling_interval;
    kitfox->calculate_power(chip_id, current_time, sampling_interval, chip_counter);
    assert(kitfox->pull_data(chip_id, current_time, sampling_interval, KITFOX_DATA_POWER, &chip_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Chip: dynamic power = %3.2lfW, leakage power = %2.2lfW after temperature feedback\n", chip_power.dynamic, chip_power.leakage);

    delete kitfox;

    return 0;
}
