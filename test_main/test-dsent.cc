#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

int main(int argc, char **argv) {
    printf("Processing DSENT test using config/dsent.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/dsent.config");
    
    // Get pseudo component IDs
    Comp_ID router_id = kitfox->get_component_id("package.router");
    Comp_ID electrical_link_id = kitfox->get_component_id("package.electrical_link");
    assert(router_id != INVALID_COMP_ID);
    assert(electrical_link_id != INVALID_COMP_ID);

    // Some fake counters for testing
    Hertz clock_frequency = 1e9;    
    double injection_rate = 0.3;
    int num_ports = 5;
    
    // Test the router and electrical link models
    counter_t router_counter;
    router_counter.read = clock_frequency * injection_rate * (double)num_ports;
    router_counter.write = clock_frequency * injection_rate * (double)num_ports;
    router_counter.switching = clock_frequency * injection_rate * (double)num_ports;

    counter_t electrical_link_counter;
    electrical_link_counter.switching = clock_frequency * injection_rate;

    Second sampling_interval = 1.0; // Sampling interval
    Second current_time = sampling_interval;

    // Acquire estimated area information
    dimension_t router_dimension;
    assert(kitfox->pull_data(router_id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_DIMENSION, &router_dimension) == KITFOX_QUEUE_ERROR_NONE);
    printf("Router: area = %3.3lfmm^2\n", router_dimension.get_area()*1e6);

    dimension_t electrical_link_dimension;
    assert(kitfox->pull_data(electrical_link_id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_DIMENSION, &electrical_link_dimension) == KITFOX_QUEUE_ERROR_NONE);
    printf("Electrical link: area = %3.6lfmm^2\n", electrical_link_dimension.get_area()*1e6);
    printf("\n");

    // Calculate power
    power_t router_power;
    kitfox->calculate_power(router_id, current_time, sampling_interval, router_counter);
    assert(kitfox->pull_data(router_id, current_time, sampling_interval, KITFOX_DATA_POWER, &router_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Router: dynamic power = %3.3lfmW, leakage power = %3.3lfmW\n", router_power.dynamic*1e3, router_power.leakage*1e3);

    power_t electrical_link_power;
    kitfox->calculate_power(electrical_link_id, current_time, sampling_interval, electrical_link_counter);
    assert(kitfox->pull_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_POWER, &electrical_link_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Electrical link: dynamic power = %3.3lfmW, leakage power = %3.3lfmW\n", electrical_link_power.dynamic*1e3, electrical_link_power.leakage*1e3);
    printf("\n");
    
    // Voltage scaling
    Volt router_voltage = 1.2;
    //assertkitfox->update_library_variable(router_id, KITFOX_DATA_VOLTAGE, &router_voltage));
    //assert(kitfox->push_data(router_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &router_voltage) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_and_synchronize_data(router_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &router_voltage) == KITFOX_QUEUE_ERROR_NONE);

    Volt electrical_link_voltage = 1.2;
    //assert(kitfox->update_library_variable(electrical_link_id, KITFOX_DATA_VOLTAGE, &electrical_link_voltage));
    assert(kitfox->push_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &electrical_link_voltage) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchonize_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_VOLTAGE, &electrical_link_voltage) == KITFOX_QUEUE_ERROR_NONE);

    current_time += sampling_interval;

    kitfox->calculate_power(router_id, current_time, sampling_interval, router_counter);
    assert(kitfox->pull_data(router_id, current_time, sampling_interval, KITFOX_DATA_POWER, &router_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Router: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after voltage scaling\n", router_power.dynamic*1e3, router_power.leakage*1e3);

    kitfox->calculate_power(electrical_link_id, current_time, sampling_interval, electrical_link_counter);
    assert(kitfox->pull_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_POWER, &electrical_link_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Electrical link: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after voltage scaling\n", electrical_link_power.dynamic*1e3, electrical_link_power.leakage*1e3);
    printf("\n");

    // Frequency scaling
    Hertz router_frequency = clock_frequency * 2.0;
    router_counter.read = router_frequency * injection_rate * (double)num_ports;
    router_counter.write = router_frequency * injection_rate * (double)num_ports;
    router_counter.switching = router_frequency * injection_rate * (double)num_ports;

    Hertz electrical_link_frequency = clock_frequency * 2.0;
    electrical_link_counter.switching = electrical_link_frequency * injection_rate;

    assert(kitfox->update_library_variable(router_id, KITFOX_DATA_CLOCK_FREQUENCY, &router_frequency));
    //assert(kitfox->push_data(router_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &router_frequency) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchronize_data(router_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &router_frequency) == KITFOX_QUEUE_ERROR_NONE);

    //assert(kitfox->update_library_variable(electrical_link_id, KITFOX_DATA_CLOCK_FREQUENCY, &electrical_link_frequency));
    //assert(kitfox->push_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &electrical_link_frequency) == KITFOX_QUEUE_ERROR_NONE);
    assert(kitfox->push_and_synchronize_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_CLOCK_FREQUENCY, &electrical_link_frequency) == KITFOX_QUEUE_ERROR_NONE);

    current_time += sampling_interval;

    kitfox->calculate_power(router_id, current_time, sampling_interval, router_counter);
    assert(kitfox->pull_data(router_id, current_time, sampling_interval, KITFOX_DATA_POWER, &router_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Router: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after frequency scaling\n", router_power.dynamic*1e3, router_power.leakage*1e3);

    kitfox->calculate_power(electrical_link_id, current_time, sampling_interval, electrical_link_counter);
    assert(kitfox->pull_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_POWER, &electrical_link_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Electrical link: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after frequency scaling\n", electrical_link_power.dynamic*1e3, electrical_link_power.leakage*1e3);
    printf("\n");

    // Temperature feedback
    Kelvin router_temperature = 380.0;
    //assert(kitfox->update_library_variable(router_id, KITFOX_DATA_TEMPERATURE, &router_temperature));
    assert(kitfox->push_data(router_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &router_temperature) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchronize_data(router_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &router_temperature) == KITFOX_QUEUE_ERROR_NONE);

    Kelvin electrical_link_temperature = 380.0;
    assert(kitfox->update_library_variable(electrical_link_id, KITFOX_DATA_TEMPERATURE, &electrical_link_temperature));
    //assert(kitfox->push_data(electrical_link_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &electrical_link_temperature) == KITFOX_QUEUE_ERROR_NONE);
    //assert(kitfox->push_and_synchronize_data(electrical_link_id, current_time, current_time, KITFOX_DATA_TEMPERATURE, &electrical_link_temperature) == KITFOX_QUEUE_ERROR_NONE);

    current_time += sampling_interval;

    kitfox->calculate_power(router_id, current_time, sampling_interval, router_counter);
    assert(kitfox->pull_data(router_id, current_time, sampling_interval, KITFOX_DATA_POWER, &router_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Router: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after temperature feedback\n", router_power.dynamic*1e3, router_power.leakage*1e3);
    
    kitfox->calculate_power(electrical_link_id, current_time, sampling_interval, electrical_link_counter);
    assert(kitfox->pull_data(electrical_link_id, current_time, sampling_interval, KITFOX_DATA_POWER, &electrical_link_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Electrical link: dynamic power = %3.3lfmW, leakage power = %3.3lfmW after temperature feedback\n", electrical_link_power.dynamic*1e3, electrical_link_power.leakage*1e3);

    delete kitfox;

    return 0;
}
