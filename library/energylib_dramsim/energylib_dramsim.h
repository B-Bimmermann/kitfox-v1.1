#ifndef __ENERGYLIB_DRAMSIM_H__
#define __ENERGYLIB_DRAMSIM_H__

#include "library.h"

/*
#include "DRAMSim2-2.2.2/AddressMapping.h"
#include "DRAMSim2-2.2.2/Bank.h"
#include "DRAMSim2-2.2.2/BankState.h"
#include "DRAMSim2-2.2.2/BusPacket.h"
#include "DRAMSim2-2.2.2/Callback.h"
#include "DRAMSim2-2.2.2/ClockDomain.h"
#include "DRAMSim2-2.2.2/CommandQueue.h"
#include "DRAMSim2-2.2.2/CSVWriter.h"
//#include "DRAMSim2-2.2.2/DRAMSim.h"
#include "DRAMSim2-2.2.2/IniReader.h"
#include "DRAMSim2-2.2.2/MemoryController.h"
#include "DRAMSim2-2.2.2/MemorySystem.h"
#include "DRAMSim2-2.2.2/MultiChannelMemorySystem.h"
#include "DRAMSim2-2.2.2/PrintMacros.h"
#include "DRAMSim2-2.2.2/Rank.h"
#include "DRAMSim2-2.2.2/SimulatorObject.h"
#include "DRAMSim2-2.2.2/SystemConfiguration.h"
#include "DRAMSim2-2.2.2/Transaction.h"
*/

namespace libKitFox {
namespace libKitFox_DRAMSIM {

enum DRAMSIM_PAGE_POLICY_TYPES {
    KITFOX_DRAMSIM_PAGE_POLICY_UNKNOWN = 0,
    KITFOX_DRAMSIM_CLOSE_PAGE_POLICY,
    KITFOX_DRAMSIM_OPEN_PAGE_POLICY,
    NUM_KITFOX_DRAMSIM_PAGE_POLICY_TYPES
};

class energylib_dramsim : public energy_library_t {
public:
    energylib_dramsim(pseudo_component_t *PseudoCompnent);
    ~energylib_dramsim();
    
    virtual void initialize();
    virtual unit_energy_t get_unit_energy();
    virtual power_t get_tdp_power(Kelvin MaxTemperature = 373.0);
    virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter);
    virtual MeterSquare get_area();
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    struct {
        unsigned IDD0;
        unsigned IDD1;
        unsigned IDD2P;
        unsigned IDD2Q;
        unsigned IDD2N;
        unsigned IDD3Pf;
        unsigned IDD3Ps;
        unsigned IDD3N;
        unsigned IDD4W;
        unsigned IDD4R;
        unsigned IDD5;
        unsigned IDD6;
        unsigned IDD6L;
        unsigned IDD7;
        
        unsigned BL;
        unsigned DEVICE_WIDTH;
        unsigned DATA_BUS_BITS;
        unsigned NUM_DEVICES;
        
        unsigned tRC;
        unsigned tRAS;
        unsigned tRFC;
        
        unsigned page_policy;
    } dram;
    
    double voltage;
    double clock_frequency;
    double energy_scaling;
    double area_scaling;
    double scaling;
};
    
}
}

#endif
