#ifndef __ENERGYLIB_MCPAT_H__
#define __ENERGYLIB_MCPAT_H__

#include "library.h"

#include "McPAT08release/arch_const.h"
#include "McPAT08release/basic_components.h"
#include "McPAT08release/core.h"
#include "McPAT08release/globalvar.h"		
#include "McPAT08release/interconnect.h"
#include "McPAT08release/iocontrollers.h"
#include "McPAT08release/logic.h"
#include "McPAT08release/memoryctrl.h"
#include "McPAT08release/noc.h"
#include "McPAT08release/processor.h"
#include "McPAT08release/sharedcache.h"
#include "McPAT08release/version.h"
#include "McPAT08release/XML_Parse.h"
#include "McPAT08release/xmlParser.h"
#include "McPAT08release/array.h"

#include "McPAT08release/cacti/arbiter.h"
#include "McPAT08release/cacti/area.h"
#include "McPAT08release/cacti/bank.h"
#include "McPAT08release/cacti/basic_circuit.h"
#include "McPAT08release/cacti/cacti_interface.h"
#include "McPAT08release/cacti/component.h"
#include "McPAT08release/cacti/const.h"
#include "McPAT08release/cacti/crossbar.h"
#include "McPAT08release/cacti/decoder.h"
//#include "McPAT08release/cacti/highradix.h"
#include "McPAT08release/cacti/htree2.h"
#include "McPAT08release/cacti/io.h"
#include "McPAT08release/cacti/mat.h"
#include "McPAT08release/cacti/nuca.h"
#include "McPAT08release/cacti/parameter.h"
#include "McPAT08release/cacti/router.h"
#include "McPAT08release/cacti/subarray.h"
#include "McPAT08release/cacti/Ucache.h"
#include "McPAT08release/cacti/uca.h"
#include "McPAT08release/cacti/wire.h"

namespace libKitFox {
namespace libKitFox_MCPAT {

enum KITFOX_MCPAT_ENERGY_MODEL_TYPES {
    KITFOX_MCPAT_ENERGY_MODEL_UNKNOWN = 0,
    KITFOX_MCPAT_ARRAYST,
    KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK,
    KITFOX_MCPAT_FLASH_CONTROLLER,
    KITFOX_MCPAT_FUNCTIONAL_UNIT,
    KITFOX_MCPAT_INST_DECODER,
    KITFOX_MCPAT_INTERCONNECT,
    KITFOX_MCPAT_MEMORY_CONTROLLER,
    KITFOX_MCPAT_NIU_CONTROLLER,
    KITFOX_MCPAT_NOC,
    KITFOX_MCPAT_PCIE_CONTROLLER,
    KITFOX_MCPAT_PIPELINE,
    KITFOX_MCPAT_SELECTION_LOGIC,
    KITFOX_MCPAT_UNDIFF_CORE,
    NUM_KITFOX_MCPAT_ENERGY_MODEL_TYPES
};

static const char *KITFOX_MCPAT_ENERGY_MODEL_STR[] = {
    "KITFOX_MCPAT_ENERGY_MODEL_UNKNOWN",
    "KITFOX_MCPAT_ARRAYST",
    "KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK",
    "KITFOX_MCPAT_FLASH_CONTROLLER",
    "KITFOX_MCPAT_FUNCTIONAL_UNIT",
    "KITFOX_MCPAT_INST_DECODER",
    "KITFOX_MCPAT_INTERCONNECT",
    "KITFOX_MCPAT_MEMORY_CONTROLLER",
    "KITFOX_MCPAT_NIU_CONTROLLER",
    "KITFOX_MCPAT_NOC",
    "KITFOX_MCPAT_PCIE_CONTROLLER",
    "KITFOX_MCPAT_PIPELINE",
    "KITFOX_MCPAT_SELECTION_LOGIC",
    "KITFOX_MCPAT_UNDIFF_CORE",
    "NUM_KITFOX_MCPAT_ENERGY_MODEL_TYPES"
};

enum KITFOX_MCPAT_ENERGY_SUBMODEL_TYPES {
    KITFOX_MCPAT_ENERGY_SUBMODEL_UNKNOWN = 0,
    KITFOX_MCPAT_ARRAYST_MEMORY,
    KITFOX_MCPAT_ARRAYST_CACHE,
    KITFOX_MCPAT_ARRAYST_RAM,
    KITFOX_MCPAT_ARRAYST_CAM,
    KITFOX_MCPAT_FUNCTIONAL_UNIT_ALU,
    KITFOX_MCPAT_FUNCTIONAL_UNIT_MUL,
    KITFOX_MCPAT_FUNCTIONAL_UNIT_FPU,
    KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND,
    KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND,
    KITFOX_MCPAT_MEMORY_CONTROLLER_PHY,
    KITFOX_MCPAT_NOC_BUS,
    KITFOX_MCPAT_NOC_ROUTER,
    NUM_KITFOX_MCPAT_ENERGY_SUBMODEL_TYPES
};

static const char *KITFOX_MCPAT_ENERGY_SUBMODEL_STR[] = {
    "KITFOX_MCPAT_ENERGY_SUBMODEL_UNKNOWN",
    "KITFOX_MCPAT_ARRAYST_MEMORY",
    "KITFOX_MCPAT_ARRAYST_CACHE",
    "KITFOX_MCPAT_ARRAYST_RAM",
    "KITFOX_MCPAT_ARRAYST_CAM",
    "KITFOX_MCPAT_FUNCTIONAL_UNIT_ALU",
    "KITFOX_MCPAT_FUNCTIONAL_UNIT_MUL",
    "KITFOX_MCPAT_FUNCTIONAL_UNIT_FPU",
    "KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND",
    "KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND",
    "KITFOX_MCPAT_MEMORY_CONTROLLER_PHY",
    "KITFOX_MCPAT_NOC_BUS",
    "KITFOX_MCPAT_NOC_ROUTER",
    "NUM_KITFOX_MCPAT_ENERGY_SUBMODEL_TYPES"
};

enum KITFOX_MCPAT_UPDATE_VAR_TYPES {
    KITFOX_MCPAT_UPDATE_VAR_UNKNOWN = 0,
    KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE,
    KITFOX_MCPAT_UPDATE_VAR_VOLTAGE,
    KITFOX_MCPAT_UPDATE_VAR_VPP,
    KITFOX_MCPAT_UPDATE_VAR_CLOCK_FREQUENCY,
    NUM_KITFOX_MCPAT_UPDATE_VAR_TYPES
};

static const char *KITFOX_MCPAT_UPDATE_VAR_STR[] = {
    "KITFOX_MCPAT_UPDATE_VAR_UNKNOWN",
    "KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE",
    "KITFOX_MCPAT_UPDATE_VAR_VOLTAGE",
    "KITFOX_MCPAT_UPDATE_VAR_VPP",
    "KITFOX_MCPAT_UPDATE_VAR_CLOCK_FREQUENCY",
    "NUM_KITFOX_MCPAT_UPDATE_VAR_TYPES"
};

class energylib_mcpat : public energy_library_t {
public:
    energylib_mcpat(pseudo_component_t *PseudoComponent);
    ~energylib_mcpat();
    
    virtual void initialize();
    virtual unit_energy_t get_unit_energy();
    virtual power_t get_tdp_power(Kelvin MaxTemperature = 373.0);
    virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter);
    virtual MeterSquare get_area();
    virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
    
private:
    int get_mcpat_data_type(int ei_data_type);
    
    int energy_model;
    int energy_submodel;
    
    double clock_frequency;
    double energy_scaling;
    double area_scaling;
    double scaling;
    
    struct {
        double switching;
        double read;
        double write;
        double search;
        double read_tag;
        double write_tag;
    } TDP_duty_cycle;
    
    // McPAT input classes
    ParseXML XML_interface;
    InputParameter input_p;
    Device_ty device_ty;
    TechnologyParameter tech_p;
    MCParam mc_p;
    NoCParam noc_p;
    NIUParam niu_p;
    PCIeParam pci_p;
    ProcParam proc_p;
    CacheDynParam cache_p;
    CoreDynParam core_p;
    
    // McPAT library models
    ArrayST *McPAT_ArrayST;
    dep_resource_conflict_check *McPAT_dep_resource_conflict_check;
    FlashController *McPAT_FlashController;
    FunctionalUnit *McPAT_FunctionalUnit;
    inst_decoder *McPAT_inst_decoder;
    interconnect *McPAT_interconnect;
    MemoryController *McPAT_MemoryController;
    MCFrontEnd *McPAT_MCFrontEnd;
    MCBackend *McPAT_MCBackend;
    MCPHY *McPAT_MCPHY;
    NIUController *McPAT_NIUController;
    NoC *McPAT_NoC;
    PCIeController *McPAT_PCIeController;
    Pipeline *McPAT_Pipeline;
    selection_logic *McPAT_selection_logic;
    UndiffCore *McPAT_UndiffCore;
};

} // namespace libKitFox_MCPAT
} // namespace libKitFox

#endif
