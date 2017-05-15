#include <iostream>
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

int main(int argc, char **argv) {
    printf("Processing DRAMsim test using config/dramsim.config ... \n");

    // Initialize KitFox
    kitfox_t *kitfox = new kitfox_t();
    kitfox->configure((char*)"config/dramsim.config");

    // Get pseudo component ID
    Comp_ID rank0_id = kitfox->get_component_id("memory.rank0");
    assert(rank0_id != INVALID_COMP_ID);

    // Some fake counters for testing
    counter_t rank0_counter;
    rank0_counter.read = 488;
    rank0_counter.write = 34;
    rank0_counter.precharge = 139;
    rank0_counter.refresh = 1;
    rank0_counter.background_open_page_high = 2816;
    rank0_counter.background_closed_page_high = 413;
    rank0_counter.background_closed_page_low = 6771;

    Second sampling_interval = (double)10000/666666666; // 10,000 cycles and 666MHz clock
    Second current_time = sampling_interval;

    // Calculate power
    power_t rank0_power;
    kitfox->calculate_power(rank0_id, current_time, sampling_interval, rank0_counter);
    assert(kitfox->pull_data(rank0_id, current_time, sampling_interval, KITFOX_DATA_POWER, &rank0_power) == KITFOX_QUEUE_ERROR_NONE);
    printf("Rank0: power = %3.3lfW\n", rank0_power.get_total());

    delete kitfox;
}
