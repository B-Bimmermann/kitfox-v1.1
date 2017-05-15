#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

int main(int argc, char **argv) {
    printf("Processing TSV test using config/tsv.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/tsv.config");

    // Get pseudo component ID
    Comp_ID tsv0_id = kitfox->get_component_id("package.tsv0");
    assert(tsv0_id != INVALID_COMP_ID);

    // Some fake counters for testing
    counter_t tsv0_counter;
    tsv0_counter.switching = 1e6;

    Second sampling_interval = 1.0;
    Second current_time = sampling_interval;

    // Calculate power
    power_t tsv0_power;
    kitfox->calculate_power(tsv0_id, current_time, sampling_interval, tsv0_counter);
    assert(kitfox->pull_data(tsv0_id, current_time, sampling_interval, KITFOX_DATA_POWER, &tsv0_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("TSV0: power = %3.3lfW\n", tsv0_power.get_total());

    delete kitfox;
}
