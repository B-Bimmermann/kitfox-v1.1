#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

int main(int argc, char **argv) {
    printf("Processing Failure test using config/failure.config ... \n");

    // Initialize Failure
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/failure.config");

    // Get pseudo component IDs
    Comp_ID floorplan_id = kitfox->get_component_id("floorplan");
    assert(floorplan_id != INVALID_COMP_ID);

    // Some fake stress conditions for testing
    Kelvin temperature = 338.0;
    Volt voltage = 1.0;
    Hertz clock_frequency = 2.0e9;

    Second sampling_interval = 1.0; // Sampling interval
    Second current_time = sampling_interval;

    // Voltage and clock frequency are initially stored in the queue
    //assert(kitfox->push_data(floorplan_id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_VOLTAGE, &voltage) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_data(floorplan_id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_CLOCK_FREQUENCY, &clock_frequency) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_data(floorplan_id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &temperature) == KITFOX_QUEUE_ERROR_NONE);

    // Test the floorplan failure model
    Unitless failure_rate;
    kitfox->calculate_failure_rate(floorplan_id, current_time, sampling_interval, false);
    assert(kitfox->pull_data(floorplan_id, current_time, sampling_interval, KITFOX_DATA_FAILURE_RATE, &failure_rate) == KITFOX_QUEUE_ERROR_NONE);
    failure_rate /= sampling_interval;

    printf("floorplan.failure_rate = %3.3e\n", failure_rate);
    printf("floorplan.MTTF = %3.3e seconds\n", 1.0/failure_rate);
    printf("\n");

    // Voltage scaling; voltage is stored in an open queue
    voltage = 1.2;
    assert(kitfox->push_data(floorplan_id, current_time, UNSPECIFIED_TIME, KITFOX_DATA_VOLTAGE, &voltage) == KITFOX_QUEUE_ERROR_NONE);

    current_time += sampling_interval;
    // Assume a constant temperature; temperature is stored in a closed queue
    assert(kitfox->push_data(floorplan_id, current_time, sampling_interval, KITFOX_DATA_TEMPERATURE, &temperature) == KITFOX_QUEUE_ERROR_NONE);

    kitfox->calculate_failure_rate(floorplan_id, current_time, sampling_interval, false);
    assert(kitfox->pull_data(floorplan_id, current_time, sampling_interval, KITFOX_DATA_FAILURE_RATE, &failure_rate) == KITFOX_QUEUE_ERROR_NONE);
    failure_rate /= sampling_interval;

    printf("floorplan.failure_rate = %3.3e after voltage scaling\n", failure_rate);
    printf("floorplan.MTTF = %3.3e seconds after voltage scaling\n", 1.0/failure_rate);

    delete kitfox;

    return 0;
}
