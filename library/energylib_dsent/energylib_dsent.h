#ifndef __ENERGYLIB_DSENT_H__
#define __ENERGYLIB_DSENT_H__

#include "library.h"

#include "dsent0.91/libutil/LibUtil.h"
#include "dsent0.91/libutil/MathUtil.h"
#include "dsent0.91/libutil/OptionParser.h"
#include "dsent0.91/libutil/libutilAssert.h"
#include "dsent0.91/libutil/libutilCalculator.h"
#include "dsent0.91/libutil/libutilConfig.h"
#include "dsent0.91/libutil/libutilException.h"
#include "dsent0.91/libutil/libutilLog.h"
#include "dsent0.91/libutil/libutilMap.h"
#include "dsent0.91/libutil/libutilString.h"

#include "dsent0.91/util/CommonType.h"
#include "dsent0.91/util/Config.h"
#include "dsent0.91/util/Constants.h"
#include "dsent0.91/util/Result.h"

#include "dsent0.91/tech/TechModel.h"

#include "dsent0.91/model/ElectricalModel.h"
#include "dsent0.91/model/EventInfo.h"
#include "dsent0.91/model/Model.h"
#include "dsent0.91/model/ModelGen.h"
#include "dsent0.91/model/OpticalModel.h"
#include "dsent0.91/model/PortInfo.h"
#include "dsent0.91/model/TransitionInfo.h"

#include "dsent0.91/model/electrical/BarrelShifter.h"
#include "dsent0.91/model/electrical/BroadcastHTree.h"
#include "dsent0.91/model/electrical/DFFRAM.h"
#include "dsent0.91/model/electrical/Decoder.h"
#include "dsent0.91/model/electrical/DemuxTreeDeserializer.h"
#include "dsent0.91/model/electrical/MatrixArbiter.h"
#include "dsent0.91/model/electrical/Multiplexer.h"
#include "dsent0.91/model/electrical/MultiplexerCrossbar.h"
#include "dsent0.91/model/electrical/MuxTreeSerializer.h"
#include "dsent0.91/model/electrical/OR.h"
#include "dsent0.91/model/electrical/RepeatedLink.h"
#include "dsent0.91/model/electrical/RippleAdder.h"
#include "dsent0.91/model/electrical/SeparableAllocator.h"
#include "dsent0.91/model/electrical/TestModel.h"

#include "dsent0.91/model/network/ElectricalClos.h"
#include "dsent0.91/model/network/ElectricalMesh.h"
#include "dsent0.91/model/network/PhotonicClos.h"

#include "dsent0.91/model/optical/GatedLaserSource.h"
#include "dsent0.91/model/optical/LaserSource.h"
#include "dsent0.91/model/optical/OpticalLinkBackendRx.h"
#include "dsent0.91/model/optical/OpticalLinkBackendTx.h"
#include "dsent0.91/model/optical/OpticalTestModel.h"
#include "dsent0.91/model/optical/RingDetector.h"
#include "dsent0.91/model/optical/RingFilter.h"
#include "dsent0.91/model/optical/RingModulator.h"
#include "dsent0.91/model/optical/SWMRLink.h"
#include "dsent0.91/model/optical/SWSRLink.h"
#include "dsent0.91/model/optical/ThrottledLaserSource.h"

#include "dsent0.91/model/optical_graph/OpticalDetector.h"
#include "dsent0.91/model/optical_graph/OpticalFilter.h"
#include "dsent0.91/model/optical_graph/OpticalGraph.h"
#include "dsent0.91/model/optical_graph/OpticalLaser.h"
#include "dsent0.91/model/optical_graph/OpticalModulator.h"
#include "dsent0.91/model/optical_graph/OpticalNode.h"
#include "dsent0.91/model/optical_graph/OpticalReceiver.h"
#include "dsent0.91/model/optical_graph/OpticalTransmitter.h"
#include "dsent0.91/model/optical_graph/OpticalWaveguide.h"
#include "dsent0.91/model/optical_graph/OpticalWavelength.h"

#include "dsent0.91/model/std_cells/ADDF.h"
#include "dsent0.91/model/std_cells/AND2.h"
#include "dsent0.91/model/std_cells/BUF.h"
#include "dsent0.91/model/std_cells/CellMacros.h"
#include "dsent0.91/model/std_cells/DFFQ.h"
#include "dsent0.91/model/std_cells/INV.h"
#include "dsent0.91/model/std_cells/LATQ.h"
#include "dsent0.91/model/std_cells/MUX2.h"
#include "dsent0.91/model/std_cells/NAND2.h"
#include "dsent0.91/model/std_cells/NOR2.h"
#include "dsent0.91/model/std_cells/OR2.h"
#include "dsent0.91/model/std_cells/StdCell.h"
#include "dsent0.91/model/std_cells/StdCellLib.h"
#include "dsent0.91/model/std_cells/XOR2.h"

#include "dsent0.91/model/timing_graph/ElectricalDelay.h"
#include "dsent0.91/model/timing_graph/ElectricalDriver.h"
#include "dsent0.91/model/timing_graph/ElectricalDriverMultiplier.h"
#include "dsent0.91/model/timing_graph/ElectricalLoad.h"
#include "dsent0.91/model/timing_graph/ElectricalNet.h"
#include "dsent0.91/model/timing_graph/ElectricalTimingNode.h"
#include "dsent0.91/model/timing_graph/ElectricalTimingOptimizer.h"
#include "dsent0.91/model/timing_graph/ElectricalTimingTree.h"

#include "dsent0.91/DSENT.h"

namespace libKitFox {
namespace libKitFox_DSENT {
    
enum KITFOX_DSENT_ENERGY_MODEL_TYPES {
    KITFOX_DSENT_ENERGY_MODEL_UNKNOWN = 0,
    KITFOX_DSENT_ROUTER,
    KITFOX_DSENT_REPEATED_LINK,
    KITFOX_DSENT_SWSRLINK,
    NUM_KITFOX_DSENT_ENERGY_MODEL_TYPES
};

static const char *KITFOX_DSENT_ENERGY_MODEL_STR[] = {
    "KITFOX_DSENT_ENERGY_MODEL_UNKNOWN",
    "KITFOX_DSENT_ROUTER",
    "KITFOX_DSENT_REPEATED_LINK",
    "KITFOX_DSENT_SWSRLINK",
    "NUM_KITFOX_DSENT_ENERGY_MODEL_TYPES"
};

class energylib_dsent : public energy_library_t {
public:
    energylib_dsent(pseudo_component_t *PseudoComponent);
    ~energylib_dsent();
    
    virtual void initialize();
    virtual unit_energy_t get_unit_energy();
    virtual power_t get_tdp_power(Kelvin MaxTemperature = 373.0);
    virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter);
    virtual MeterSquare get_area();
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    double clock_frequency;
    double energy_scaling;
    double area_scaling;
    double scaling;
    double tdp_injection_rate;
    
    int energy_model;
    
    DSENT::String ConvertString(double v);
    DSENT::String ConvertString(int v);

    //DSENT::DSENTConfig *dsent_config;
    DSENT::TechModel* tech_model;
    DSENT::StdCellLib* std_cell_lib;
    DSENT::Model *ms_model;

    bool IsPerformTimingOptimization;
    std::vector<DSENT::String> start_net_names;
};
    
} // namespace libKitFox_DSENT
} // namespace libKitFox

#endif
