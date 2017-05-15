#include <iostream>
#include <cmath>
#include "kitfox.h"
#include "energylib_mcpat.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_MCPAT;

energylib_mcpat::energylib_mcpat(pseudo_component_t *PseudoComponent) :
    energy_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_MCPAT),
    McPAT_ArrayST(NULL),
    McPAT_dep_resource_conflict_check(NULL),
    McPAT_FlashController(NULL),
    McPAT_FunctionalUnit(NULL),
    McPAT_inst_decoder(NULL),
    McPAT_interconnect(NULL),
    McPAT_MemoryController(NULL),
    McPAT_MCFrontEnd(NULL),
    McPAT_MCBackend(NULL),
    McPAT_MCPHY(NULL),
    McPAT_NIUController(NULL),
    McPAT_NoC(NULL),
    McPAT_PCIeController(NULL),
    McPAT_Pipeline(NULL),
    McPAT_selection_logic(NULL),
    McPAT_UndiffCore(NULL),
    energy_model(KITFOX_MCPAT_ENERGY_MODEL_UNKNOWN),
    energy_submodel(KITFOX_MCPAT_ENERGY_SUBMODEL_UNKNOWN),
    energy_scaling(1.0),
    area_scaling(1.0),
    scaling(1.0)
{}

energylib_mcpat::~energylib_mcpat() {
  delete McPAT_ArrayST;
  delete McPAT_dep_resource_conflict_check;
  delete McPAT_FlashController;
  delete McPAT_FunctionalUnit;
  delete McPAT_inst_decoder;
  delete McPAT_interconnect;
  delete McPAT_MemoryController;
  delete McPAT_MCFrontEnd;
  delete McPAT_MCBackend;
  delete McPAT_MCPHY;
  delete McPAT_NIUController;
  delete McPAT_NoC;
  delete McPAT_PCIeController;
  delete McPAT_Pipeline;
  delete McPAT_selection_logic;
  delete McPAT_UndiffCore;
}

void energylib_mcpat::initialize() {
    try {
        if(pseudo_component->exists_in_library("TDP_switching_cycle"))
            TDP_duty_cycle.switching = pseudo_component->lookup_in_library("TDP_switching_cycle");
        else
            TDP_duty_cycle.switching = 1.0;
        if(pseudo_component->exists_in_library("TDP_read_cycle"))
            TDP_duty_cycle.read = pseudo_component->lookup_in_library("TDP_read_cycle");
        else
            TDP_duty_cycle.read = 1.0;
        if(pseudo_component->exists_in_library("TDP_write_cycle"))
            TDP_duty_cycle.write = pseudo_component->lookup_in_library("TDP_write_cycle");
        else
            TDP_duty_cycle.write = 1.0;
        if(pseudo_component->exists_in_library("TDP_search_cycle"))
            TDP_duty_cycle.search = pseudo_component->lookup_in_library("TDP_search_cycle");
        else
            TDP_duty_cycle.search = 1.0;
        if(pseudo_component->exists_in_library("TDP_read_tag_cycle"))
            TDP_duty_cycle.read_tag = pseudo_component->lookup_in_library("TDP_read_tag_cycle");
        else
            TDP_duty_cycle.read_tag = 0.0;
        if(pseudo_component->exists_in_library("TDP_write_cycle"))
            TDP_duty_cycle.write_tag = pseudo_component->lookup_in_library("TDP_write_cycle");
        else
            TDP_duty_cycle.write_tag = 0.0;
        
        // Clock frequency
        clock_frequency = pseudo_component->lookup_in_library("clock_frequency",true);
        core_p.clockRate = clock_frequency;
        
        // Optimization for Clock frequency - McPAT variable
        if(pseudo_component->exists_in_library("opt_for_clk",true))
            opt_for_clk = pseudo_component->lookup_in_library("opt_for_clk",true);
        else
            opt_for_clk = true;
        
        // Scaling factors to adjust computed results
        if(pseudo_component->exists_in_library("area_scaling",true))
            area_scaling = pseudo_component->lookup_in_library("area_scaling",true);
        else
            area_scaling = 1.0;
        
        // Energy scaling applies to both dynamic and leakage energies
        if(pseudo_component->exists_in_library("energy_scaling",true))
            energy_scaling = pseudo_component->lookup_in_library("energy_scaling",true);
        else
            energy_scaling = 1.0;
        
        // Scaling factor applies to area and leakage energy.
        // Scaling factor is used for duplicated components.
        // For instance, if there are two decoders to be modeled,
        // they can be configured as Dec0 and Dec1,
        // or simply Dec with scaling = 2 such that area and leakage power are doubled.
        // Note that dynamic energy is not scaled, otherwise 1 counter access represents
        // simultaneous accesses to both decoders which may not be the intended case.
        if(pseudo_component->exists_in_library("scaling",true))
            scaling = pseudo_component->lookup_in_library("scaling",true);
        else
            scaling = 1.0;
        
        // Energy Model and Submodel
        const char *energy_model_str = pseudo_component->lookup_in_library("energy_model");
        if(!strcasecmp(energy_model_str,"array")) {
            energy_model = KITFOX_MCPAT_ARRAYST;
            const char *energy_submodel_str = pseudo_component->lookup_in_library("energy_submodel");
            if(!strcasecmp(energy_submodel_str,"memory"))
                energy_submodel = KITFOX_MCPAT_ARRAYST_MEMORY;
            else if(!strcasecmp(energy_submodel_str,"cache"))
                energy_submodel = KITFOX_MCPAT_ARRAYST_CACHE;
            else if(!strcasecmp(energy_submodel_str,"ram"))
                energy_submodel = KITFOX_MCPAT_ARRAYST_RAM;
            else if(!strcasecmp(energy_submodel_str,"cam"))
                energy_submodel = KITFOX_MCPAT_ARRAYST_CAM;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.energy_submodel = %s is unknown",pseudo_component->name.c_str(),energy_submodel_str);
            }
        }
        else if(!strcasecmp(energy_model_str,"dep_resource_conflict_check"))
            energy_model = KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK;
        else if(!strcasecmp(energy_model_str,"flash_controller"))
            energy_model = KITFOX_MCPAT_FLASH_CONTROLLER;
        else if(!strcasecmp(energy_model_str,"functional_unit")) {
            energy_model = KITFOX_MCPAT_FUNCTIONAL_UNIT;
            const char *energy_submodel_str = pseudo_component->lookup_in_library("energy_submodel");
            if(!strcasecmp(energy_submodel_str,"alu"))
                energy_submodel = KITFOX_MCPAT_FUNCTIONAL_UNIT_ALU;
            else if(!strcasecmp(energy_submodel_str,"mul"))
                energy_submodel = KITFOX_MCPAT_FUNCTIONAL_UNIT_MUL;
            else if(!strcasecmp(energy_submodel_str,"fpu"))
                energy_submodel = KITFOX_MCPAT_FUNCTIONAL_UNIT_FPU;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.energy_submodel = %s is unknown",pseudo_component->name.c_str(),energy_submodel_str);
            }
        }
        else if(!strcasecmp(energy_model_str,"inst_decoder"))
            energy_model = KITFOX_MCPAT_INST_DECODER;
        else if(!strcasecmp(energy_model_str,"interconnect"))
            energy_model = KITFOX_MCPAT_INTERCONNECT;
        else if(!strcasecmp(energy_model_str,"memory_controller")) {
            energy_model = KITFOX_MCPAT_MEMORY_CONTROLLER;
            /*const char *energy_submodel_str = pseudo_component->lookup_in_library("energy_submodel");
            if(!strcasecmp(energy_submodel_str,"memory_controller_frontend"))
                energy_submodel = KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND;
            else if(!strcasecmp(energy_submodel_str,"memory_controller_backend"))
                energy_submodel = KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND;
            else if(!strcasecmp(energy_submodel_str,"memory_controller_phy"))
                energy_submodel = KITFOX_MCPAT_MEMORY_CONTROLLER_PHY;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.energy_submodel = %s is unknown",pseudo_component->name.c_str(),energy_submodel_str);
            }*/
        }
        else if(!strcasecmp(energy_model_str,"noc")) {
            energy_model = KITFOX_MCPAT_NOC;
            const char *energy_submodel_str = pseudo_component->lookup_in_library("energy_submodel");
            if(!strcasecmp(energy_submodel_str,"noc_bus"))
                energy_submodel = KITFOX_MCPAT_NOC_BUS;
            else if(!strcasecmp(energy_submodel_str,"noc_router"))
                energy_submodel = KITFOX_MCPAT_NOC_ROUTER;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.energy_submodel = %s is unknown",pseudo_component->name.c_str(),energy_submodel_str);
            }
        }
        else if(!strcasecmp(energy_model_str,"niu_controller"))
            energy_model = KITFOX_MCPAT_NIU_CONTROLLER;
        else if(!strcasecmp(energy_model_str,"pcie_controller"))
            energy_model = KITFOX_MCPAT_PCIE_CONTROLLER;
        else if(!strcasecmp(energy_model_str,"pipeline"))
            energy_model = KITFOX_MCPAT_PIPELINE;
        else if(!strcasecmp(energy_model_str,"selection_logic"))
            energy_model = KITFOX_MCPAT_SELECTION_LOGIC;
        else if(!strcasecmp(energy_model_str,"undiff_core"))
            energy_model = KITFOX_MCPAT_UNDIFF_CORE;
        else {
            pseudo_component->kitfox->error("mcpat","%s.library.energy_model = %s is unknown",pseudo_component->name.c_str(),energy_model_str);
        }
        
        // Feature size
        input_p.F_sz_nm = pseudo_component->lookup_in_library("feature_size",true);
        input_p.F_sz_nm *= 1e9;
        input_p.F_sz_um = input_p.F_sz_nm*0.001;
        
        input_p.temp = pseudo_component->lookup_in_library("temperature",true);
        
        // List of optimization weight factors - McPAT variables
        if(pseudo_component->exists_in_library("obj_func_dyn_energy",true))
            input_p.obj_func_dyn_energy = pseudo_component->lookup_in_library("obj_func_dyn_energy",true);
        else
            input_p.obj_func_dyn_energy = 0;
        if(pseudo_component->exists_in_library("obj_func_dyn_power",true))
            input_p.obj_func_dyn_power = pseudo_component->lookup_in_library("obj_func_dyn_power",true);
        else
            input_p.obj_func_dyn_power = 0;
        
        if(pseudo_component->exists_in_library("obj_func_leak_power",true))
            input_p.obj_func_leak_power = pseudo_component->lookup_in_library("obj_func_leak_power",true);
        else
            input_p.obj_func_leak_power = 0;
        if(pseudo_component->exists_in_library("obj_func_cycle_t",true))
            input_p.obj_func_cycle_t = pseudo_component->lookup_in_library("obj_func_cycle_t",true);
        else
            input_p.obj_func_cycle_t = 1;
        if(pseudo_component->exists_in_library("delay_wt",true))
            input_p.delay_wt = pseudo_component->lookup_in_library("delay_wt",true);
        else
            input_p.delay_wt = 100;
        if(pseudo_component->exists_in_library("area_wt",true))
            input_p.area_wt = pseudo_component->lookup_in_library("area_wt",true);
        else
            input_p.area_wt = 0;
        if(pseudo_component->exists_in_library("dynamic_power_wt",true))
            input_p.dynamic_power_wt = pseudo_component->lookup_in_library("dynamic_power_wt",true);
        else
            input_p.dynamic_power_wt = 100;
        if(pseudo_component->exists_in_library("leakage_power_wt",true))
            input_p.leakage_power_wt = pseudo_component->lookup_in_library("leakage_power_wt",true);
        else
            input_p.leakage_power_wt = 0;
        if(pseudo_component->exists_in_library("cycle_time_wt",true))
            input_p.cycle_time_wt = pseudo_component->lookup_in_library("cycle_time_wt",true);
        else
            input_p.cycle_time_wt = 0;
        if(pseudo_component->exists_in_library("delay_dev",true))
            input_p.delay_dev = pseudo_component->lookup_in_library("delay_dev",true);
        else
            input_p.delay_dev = 10000;
        if(pseudo_component->exists_in_library("area_dev",true))
            input_p.area_dev = pseudo_component->lookup_in_library("area_dev",true);
        else
            input_p.area_dev = 10000;
        if(pseudo_component->exists_in_library("dynamic_power_dev",true))
            input_p.dynamic_power_dev = pseudo_component->lookup_in_library("dynamic_power_dev",true);
        else
            input_p.dynamic_power_dev = 10000;
        if(pseudo_component->exists_in_library("leakage_power_dev",true))
            input_p.leakage_power_dev = pseudo_component->lookup_in_library("leakage_power_dev",true);
        else
            input_p.leakage_power_dev = 10000;
        if(pseudo_component->exists_in_library("cycle_time_dev",true))
            input_p.cycle_time_dev = pseudo_component->lookup_in_library("cycle_time_dev",true);
        else
            input_p.cycle_time_dev = 10000;
        if(pseudo_component->exists_in_library("ed",true))
            input_p.ed = pseudo_component->lookup_in_library("ed",true);
        else
            input_p.ed = 2;
        if(pseudo_component->exists_in_library("nuca",true))
            input_p.nuca = pseudo_component->lookup_in_library("nuca",true);
        else
            input_p.nuca = 0;
        if(pseudo_component->exists_in_library("nuca_bank_count",true))
            input_p.nuca_bank_count = pseudo_component->lookup_in_library("nuca_bank_count",true);
        else
            input_p.nuca_bank_count = 0;
        if(pseudo_component->exists_in_library("delay_wt_nuca",true))
            input_p.delay_wt_nuca = pseudo_component->lookup_in_library("delay_wt_nuca",true);
        else
            input_p.delay_wt_nuca = 0;
        if(pseudo_component->exists_in_library("area_wt_nuca",true))
            input_p.area_wt_nuca = pseudo_component->lookup_in_library("area_wt_nuca",true);
        else
            input_p.area_wt_nuca = 0;
        if(pseudo_component->exists_in_library("dynamic_power_wt_nuca",true))
            input_p.dynamic_power_wt_nuca = pseudo_component->lookup_in_library("dynamic_power_wt_nuca",true);
        else
            input_p.dynamic_power_wt_nuca = 0;
        if(pseudo_component->exists_in_library("leakage_power_wt_nuca",true))
            input_p.leakage_power_wt_nuca = pseudo_component->lookup_in_library("leakage_power_wt_nuca",true);
        else
            input_p.leakage_power_wt_nuca = 0;
        if(pseudo_component->exists_in_library("cycle_time_wt_nuca",true))
            input_p.cycle_time_wt_nuca = pseudo_component->lookup_in_library("cycle_time_wt_nuca",true);
        else
            input_p.cycle_time_wt_nuca = 0;
        if(pseudo_component->exists_in_library("delay_dev_nuca",true))
            input_p.delay_dev_nuca = pseudo_component->lookup_in_library("delay_dev_nuca",true);
        else
            input_p.delay_dev_nuca = 10000;
        if(pseudo_component->exists_in_library("area_dev_nuca",true))
            input_p.area_dev_nuca = pseudo_component->lookup_in_library("area_dev_nuca",true);
        else
            input_p.area_dev_nuca = 10000;
        if(pseudo_component->exists_in_library("dynamic_power_dev_nuca",true))
            input_p.dynamic_power_dev_nuca = pseudo_component->lookup_in_library("dynamic_power_dev_nuca",true);
        else
            input_p.dynamic_power_dev_nuca = 10;
        if(pseudo_component->exists_in_library("leakage_power_dev_nuca",true))
            input_p.leakage_power_dev_nuca = pseudo_component->lookup_in_library("leakage_power_dev_nuca",true);
        else
            input_p.leakage_power_dev_nuca = 10000;
        if(pseudo_component->exists_in_library("cycle_time_dev_nuca",true))
            input_p.cycle_time_dev_nuca = pseudo_component->lookup_in_library("cycle_time_dev_nuca",true);
        else
            input_p.cycle_time_dev_nuca = 10000;
        if(pseudo_component->exists_in_library("force_wiretype",true))
            input_p.force_wiretype = pseudo_component->lookup_in_library("force_wiretype",true);
        else
            input_p.force_wiretype = false;
        if(pseudo_component->exists_in_library("rpters_in_htree",true))
            input_p.rpters_in_htree = pseudo_component->lookup_in_library("rpters_in_htree",true);
        else
            input_p.rpters_in_htree = true;
        if(pseudo_component->exists_in_library("with_clock_grid",true))
            input_p.with_clock_grid = pseudo_component->lookup_in_library("with_clock_grid",true);
        else
            input_p.with_clock_grid = false;
        if(pseudo_component->exists_in_library("force_cache_config",true))
            input_p.force_cache_config = pseudo_component->lookup_in_library("force_cache_config",true);
        else
            input_p.force_cache_config = false;
        if(input_p.force_cache_config) {
            input_p.ndbl = pseudo_component->lookup_in_library("ndbl",true);
            input_p.ndwl = pseudo_component->lookup_in_library("ndwl",true);
            input_p.nspd = pseudo_component->lookup_in_library("nspd",true);
            input_p.ndsam1 = pseudo_component->lookup_in_library("ndsam1",true);
            input_p.ndsam2 = pseudo_component->lookup_in_library("ndsam2",true);
            input_p.ndcm = pseudo_component->lookup_in_library("ndcm",true);
        }
        
        // Don't cares
        input_p.fast_access = false; // Don't care
        input_p.is_seq_acc = false; // Don't care
        input_p.fully_assoc = false; // Don't care
        input_p.print_input_args = false; // Don't care
        input_p.burst_len = 1; // Don't care
        input_p.int_prefetch_w = 1; // dont' care
        input_p.page_sz_bits = 0; // Don't care
        input_p.ram_cell_tech_type = 0; // Don't care
        input_p.peri_global_tech_type = 0; // Don't care
        input_p.block_sz = 0; // Don't care
        input_p.tag_assoc = 1; // Don't care
        input_p.data_assoc = 1; // Don't care
        input_p.cores = 0; // Don't care
        input_p.print_detail = 5; // Don't care
        input_p.cache_level = 0; // Don't care
        
        // Default values
        // These are modified by each energy model
        input_p.line_sz = 8;
        input_p.out_w = 64;
        input_p.assoc = 1;
        input_p.nbanks = 1;
        input_p.cache_sz = 64;
        input_p.tag_w = 0;
        input_p.num_rw_ports = 1;
        input_p.num_rd_ports = 0;
        input_p.num_wr_ports = 0;
        input_p.num_se_rd_ports = 0;
        input_p.num_search_ports = 0;
        input_p.throughput = 1.0/clock_frequency;
        input_p.latency = 1.0/clock_frequency;
        input_p.is_main_mem = false;
        input_p.is_cache = false;
        input_p.pure_ram = false;
        input_p.pure_cam = false;
        input_p.add_ecc_b_ = true;
        
        // Access mode
        if(pseudo_component->exists_in_library("access_mode",true)) {
            const char *access_mode_str = pseudo_component->lookup_in_library("access_mode",true);
            if(!strcasecmp(access_mode_str,"normal"))
                input_p.access_mode = 0;
            else if(!strcasecmp(access_mode_str,"sequential"))
                input_p.access_mode = 1;
            else if(!strcasecmp(access_mode_str,"fast"))
                input_p.access_mode = 2;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.access_mode = %s is unknown",pseudo_component->name.c_str(),access_mode_str);
            }
        }
        else
            input_p.access_mode = 0;
        
        // Wire type
        if(pseudo_component->exists_in_library("wire_type",true)) {
            const char *wire_type_str = pseudo_component->lookup_in_library("wire_type",true);
            if(!strcasecmp(wire_type_str,"global"))
                input_p.wt = (Wire_type)Global;
            else if(!strcasecmp(wire_type_str,"global_5"))
                input_p.wt = (Wire_type)Global_5;
            else if(!strcasecmp(wire_type_str,"global_10"))
                input_p.wt = (Wire_type)Global_10;
            else if(!strcasecmp(wire_type_str,"global_20"))
                input_p.wt = (Wire_type)Global_20;
            else if(!strcasecmp(wire_type_str,"global_30"))
                input_p.wt = (Wire_type)Global_30;
            else if(!strcasecmp(wire_type_str,"low_swing"))
                input_p.wt = (Wire_type)Low_swing;
            else if(!strcasecmp(wire_type_str,"semi_global"))
                input_p.wt = (Wire_type)Semi_global;
            else if(!strcasecmp(wire_type_str,"transmission"))
                input_p.wt = (Wire_type)Transmission;
            else if(!strcasecmp(wire_type_str,"optical"))
                input_p.wt = (Wire_type)Optical;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.wire_type = %s is unknown",pseudo_component->name.c_str(),wire_type_str);
            }
        }
        else
            input_p.wt = (Wire_type)Global;
        
        // Device type that applies to all different types
        if(pseudo_component->exists_in_library("device_type",true)) {
            const char *device_type_str = pseudo_component->lookup_in_library("device_type",true);
            if(!strcasecmp(device_type_str,"hp")) {
                input_p.data_arr_ram_cell_tech_type = 0;
                input_p.data_arr_peri_global_tech_type = 0;
                input_p.tag_arr_ram_cell_tech_type = 0;
                input_p.tag_arr_peri_global_tech_type = 0;
            }
            else if(!strcasecmp(device_type_str,"lstp")) {
                input_p.data_arr_ram_cell_tech_type = 1;
                input_p.data_arr_peri_global_tech_type = 1;
                input_p.tag_arr_ram_cell_tech_type = 1;
                input_p.tag_arr_peri_global_tech_type = 1;
            }
            else if(!strcasecmp(device_type_str,"lop")) {
                input_p.data_arr_ram_cell_tech_type = 2;
                input_p.data_arr_peri_global_tech_type = 2;
                input_p.tag_arr_ram_cell_tech_type = 2;
                input_p.tag_arr_peri_global_tech_type = 2;
            }
            else if(!strcasecmp(device_type_str,"lp_dram")) {
                input_p.data_arr_ram_cell_tech_type = 3;
                input_p.data_arr_peri_global_tech_type = 3;
                input_p.tag_arr_ram_cell_tech_type = 3;
                input_p.tag_arr_peri_global_tech_type = 3;
            }
            else if(!strcasecmp(device_type_str,"comm_dram")) {
                input_p.data_arr_ram_cell_tech_type = 4;
                input_p.data_arr_peri_global_tech_type = 4;
                input_p.tag_arr_ram_cell_tech_type = 4;
                input_p.tag_arr_peri_global_tech_type = 4;
            }
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.device_type = %s is unknown",pseudo_component->name.c_str(),device_type_str);
            }
        }
        else {
            input_p.data_arr_ram_cell_tech_type = 0;
            input_p.data_arr_peri_global_tech_type = 0;
            input_p.tag_arr_ram_cell_tech_type = 0;
            input_p.tag_arr_peri_global_tech_type = 0;
        }
        
        // Individual specification of the device type
        if(pseudo_component->exists_in_library("data_arr_ram_cell_tech_type",true)) {
            const char *device_type_str = pseudo_component->lookup_in_library("data_arr_ram_cell_tech_type",true);
            if(!strcasecmp(device_type_str,"hp"))
                input_p.data_arr_ram_cell_tech_type = 0;
            else if(!strcasecmp(device_type_str,"lstp"))
                input_p.data_arr_ram_cell_tech_type = 1;
            else if(!strcasecmp(device_type_str,"lop"))
                input_p.data_arr_ram_cell_tech_type = 2;
            else if(!strcasecmp(device_type_str,"lp_dram"))
                input_p.data_arr_ram_cell_tech_type = 3;
            else if(!strcasecmp(device_type_str,"comm_dram"))
                input_p.data_arr_ram_cell_tech_type = 4;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.data_arr_ram_cell_tech_type = %s is unknown",pseudo_component->name.c_str(),device_type_str);
            }
        }
        if(pseudo_component->exists_in_library("data_arr_peri_global_tech_type",true)) {
            const char *device_type_str = pseudo_component->lookup_in_library("data_arr_peri_global_tech_type",true);
            if(!strcasecmp(device_type_str,"hp"))
                input_p.data_arr_peri_global_tech_type = 0;
            else if(!strcasecmp(device_type_str,"lstp"))
                input_p.data_arr_peri_global_tech_type = 1;
            else if(!strcasecmp(device_type_str,"lop"))
                input_p.data_arr_peri_global_tech_type = 2;
            else if(!strcasecmp(device_type_str,"lp_dram"))
                input_p.data_arr_peri_global_tech_type = 3;
            else if(!strcasecmp(device_type_str,"comm_dram"))
                input_p.data_arr_peri_global_tech_type = 4;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.data_arr_peri_global_tech_type = %s is unknown",pseudo_component->name.c_str(),device_type_str);
            }
        }
        if(pseudo_component->exists_in_library("tag_arr_ram_cell_tech_type",true)) {
            const char *device_type_str = pseudo_component->lookup_in_library("tag_arr_ram_cell_tech_type",true);
            if(!strcasecmp(device_type_str,"hp"))
                input_p.tag_arr_ram_cell_tech_type = 0;
            else if(!strcasecmp(device_type_str,"lstp"))
                input_p.tag_arr_ram_cell_tech_type = 1;
            else if(!strcasecmp(device_type_str,"lop"))
                input_p.tag_arr_ram_cell_tech_type = 2;
            else if(!strcasecmp(device_type_str,"lp_dram"))
                input_p.tag_arr_ram_cell_tech_type = 3;
            else if(!strcasecmp(device_type_str,"comm_dram"))
                input_p.tag_arr_ram_cell_tech_type = 4;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.tag_arr_ram_cell_tech_type = %s is unknown",pseudo_component->name.c_str(),device_type_str);
            }
        }
        if(pseudo_component->exists_in_library("tag_arr_peri_global_tech_type",true)) {
            const char *device_type_str = pseudo_component->lookup_in_library("tag_arr_peri_global_tech_type",true);
            if(!strcasecmp(device_type_str,"hp"))
                input_p.tag_arr_peri_global_tech_type = 0;
            else if(!strcasecmp(device_type_str,"lstp"))
                input_p.tag_arr_peri_global_tech_type = 1;
            else if(!strcasecmp(device_type_str,"lop"))
                input_p.tag_arr_peri_global_tech_type = 2;
            else if(!strcasecmp(device_type_str,"lp_dram"))
                input_p.tag_arr_peri_global_tech_type = 3;
            else if(!strcasecmp(device_type_str,"comm_dram"))
                input_p.tag_arr_peri_global_tech_type = 4;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.tag_arr_peri_global_tech_type = %s is unknown",pseudo_component->name.c_str(),device_type_str);
            }
        }
        
        // Interconnect projection
        if(pseudo_component->exists_in_library("interconnect_projection",true)) {
            const char *ic_proj_type_str = pseudo_component->lookup_in_library("interconnect_projection",true);
            if(!strcasecmp(ic_proj_type_str,"aggressive"))
                input_p.ic_proj_type = 0;
            else if(!strcasecmp(ic_proj_type_str,"conservative"))
                input_p.ic_proj_type = 1;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.interconnect_projection = %s is unknown",pseudo_component->name.c_str(),ic_proj_type_str);
            }
        }
        else
            input_p.ic_proj_type = 0;
        
        // Wiring type that applies to all different types
        if(pseudo_component->exists_in_library("wiring_type",true)) {
            const char *wiring_type_str = pseudo_component->lookup_in_library("wiring_type",true);
            if(!strcasecmp(wiring_type_str,"local")) {
                input_p.wire_is_mat_type = 0;
                input_p.wire_os_mat_type = 0;
            }
            else if(!strcasecmp(wiring_type_str,"semi_global")) {
                input_p.wire_is_mat_type = 1;
                input_p.wire_os_mat_type = 1;
            }
            else if(!strcasecmp(wiring_type_str,"global")) {
                input_p.wire_is_mat_type = 2;
                input_p.wire_os_mat_type = 2;
            }
            else if(!strcasecmp(wiring_type_str,"dram")) {
                input_p.wire_is_mat_type = 3;
                input_p.wire_os_mat_type = 3;
            }
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.wiring_type = %s is unknown",pseudo_component->name.c_str(),wiring_type_str);
            }
        }
        else {
            input_p.wire_is_mat_type = 2;
            input_p.wire_os_mat_type = 2;
        }
        
        // Individual specification of the wiring type
        if(pseudo_component->exists_in_library("wire_is_mat_type",true)) {
            const char *wiring_type_str = pseudo_component->lookup_in_library("wire_is_mat_type",true);
            if(!strcasecmp(wiring_type_str,"local"))
                input_p.wire_is_mat_type = 0;
            else if(!strcasecmp(wiring_type_str,"semi_global"))
                input_p.wire_is_mat_type = 1;
            else if(!strcasecmp(wiring_type_str,"global"))
                input_p.wire_is_mat_type = 2;
            else if(!strcasecmp(wiring_type_str,"dram"))
                input_p.wire_is_mat_type = 3;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.wire_is_mat_type = %s is unknown",pseudo_component->name.c_str(),wiring_type_str);
            }
        }
        if(pseudo_component->exists_in_library("wire_os_mat_type",true)) {
            const char *wiring_type_str = pseudo_component->lookup_in_library("wire_os_mat_type",true);
            if(!strcasecmp(wiring_type_str,"local"))
                input_p.wire_os_mat_type = 0;
            else if(!strcasecmp(wiring_type_str,"semi_global"))
                input_p.wire_os_mat_type = 1;
            else if(!strcasecmp(wiring_type_str,"global"))
                input_p.wire_os_mat_type = 2;
            else if(!strcasecmp(wiring_type_str,"dram"))
                input_p.wire_os_mat_type = 3;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.wire_os_mat_type = %s is unknown",pseudo_component->name.c_str(),wiring_type_str);
            }
        }
        
        // General wiring type
        if(pseudo_component->exists_in_library("ver_htree_wires_over_array",true)) {
            const char *ver_htree_wires_over_array_str = pseudo_component->lookup_in_library("ver_htree_wires_over_array",true);
            if(!strcasecmp(ver_htree_wires_over_array_str,"local"))
                input_p.ver_htree_wires_over_array = 0;
            else if(!strcasecmp(ver_htree_wires_over_array_str,"semi_global"))
                input_p.ver_htree_wires_over_array = 1;
            else if(!strcasecmp(ver_htree_wires_over_array_str,"global"))
                input_p.ver_htree_wires_over_array = 2;
            else if(!strcasecmp(ver_htree_wires_over_array_str,"dram"))
                input_p.ver_htree_wires_over_array = 3;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.ver_htree_wires_over_array = %s is unknown",pseudo_component->name.c_str(),ver_htree_wires_over_array_str);
            }
        }
        else
            input_p.ver_htree_wires_over_array = 0;
        if(pseudo_component->exists_in_library("broadcast_addr_din_over_ver_htrees",true)) {
            const char *broadcast_addr_din_over_ver_htrees_str = pseudo_component->lookup_in_library("broadcast_addr_din_over_ver_htrees",true);
            if(!strcasecmp(broadcast_addr_din_over_ver_htrees_str,"local"))
                input_p.broadcast_addr_din_over_ver_htrees = 0;
            else if(!strcasecmp(broadcast_addr_din_over_ver_htrees_str,"semi_global"))
                input_p.broadcast_addr_din_over_ver_htrees = 1;
            else if(!strcasecmp(broadcast_addr_din_over_ver_htrees_str,"global"))
                input_p.broadcast_addr_din_over_ver_htrees = 2;
            else if(!strcasecmp(broadcast_addr_din_over_ver_htrees_str,"dram"))
                input_p.broadcast_addr_din_over_ver_htrees = 3;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.broadcast_addr_din_over_ver_htrees = %s is unknown",pseudo_component->name.c_str(),broadcast_addr_din_over_ver_htrees_str);
            }
        }
        else
            input_p.broadcast_addr_din_over_ver_htrees = 0;
        
        const char *component_type_str = pseudo_component->lookup_in_library("component_type",true);
        if(!strcasecmp(component_type_str,"core"))
            device_ty = Core_device;
        else if(!strcasecmp(component_type_str,"llc"))
            device_ty = LLC_device;
        else if(!strcasecmp(component_type_str,"uncore"))
            device_ty = Uncore_device;
        else {
            pseudo_component->kitfox->error("mcpat","%s.library.component_type = %s is unknown",pseudo_component->name.c_str(),component_type_str);
        }
        
        if(device_ty == Core_device) {
            if(pseudo_component->exists_in_library("opt_local",true))
                core_p.opt_local = (bool)pseudo_component->lookup_in_library("opt_local",true);
            else
                core_p.opt_local = false;
            
            const char *core_type_str = pseudo_component->lookup_in_library("core_type",true);
            if(!strcasecmp(core_type_str,"ooo"))
                core_p.core_ty = OOO;
            else if(!strcasecmp(core_type_str,"inorder"))
                core_p.core_ty = Inorder;
            else {
                pseudo_component->kitfox->error("mcpat","%s.library.core_type = %s is unknown",pseudo_component->name.c_str(),core_type_str);
            }
        }
        
        if(pseudo_component->exists_in_library("embedded",true))
            XML_interface.sys.Embedded = (bool)pseudo_component->lookup_in_library("embedded",true);
        else
            XML_interface.sys.Embedded = false;
        
        if(pseudo_component->exists_in_library("longer_channel_device",true))
            XML_interface.sys.longer_channel_device = (bool)pseudo_component->lookup_in_library("longer_channel_device",true);
        else
            XML_interface.sys.longer_channel_device = false;
        
        switch(energy_model) {
            case KITFOX_MCPAT_ARRAYST: {
                input_p.line_sz = pseudo_component->lookup_in_library("line_size");
                if(pseudo_component->exists_in_library("out_width"))
                    input_p.out_w = pseudo_component->lookup_in_library("out_width");
                else
                    input_p.out_w = input_p.line_sz*8;
                if(pseudo_component->exists_in_library("assoc"))
                    input_p.assoc = pseudo_component->lookup_in_library("assoc");
                else
                    input_p.assoc = 1;
                if(pseudo_component->exists_in_library("nbanks"))
                    input_p.nbanks = pseudo_component->lookup_in_library("nbanks");
                else
                    input_p.nbanks = 1;
                if(pseudo_component->exists_in_library("nsets"))
                    input_p.nsets = pseudo_component->lookup_in_library("nsets");
                else
                    input_p.nsets = 0;
                if(pseudo_component->exists_in_library("size"))
                    input_p.cache_sz = pseudo_component->lookup_in_library("size");
                else
                    input_p.cache_sz = 0;
                if(pseudo_component->exists_in_library("tag_width"))
                    input_p.tag_w = pseudo_component->lookup_in_library("tag_width");
                else
                    input_p.tag_w = 0;
                if(pseudo_component->exists_in_library("num_rw_ports"))
                    input_p.num_rw_ports = pseudo_component->lookup_in_library("num_rw_ports");
                else
                    input_p.num_rw_ports = 0;
                if(pseudo_component->exists_in_library("num_rd_ports"))
                    input_p.num_rd_ports = pseudo_component->lookup_in_library("num_rd_ports");
                else
                    input_p.num_rd_ports = 0;
                if(pseudo_component->exists_in_library("num_wr_ports"))
                    input_p.num_wr_ports = pseudo_component->lookup_in_library("num_wr_ports");
                else
                    input_p.num_wr_ports = 0;
                if(pseudo_component->exists_in_library("num_se_rd_ports"))
                    input_p.num_se_rd_ports = pseudo_component->lookup_in_library("num_se_rd_ports");
                else
                    input_p.num_se_rd_ports = 0;
                if(pseudo_component->exists_in_library("num_search_ports"))
                    input_p.num_search_ports = pseudo_component->lookup_in_library("num_search_ports");
                else
                    input_p.num_search_ports = 0;
                if(pseudo_component->exists_in_library("cycle_time"))
                    input_p.throughput = pseudo_component->lookup_in_library("cycle_time");
                else
                    input_p.throughput = 1.0;
                if(pseudo_component->exists_in_library("access_time"))
                    input_p.latency = pseudo_component->lookup_in_library("access_time");
                else
                    input_p.latency = 1.0;
                if(pseudo_component->exists_in_library("add_ecc_b"))
                    input_p.add_ecc_b_ = pseudo_component->lookup_in_library("add_ecc_b");
                else
                    input_p.add_ecc_b_ = true;
                
                switch(energy_submodel) {
                    case KITFOX_MCPAT_ARRAYST_MEMORY: {
                        input_p.is_main_mem = true;
                        break;
                    }
                    case KITFOX_MCPAT_ARRAYST_CACHE: {
                        input_p.is_cache = true;
                        break;
                    }
                    case KITFOX_MCPAT_ARRAYST_RAM: {
                        input_p.pure_ram = true;
                        break;
                    }
                    case KITFOX_MCPAT_ARRAYST_CAM: {
                        input_p.pure_cam = true;
                        break;
                    }
                    default: break;
                }
                
                if(!(input_p.nsets||input_p.cache_sz)) {
                    pseudo_component->kitfox->error("mcpat","%s.library has both nsets and size undefined",pseudo_component->name.c_str());
                }
                
                if(input_p.cache_sz == 0)
                    input_p.cache_sz = input_p.line_sz*input_p.nsets*(input_p.assoc>0?input_p.assoc:1);
                else
                    input_p.nsets = input_p.cache_sz/(input_p.line_sz*(input_p.assoc>0?input_p.assoc:1));
                input_p.specific_tag = (bool)(input_p.tag_w > 0);
                input_p.throughput /= clock_frequency;
                input_p.latency /= clock_frequency;
                
                McPAT_ArrayST = new ArrayST(&input_p,(const char*)"array",device_ty,core_p.opt_local,
                                            core_p.core_ty);
                if(!McPAT_ArrayST) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create ArrayST",pseudo_component->name.c_str());
                }
                
                if(pseudo_component->exists_in_library("adjust_area")) {
                    bool adjust_area = pseudo_component->lookup_in_library("adjust_area");
                    if(adjust_area)
                        McPAT_ArrayST->local_result.adjust_area();
                }
                break;
            }
            case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
                int compare_bits = pseudo_component->lookup_in_library("compare_bits");
                core_p.decodeW = pseudo_component->lookup_in_library("decode_width");
                
                McPAT_dep_resource_conflict_check
                = new dep_resource_conflict_check(&input_p,core_p,compare_bits,true);
                
                if(!McPAT_dep_resource_conflict_check) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create dep_resource_conflict_check",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_FLASH_CONTROLLER: {
                // Convert clock frequency in MHz
                XML_interface.sys.flashc.mc_clock = clock_frequency * 1e-6;
                // Number of controller is set by scaling
                XML_interface.sys.flashc.number_mcs = 1;
                
                const char *controller_type_str = pseudo_component->lookup_in_library("controller_type");
                if(!strcasecmp(controller_type_str,"high_performance"))
                    XML_interface.sys.flashc.type = 0;
                else if(!strcasecmp(controller_type_str,"low_power"))
                    XML_interface.sys.flashc.type = 1;
                else {
                    pseudo_component->kitfox->error("mcpat","%s.library.controller_type = %s is unknown",pseudo_component->name.c_str(),controller_type_str);
                }
                
                if(pseudo_component->exists_in_library("total_load_percentage"))
                    XML_interface.sys.flashc.total_load_perc = pseudo_component->lookup_in_library("total_load_percentage");
                else
                    XML_interface.sys.flashc.total_load_perc = 1.0;
                
                XML_interface.sys.flashc.peak_transfer_rate = pseudo_component->lookup_in_library("peak_transfer_rate");
                XML_interface.sys.flashc.withPHY = pseudo_component->lookup_in_library("withPHY");
                
                McPAT_FlashController = new FlashController(&XML_interface,&input_p);
                
                if(!McPAT_FlashController) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create FlashController",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
                // Number of functional units is set by scaling
                core_p.num_alus = scaling;
                core_p.num_fpus = scaling;
                core_p.num_muls = scaling;
                
                switch(energy_submodel) {
                    case KITFOX_MCPAT_FUNCTIONAL_UNIT_ALU: {
                        McPAT_FunctionalUnit = new FunctionalUnit(&XML_interface,0,&input_p,core_p,ALU);
                        break;
                    }
                    case KITFOX_MCPAT_FUNCTIONAL_UNIT_MUL: {
                        McPAT_FunctionalUnit = new FunctionalUnit(&XML_interface,0,&input_p,core_p,MUL);
                        break;
                    }
                    case KITFOX_MCPAT_FUNCTIONAL_UNIT_FPU: {
                        McPAT_FunctionalUnit = new FunctionalUnit(&XML_interface,0,&input_p,core_p,FPU);
                        break;
                    }
                    default: break;
                }
                
                if(!McPAT_FunctionalUnit) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create FunctionalUnit",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_INST_DECODER: {
                if(pseudo_component->exists_in_library("x86"))
                    core_p.x86 = (bool)pseudo_component->lookup_in_library("x86");
                else
                    core_p.x86 = false;
                
                // McPAT bug: McPAT doesn't scale the leakage of the instruction decoder
                // even if the decodeW changes.
                if(core_p.x86) {
                    area_scaling = scaling;
                    scaling = 1.0;
                }
                
                int decode_length = pseudo_component->lookup_in_library("decode_length");
                
                McPAT_inst_decoder = new inst_decoder(true,&input_p,decode_length,1,core_p.x86,
                                                      device_ty,core_p.core_ty);
                
                if(!McPAT_inst_decoder) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create inst_decoder",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_INTERCONNECT: {
                double wire_length = 0.0;
                if(pseudo_component->exists_in_library("wire_length"))
                    wire_length = pseudo_component->lookup_in_library("wire_length");
                else {
                    Setting &setting = pseudo_component->lookup_in_library("connect");
                    for(unsigned c = 0; c < setting.getLength(); c++) {
                        string source_name = (const char*)setting[c];
                        Comp_ID source_id = pseudo_component->kitfox->get_component_id(source_name);
                        if(source_id == INVALID_COMP_ID) {
                            pseudo_component->kitfox->error("mcpat","%s.library can't find a pseudo component %s as a connecting source",pseudo_component->name.c_str(),source_name.c_str());
                        }
                        
                        dimension_t source_dimension;
                        if(pseudo_component->kitfox->pull_data(source_id,0.0,UNSPECIFIED_TIME,KITFOX_DATA_DIMENSION,&source_dimension) != KITFOX_QUEUE_ERROR_NONE) {
                            pseudo_component->kitfox->error("mcpat","%s.library can't find dimension data of connecting source component %s",pseudo_component->name.c_str(),source_name.c_str());
                        }
                        wire_length += sqrt(source_dimension.get_area());
                    }
                }
                
                if((wire_length <= 0.0)||(wire_length >= 1.0)) {
                    pseudo_component->kitfox->error("mcpat","%s.library has unrealistic wire_length = %lf [Meters]",pseudo_component->name.c_str(),wire_length);
                }
                // Convert wire_length in um
                wire_length *= 1e6;
                
                if(XML_interface.sys.Embedded) {
                    input_p.wire_is_mat_type = 0;
                    input_p.wire_os_mat_type = 0;
                    input_p.wt = Global_30;
                }
                else {
                    input_p.wire_is_mat_type = 2;
                    input_p.wire_os_mat_type = 2;
                    input_p.wt = Global;
                }
                input_p.throughput = 1.0/clock_frequency;
                input_p.latency = 1.0/clock_frequency;
                
                if(pseudo_component->exists_in_library("pipelinable"))
                    input_p.pipelinable = pseudo_component->lookup_in_library("pipelinable");
                else
                    input_p.pipelinable = false;
                
                double routing_over_percentage;
                if(input_p.pipelinable)
                    routing_over_percentage = pseudo_component->lookup_in_library("routing_over_percentage");
                
                bool opt_local;
                if(device_ty == Core_device)
                    opt_local = core_p.opt_local;
                else if(pseudo_component->exists_in_library("opt_local"))
                    opt_local = pseudo_component->lookup_in_library("opt_local");
                else
                    opt_local = false;
                
                int data_width = pseudo_component->lookup_in_library("data_width");
                
                // Overwrite wire_type
                if(pseudo_component->exists_in_library("wire_type")) {
                    const char *wire_type_str = pseudo_component->lookup_in_library("wire_type");
                    if(!strcasecmp(wire_type_str,"global"))
                        input_p.wt = (Wire_type)Global;
                    else if(!strcasecmp(wire_type_str,"global_5"))
                        input_p.wt = (Wire_type)Global_5;
                    else if(!strcasecmp(wire_type_str,"global_10"))
                        input_p.wt = (Wire_type)Global_10;
                    else if(!strcasecmp(wire_type_str,"global_20"))
                        input_p.wt = (Wire_type)Global_20;
                    else if(!strcasecmp(wire_type_str,"global_30"))
                        input_p.wt = (Wire_type)Global_30;
                    else if(!strcasecmp(wire_type_str,"low_swing"))
                        input_p.wt = (Wire_type)Low_swing;
                    else if(!strcasecmp(wire_type_str,"semi_global"))
                        input_p.wt = (Wire_type)Semi_global;
                    else if(!strcasecmp(wire_type_str,"transmission"))
                        input_p.wt = (Wire_type)Transmission;
                    else if(!strcasecmp(wire_type_str,"optical"))
                        input_p.wt = (Wire_type)Optical;
                    else {
                        pseudo_component->kitfox->error("mcpat","%s.library.wire_type = %s is unknown",pseudo_component->name.c_str(),wire_type_str);
                    }
                }
                
                McPAT_interconnect = new interconnect((const char*)"interconnect",device_ty,1,1,data_width,
                                                      wire_length,&input_p,3,input_p.pipelinable,
                                                      routing_over_percentage,opt_local,
                                                      core_p.core_ty,input_p.wt);
                
                if(!McPAT_interconnect) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create interconnect",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_MEMORY_CONTROLLER: {
                MemoryCtrl_type memoryctrl_type = MC;
                
                // Correct some variables
                input_p.wire_is_mat_type = 2;
                input_p.wire_os_mat_type = 2;
                input_p.wt = (Wire_type)Global;
                
                XML_interface.sys.mc.mc_clock = clock_frequency*1e-6; // in MHz
                //clock_frequency *= 2; // DDR
                
                // Number of MCs is set by scaling option
                XML_interface.sys.mc.number_mcs = 1;
                
                XML_interface.sys.mc.llc_line_length = pseudo_component->lookup_in_library("line_size");
                XML_interface.sys.mc.databus_width = pseudo_component->lookup_in_library("databus_width");
                XML_interface.sys.mc.addressbus_width = pseudo_component->lookup_in_library("addressbus_width");
                XML_interface.sys.mc.memory_channels_per_mc = pseudo_component->lookup_in_library("num_channels");
                XML_interface.sys.mc.peak_transfer_rate = pseudo_component->lookup_in_library("transfer_rate");
                XML_interface.sys.mc.number_ranks = pseudo_component->lookup_in_library("num_ranks");
                
                if(pseudo_component->exists_in_library("lvds"))
                    XML_interface.sys.mc.LVDS = (bool)pseudo_component->lookup_in_library("lvds");
                else
                    XML_interface.sys.mc.LVDS = true;
                if(pseudo_component->exists_in_library("withPHY"))
                    XML_interface.sys.mc.withPHY = (bool)pseudo_component->lookup_in_library("withPHY");
                else
                    XML_interface.sys.mc.withPHY = false;
                
                if(pseudo_component->exists_in_library("type")) {
                    const char *type_str = pseudo_component->lookup_in_library("type");
                    if(!strcasecmp(type_str,"high_performance"))
                        XML_interface.sys.mc.type = 0;
                    else if(!strcasecmp(type_str,"low_power"))
                        XML_interface.sys.mc.type = 1;
                    else {
                        pseudo_component->kitfox->error("mcpat","%s.library.type = %s is unknown",pseudo_component->name.c_str(),type_str);
                    }
                }
                else {
                    XML_interface.sys.mc.type = 0; // high_performance by default
                }
                
                // The following lines are part of MemoryController::set_mc_param()
                mc_p.clockRate = XML_interface.sys.mc.mc_clock; //*2; // DDR
                mc_p.clockRate *= 1e6;
                mc_p.llcBlockSize = int(ceil(XML_interface.sys.mc.llc_line_length/8.0))
                +XML_interface.sys.mc.llc_line_length;
                mc_p.dataBusWidth = int(ceil(XML_interface.sys.mc.databus_width/8.0))
                +XML_interface.sys.mc.databus_width;
                mc_p.addressBusWidth = int(ceil(XML_interface.sys.mc.addressbus_width));
                mc_p.opcodeW = 16; // fixed
                mc_p.num_mcs = XML_interface.sys.mc.number_mcs;
                mc_p.num_channels = XML_interface.sys.mc.memory_channels_per_mc;
                mc_p.peakDataTransferRate = XML_interface.sys.mc.peak_transfer_rate;
                mc_p.memRank = XML_interface.sys.mc.number_ranks;
                mc_p.frontend_duty_cycle = 0.5; // fixed
                mc_p.LVDS = XML_interface.sys.mc.LVDS;
                mc_p.type = XML_interface.sys.mc.type;
                mc_p.withPHY = XML_interface.sys.mc.withPHY;

                //McPAT_MemoryController = new MemoryController(&XML_interface,&input_p,memoryctrl_type);
                XML_interface.sys.mc.req_window_size_per_channel = pseudo_component->lookup_in_library("request_window_size");
                XML_interface.sys.mc.IO_buffer_size_per_channel = pseudo_component->lookup_in_library("IO_buffer_size");

                if(pseudo_component->exists_in_library("physical_address_width"))
                    XML_interface.sys.physical_address_width = pseudo_component->lookup_in_library("physical_address_width");
                else
                    XML_interface.sys.physical_address_width = XML_interface.sys.mc.addressbus_width;                
                McPAT_MCFrontEnd = new MCFrontEnd(&XML_interface,&input_p,mc_p,memoryctrl_type);
                if(!McPAT_MCFrontEnd) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create MCFrontEnd",pseudo_component->name.c_str());
                }
                
                McPAT_MCBackend = new MCBackend(&input_p,mc_p,memoryctrl_type);
                if(!McPAT_MCBackend) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create MCBackend",pseudo_component->name.c_str());
                }

                McPAT_MCPHY = new MCPHY(&input_p,mc_p,memoryctrl_type);
                if(!McPAT_MCPHY) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create MCPHY",pseudo_component->name.c_str());
                }

                /*switch(energy_submodel) {
                    case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                        XML_interface.sys.mc.req_window_size_per_channel = pseudo_component->lookup_in_library("request_window_size");
                        XML_interface.sys.mc.IO_buffer_size_per_channel = pseudo_component->lookup_in_library("IO_buffer_size");
                        
                        if(pseudo_component->exists_in_library("physical_address_width"))
                            XML_interface.sys.physical_address_width = pseudo_component->lookup_in_library("physical_address_width");
                        else
                            XML_interface.sys.physical_address_width = XML_interface.sys.mc.addressbus_width;
                        
                        McPAT_MCFrontEnd = new MCFrontEnd(&XML_interface,&input_p,mc_p,memoryctrl_type);
                        
                        if(!McPAT_MCFrontEnd) {
                            pseudo_component->kitfox->error("mcpat","%s.library failed to create MCFrontEnd",pseudo_component->name.c_str());
                        }
                        break;
                    }
                    case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                        McPAT_MCBackend = new MCBackend(&input_p,mc_p,memoryctrl_type);
                        
                        if(!McPAT_MCBackend) {
                            pseudo_component->kitfox->error("mcpat","%s.library failed to create MCBackend",pseudo_component->name.c_str());
                        }
                        break;
                    }
                    case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                        McPAT_MCPHY = new MCPHY(&input_p,mc_p,memoryctrl_type);
                        
                        if(!McPAT_MCPHY) {
                            pseudo_component->kitfox->error("mcpat","%s.library failed to create MCPHY",pseudo_component->name.c_str());
                        }
                        break;
                    }
                    default: break;
                }*/
                /*
                 // MC param debugging
                 cout << "mcp.clockRate = " << mcp.clockRate << endl;
                 cout << "mcp.llcBlockSize = " << mcp.llcBlockSize << endl;
                 cout << "mcp.dataBusWidth = " << mcp.dataBusWidth << endl;
                 cout << "mcp.addressBusWidth = " << mcp.addressBusWidth << endl;
                 cout << "mcp.opcodeW = " << mcp.opcodeW << endl;
                 cout << "mcp.num_mcs = " << mcp.num_mcs << endl;
                 cout << "mcp.num_channels = " << mcp.num_channels;
                 cout << "mcp.peakDataTransferRate = " << mcp.peakDataTransferRate << endl;
                 cout << "mcp.memRank = " << mcp.memRank << endl;
                 cout << "mcp.frontend_duty_cycle = " << mcp.frontend_duty_cycle << endl;
                 cout << "mcp.LVDS = " << mcp.LVDS << endl;
                 cout << "mcp.type = " << mcp.type << endl;
                 cout << "mcp.withPHY = " << mcp.withPHY << endl;
                 */
                break;
            }
            case KITFOX_MCPAT_NIU_CONTROLLER: {
                XML_interface.sys.niu.clockrate = clock_frequency * 1e-6; // in MHz
                
                // Number of NIUs is set by scaling option
                XML_interface.sys.niu.number_units = 1;
                
                const char *type_str = pseudo_component->lookup_in_library("type");
                
                if(!strcasecmp(type_str,"high_performance"))
                    XML_interface.sys.niu.type = 0;
                else if(!strcasecmp(type_str,"low_power"))
                    XML_interface.sys.niu.type = 1;
                else {
                    pseudo_component->kitfox->error("mcpat","%s.library.type = %s is unknonw",pseudo_component->name.c_str(),type_str);
                }
                
                if(pseudo_component->exists_in_library("load_percentage"))
                    XML_interface.sys.niu.total_load_perc = pseudo_component->lookup_in_library("load_percentage");
                else
                    XML_interface.sys.niu.total_load_perc = 1.0;
                
                McPAT_NIUController = new NIUController(&XML_interface,&input_p);
                
                if(!McPAT_NIUController) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create NIUController",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_NOC: {
                // Correct some variables
                if(XML_interface.sys.Embedded) {
                    input_p.wt = (Wire_type)Global_30;
                    input_p.wire_is_mat_type = 0;
                    input_p.wire_os_mat_type = 1;
                }
                else {
                    input_p.wt = (Wire_type)Global;
                    input_p.wire_is_mat_type = 2;
                    input_p.wire_os_mat_type = 2;
                }
                
                XML_interface.sys.NoC[0].clockrate = clock_frequency*1e-6;
                
                double link_length = 0.0;
                switch(energy_submodel) {
                    case KITFOX_MCPAT_NOC_BUS: {
                        XML_interface.sys.NoC[0].type = 0;
                        
                        if(pseudo_component->exists_in_library("link_throughput"))
                            XML_interface.sys.NoC[0].link_throughput = pseudo_component->lookup_in_library("link_throughput");
                        else
                            XML_interface.sys.NoC[0].link_throughput = 1.0;
                        if(pseudo_component->exists_in_library("link_latency"))
                            XML_interface.sys.NoC[0].link_latency = pseudo_component->lookup_in_library("link_latency");
                        else
                            XML_interface.sys.NoC[0].link_latency = 1.0;
                        if(pseudo_component->exists_in_library("chip_coverage"))
                            XML_interface.sys.NoC[0].chip_coverage = pseudo_component->lookup_in_library("chip_coverage");
                        else
                            XML_interface.sys.NoC[0].chip_coverage = 1.0;
                        if(pseudo_component->exists_in_library("route_over_percentage"))
                            XML_interface.sys.NoC[0].route_over_perc = pseudo_component->lookup_in_library("route_over_percentage");
                        else
                            XML_interface.sys.NoC[0].route_over_perc = 0.5;
                        
                        if(pseudo_component->exists_in_library("link_length"))
                            link_length = (double)pseudo_component->lookup_in_library("link_length")*1e6; // Link length in um
                        else
                            link_length = sqrt((double)pseudo_component->lookup_in_library("chip_area")*1e12); // Chip area in um^2
                        
                        if(link_length == 0.0) {
                            pseudo_component->kitfox->error("mcpat","%s.library has invalid link_length",pseudo_component->name.c_str());
                        }
                        break;
                    }
                    case KITFOX_MCPAT_NOC_ROUTER: {
                        XML_interface.sys.NoC[0].type = 1;
                        
                        XML_interface.sys.NoC[0].input_ports = pseudo_component->lookup_in_library("input_ports");
                        XML_interface.sys.NoC[0].output_ports = pseudo_component->lookup_in_library("output_port");
                        XML_interface.sys.NoC[0].virtual_channel_per_port = pseudo_component->lookup_in_library("virtual_channels");
                        XML_interface.sys.NoC[0].input_buffer_entries_per_vc = pseudo_component->lookup_in_library("input_buffer_entries");
                        break;
                    }
                    default: break;
                }
                
                // Number of nodes is set by scaling option
                XML_interface.sys.NoC[0].vertical_nodes = 1;
                XML_interface.sys.NoC[0].horizontal_nodes = 1;
                
                XML_interface.sys.NoC[0].flit_bits = pseudo_component->lookup_in_library("flit_bits");
                if(pseudo_component->exists_in_library("duty_cycle"))
                    XML_interface.sys.NoC[0].duty_cycle = pseudo_component->lookup_in_library("duty_cycle");
                else
                    XML_interface.sys.NoC[0].duty_cycle = 1.0;
                
                double traffic_pattern;
                if(pseudo_component->exists_in_library("traffic_pattern"))
                    traffic_pattern = pseudo_component->lookup_in_library("traffic_pattern");
                else
                    traffic_pattern = 1.0;
                
                McPAT_NoC = new NoC(&XML_interface,0,&input_p,traffic_pattern,link_length);
                
                if(!McPAT_NoC) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create NoC",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_PCIE_CONTROLLER: {
                XML_interface.sys.pcie.clockrate = clock_frequency * 1e-6; // in MHz
                
                // Number of PCIe is set by scaling option
                XML_interface.sys.pcie.number_units = 1;
                
                const char *type_str = pseudo_component->lookup_in_library("type");
                
                if(!strcasecmp(type_str,"high_performance"))
                    XML_interface.sys.pcie.type = 0;
                else if(!strcasecmp(type_str,"low_power"))
                    XML_interface.sys.pcie.type = 1;
                else {
                    pseudo_component->kitfox->error("mcpat","%s.library.type = %s is unknown",pseudo_component->name.c_str(),type_str);
                }
                
                if(pseudo_component->exists_in_library("load_percentage"))
                    XML_interface.sys.pcie.total_load_perc = pseudo_component->lookup_in_library("load_percentage");
                else
                    XML_interface.sys.pcie.total_load_perc = 1.0;
                
                if(pseudo_component->exists_in_library("withPHY"))
                    XML_interface.sys.pcie.withPHY = (bool)pseudo_component->lookup_in_library("withPHY");
                else
                    XML_interface.sys.pcie.withPHY = false;
                
                XML_interface.sys.pcie.num_channels = pseudo_component->lookup_in_library("num_channels");
                
                McPAT_PCIeController = new PCIeController(&XML_interface,&input_p);
                
                if(!McPAT_PCIeController) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create PCIeController",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_PIPELINE: {
                core_p.Embedded = XML_interface.sys.Embedded;
                
                input_p.pipeline_stages = pseudo_component->lookup_in_library("pipeline_stages");
                
                if(pseudo_component->exists_in_library("per_stage_vector"))
                    input_p.per_stage_vector = pseudo_component->lookup_in_library("per_stage_vector");
                else
                    input_p.per_stage_vector = 0;
                
                if(input_p.per_stage_vector == 0) { // McPAT calculates stage vector
                    core_p.pipeline_stages = input_p.pipeline_stages;
                    
                    if(pseudo_component->exists_in_library("x86"))
                        core_p.x86 = (bool)pseudo_component->lookup_in_library("x86");
                    else
                        core_p.x86 = false;
                    
                    if(core_p.x86)
                        core_p.micro_opcode_length = pseudo_component->lookup_in_library("micro_opcode_length");
                    else
                        core_p.opcode_length = pseudo_component->lookup_in_library("opcode_length");
                    
                    core_p.pc_width = pseudo_component->lookup_in_library("pc_width");
                    core_p.fetchW = pseudo_component->lookup_in_library("fetch_width");
                    core_p.decodeW = pseudo_component->lookup_in_library("decode_width");
                    core_p.issueW = pseudo_component->lookup_in_library("issue_width");
                    core_p.instruction_length = pseudo_component->lookup_in_library("instruction_length");
                    core_p.int_data_width = pseudo_component->lookup_in_library("int_data_width");
                    if(pseudo_component->exists_in_library("num_hthreads"))
                        core_p.num_hthreads = pseudo_component->lookup_in_library("num_hthreads");
                    else
                        core_p.num_hthreads = 1;
                    core_p.multithreaded = (bool)(core_p.num_hthreads > 1);
                    if(core_p.multithreaded)
                        core_p.perThreadState = pseudo_component->lookup_in_library("thread_states");
                    
                    if(core_p.core_ty == Inorder) {
                        core_p.arch_ireg_width = pseudo_component->lookup_in_library("arch_int_regs");
                        core_p.arch_ireg_width = int(ceil(log2(core_p.arch_ireg_width)));
                    }
                    else {
                        core_p.phy_ireg_width = pseudo_component->lookup_in_library("phy_int_regs");
                        core_p.phy_ireg_width = int(ceil(log2(core_p.phy_ireg_width)));
                        core_p.v_address_width = pseudo_component->lookup_in_library("virtual_address_width");
                        core_p.commitW = pseudo_component->lookup_in_library("commit_width");
                    }
                }
                
                McPAT_Pipeline = new Pipeline(&input_p,core_p,device_ty,(device_ty==Core_device)
                                              &&(input_p.per_stage_vector==0),true);
                
                if(!McPAT_Pipeline) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create Pipeline",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_SELECTION_LOGIC: {
                int input = pseudo_component->lookup_in_library("selection_input");
                int output = pseudo_component->lookup_in_library("selection_output");
                
                McPAT_selection_logic = new selection_logic(true,input,output,&input_p,device_ty,
                                                            core_p.core_ty);
                
                if(!McPAT_selection_logic) {
                    pseudo_component->kitfox->error("mcpat","%s.library failed to create selection_logic",pseudo_component->name.c_str());
                }
                break;
            }
            case KITFOX_MCPAT_UNDIFF_CORE: {
                core_p.pipeline_stages = pseudo_component->lookup_in_library("pipeline_stages");
                core_p.issueW = pseudo_component->lookup_in_library("issue_width");

                if(pseudo_component->exists_in_library("num_hthreads"))
                    core_p.num_hthreads = pseudo_component->lookup_in_library("num_hthreads");
                else
                    core_p.num_hthreads = 1;
                
                if(XML_interface.sys.Embedded) {
                    XML_interface.sys.opt_clockrate = (bool)pseudo_component->lookup_in_library("opt_clockrate");
                }
                
                McPAT_UndiffCore = new UndiffCore(&XML_interface,0,&input_p,core_p);
                
                if(!McPAT_UndiffCore) {
                    pseudo_component->kitfox->error("mcpat","%s.library.failed to create UndiffCore",pseudo_component->name.c_str());
                }
                break;
            }
            default: break;
        }
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
    
    input_p = *g_ip; // Copy back error-corrected arch parameters
    tech_p = g_tp; // Copy back initialzed tech parameters
    
    // McPAT technology defines a default voltage for each technology node.
    double voltage = 0.0;
    if(pseudo_component->exists_in_library("voltage",true))
        voltage = pseudo_component->lookup_in_library("voltage",true);
    if(voltage > 0.0)
        update_library_variable(KITFOX_MCPAT_UPDATE_VAR_VOLTAGE,&voltage,true);
    
     /*
     // architectural parameters debugging
     cout << "cache_sz: " << g_ip->cache_sz << endl;
     cout << "line_sz: " << g_ip->line_sz << endl;
     cout << "assoc: " << g_ip->assoc << endl;
     cout << "nbanks: " << g_ip->nbanks << endl;
     cout << "out_w: " << g_ip->out_w << endl;
     cout << "specific_tag: " << g_ip->specific_tag << endl;
     cout << "tag_w: " << g_ip->tag_w << endl;
     cout << "access_mode: " << g_ip->access_mode << endl;
     cout << "obj_func_dyn_energy: " << g_ip->obj_func_dyn_energy << endl;
     cout << "obj_func_dyn_power: " << g_ip->obj_func_dyn_power << endl;
     cout << "obj_func_leak_power: " << g_ip->obj_func_leak_power << endl;
     cout << "obj_func_cycle_t: " << g_ip->obj_func_cycle_t << endl;
     cout << "F_sz_nm: " << g_ip->F_sz_nm << endl;
     cout << "F_sz_um: " << g_ip->F_sz_um << endl;
     cout << "num_rw_ports: " << g_ip->num_rw_ports << endl;
     cout << "num_rd_ports: " << g_ip->num_rd_ports << endl;
     cout << "num_wr_ports: " << g_ip->num_wr_ports << endl;
     cout << "num_se_rd_ports: " << g_ip->num_se_rd_ports << endl;
     cout << "num_search_ports: " << g_ip->num_search_ports << endl;
     cout << "is_main_mem: " << g_ip->is_main_mem << endl;
     cout << "is_cache: " << g_ip->is_cache << endl;
     cout << "pure_ram: " << g_ip->pure_ram << endl;
     cout << "pure_cam: " << g_ip->pure_cam << endl;
     cout << "rpters_in_htree: " << g_ip->rpters_in_htree << endl;
     cout << "ver_htree_wires_over_array: " << g_ip->ver_htree_wires_over_array << endl;
     cout << "broadcast_addr_din_over_ver_htrees: " << g_ip->broadcast_addr_din_over_ver_htrees << endl;
     cout << "temp: " << g_ip->temp << endl;
     cout << "ram_cell_tech_type: " << g_ip->ram_cell_tech_type << endl;
     cout << "peri_global_tech_type: " << g_ip->peri_global_tech_type << endl;
     cout << "data_arr_ram_cell_tech_type: " << g_ip->data_arr_ram_cell_tech_type << endl;
     cout << "data_arr_peri_global_tech_type: " << g_ip->data_arr_peri_global_tech_type << endl;
     cout << "tag_arr_ram_cell_tech_type: " << g_ip->tag_arr_ram_cell_tech_type << endl;
     cout << "tag_arr_peri_global_tech_type: " << g_ip->tag_arr_peri_global_tech_type << endl;
     cout << "burst_len: " << g_ip->burst_len << endl;
     cout << "int_prefetch_w: " << g_ip->int_prefetch_w << endl;
     cout << "page_sz_bits: " << g_ip->page_sz_bits << endl;
     cout << "ic_proj_type: " << g_ip->ic_proj_type << endl;
     cout << "wire_is_mat_type: " << g_ip->wire_is_mat_type << endl;
     cout << "wire_os_mat_type: " << g_ip->wire_os_mat_type << endl;
     cout << "Wiretype: " << g_ip->wt << endl;
     cout << "force_wiretype: " << g_ip->force_wiretype << endl;
     cout << "print_input_args: " << g_ip->print_input_args << endl;
     cout << " nuca_cache_sz: " << g_ip->nuca_cache_sz << endl;
     cout << "ndbl: " << g_ip->ndbl << endl;
     cout << "ndwl: " << g_ip->ndwl << endl;
     cout << "nspd: " << g_ip->nspd << endl;
     cout << "ndsam1: " << g_ip->ndsam1 << endl;
     cout << "ndsam2: " << g_ip->ndsam2 << endl;
     cout << "ndcm: " << g_ip->ndcm << endl;
     cout << "force_cache_config: " << g_ip->force_cache_config << endl;
     cout << "cache_level: " << g_ip->cache_level << endl;
     cout << "cores: " << g_ip->cores << endl;
     cout << "nuca_bank_count: " << g_ip->nuca_bank_count << endl;
     cout << "force_nuca_bank: " << g_ip->force_nuca_bank << endl;
     cout << "delay_wt: " << g_ip->delay_wt << endl;
     cout << "dynamic_power_wt: " << g_ip->dynamic_power_wt << endl;
     cout << "leakage_power_wt: " << g_ip->leakage_power_wt << endl;
     cout << "cycle_time_wt: " << g_ip->cycle_time_wt << endl;
     cout << "area_wt: " << g_ip->area_wt << endl;
     cout << "delay_wt_nuca: " << g_ip->delay_wt_nuca << endl;
     cout << "dynamic_power_wt_nuca: " << g_ip->dynamic_power_wt_nuca << endl;
     cout << "leakage_power_wt_nuca: " << g_ip->leakage_power_wt_nuca << endl;
     cout << "cycle_time_wt_nuca: " << g_ip->cycle_time_wt_nuca << endl;
     cout << "area_wt_nuca: " << g_ip->area_wt_nuca << endl;
     cout << "delay_dev: " << g_ip->delay_dev << endl;
     cout << "dynamic_power_dev: " << g_ip->dynamic_power_dev << endl;
     cout << "leakage_power_dev: " << g_ip->leakage_power_dev << endl;
     cout << "cycle_time_dev: " << g_ip->cycle_time_dev << endl;
     cout << "area_dev: " << g_ip->area_dev << endl;
     cout << "delay_dev_nuca: " << g_ip->delay_dev_nuca << endl;
     cout << "dynamic_power_dev_nuca: " << g_ip->dynamic_power_dev_nuca << endl;
     cout << "leakage_power_dev_nuca: " << g_ip->leakage_power_dev_nuca << endl;
     cout << "cycle_time_dev_nuca: " << g_ip->cycle_time_dev_nuca << endl;
     cout << "area_dev_nuca: " << g_ip->area_dev_nuca << endl;
     cout << "ed: " << g_ip->ed << endl;
     cout << "nuca: " << g_ip->nuca << endl;
     cout << "fast_access: " << g_ip->fast_access << endl;
     cout << "block_sz: " << g_ip->block_sz << endl;
     cout << "tag_assoc: " << g_ip->tag_assoc << endl;
     cout << "data_assoc: " << g_ip->data_assoc << endl;
     cout << "is_seq_acc: " << g_ip->is_seq_acc << endl;
     cout << "fully_assoc: " << g_ip->fully_assoc << endl;
     cout << "nsets: " << g_ip->nsets << endl;
     cout << "print_detail: " << g_ip->print_detail << endl;
     cout << "add_ecc_b_: " << g_ip->add_ecc_b_ << endl;
     cout << "throughput: " << g_ip->throughput << endl;
     cout << "latency: " << g_ip->latency << endl;
     cout << "pipelinable: " << g_ip->pipelinable << endl;
     cout << "pipeline_stages: " << g_ip->pipeline_stages << endl;
     cout << "per_stage_vector: " << g_ip->per_stage_vector << endl;
     cout << "with_clock_grid: " << g_ip->with_clock_grid << endl;
     cout << "\n" << endl;
     */
     /*
     // technology parameters debugging
     cout << "g_tp.peri_global.Vdd = " << g_tp.peri_global.Vdd << endl;
     cout << "g_tp.peri_global.Vth = " << g_tp.peri_global.Vth << endl;
     cout << "g_tp.peri_global.l_phy = " << g_tp.peri_global.l_phy << endl;
     cout << "g_tp.peri_global.l_elec = " << g_tp.peri_global.l_elec << endl;
     cout << "g_tp.peri_global.t_ox = " << g_tp.peri_global.t_ox << endl;
     cout << "g_tp.peri_global.C_ox = " << g_tp.peri_global.C_ox << endl;
     cout << "g_tp.peri_global.C_g_ideal = " << g_tp.peri_global.C_g_ideal << endl;
     cout << "g_tp.peri_global.C_fringe = " << g_tp.peri_global.C_fringe << endl;
     cout << "g_tp.peri_global.C_overlap = " << g_tp.peri_global.C_overlap << endl;
     cout << "g_tp.peri_global.C_junc = " << g_tp.peri_global.C_junc << endl;
     cout << "g_tp.peri_global.C_junc_sidewall = " << g_tp.peri_global.C_junc_sidewall << endl;
     cout << "g_tp.peri_global.I_on_n = " << g_tp.peri_global.I_on_n << endl;
     cout << "g_tp.peri_global.I_on_p = " << g_tp.peri_global.I_on_p << endl;
     cout << "g_tp.peri_global.R_nch_on = " << g_tp.peri_global.R_nch_on << endl;
     cout << "g_tp.peri_global.R_pch_on = " << g_tp.peri_global.R_pch_on << endl;
     cout << "g_tp.peri_global.n_to_p_eff_curr_drv_ratio = " << g_tp.peri_global.n_to_p_eff_curr_drv_ratio << endl;
     cout << "g_tp.peri_global.long_channel_leakage_reduction = " << g_tp.peri_global.long_channel_leakage_reduction << endl;
     cout << "g_tp.peri_global.I_off_n = " << g_tp.peri_global.I_off_n << endl;
     cout << "g_tp.peri_global.I_off_p = " << g_tp.peri_global.I_off_p << endl;
     cout << "g_tp.peri_global.I_g_on_n = " << g_tp.peri_global.I_g_on_n << endl;
     cout << "g_tp.peri_global.I_g_on_p = " << g_tp.peri_global.I_g_on_p << endl;
     cout << "g_tp.sram_cell.Vdd = " << g_tp.sram_cell.Vdd << endl;
     cout << "g_tp.sram_cell.Vth = " << g_tp.sram_cell.Vth << endl;
     cout << "g_tp.sram_cell.l_phy = " << g_tp.sram_cell.l_phy << endl;
     cout << "g_tp.sram_cell.l_elec = " << g_tp.sram_cell.l_elec << endl;
     cout << "g_tp.sram_cell.t_ox = " << g_tp.sram_cell.t_ox << endl;
     cout << "g_tp.sram_cell.C_ox = " << g_tp.sram_cell.C_ox << endl;
     cout << "g_tp.sram_cell.C_g_ideal = " << g_tp.sram_cell.C_g_ideal << endl;
     cout << "g_tp.sram_cell.C_fringe = " << g_tp.sram_cell.C_fringe << endl;
     cout << "g_tp.sram_cell.C_overlap = " << g_tp.sram_cell.C_overlap << endl;
     cout << "g_tp.sram_cell.C_junc = " << g_tp.sram_cell.C_junc << endl;
     cout << "g_tp.sram_cell.C_junc_sidewall = " << g_tp.sram_cell.C_junc_sidewall << endl;
     cout << "g_tp.sram_cell.I_on_n = " << g_tp.sram_cell.I_on_n << endl;
     cout << "g_tp.sram_cell.I_on_p = " << g_tp.sram_cell.I_on_p << endl;
     cout << "g_tp.sram_cell.R_nch_on = " << g_tp.sram_cell.R_nch_on << endl;
     cout << "g_tp.sram_cell.R_pch_on = " << g_tp.sram_cell.R_pch_on << endl;
     cout << "g_tp.sram_cell.n_to_p_eff_curr_drv_ratio = " << g_tp.sram_cell.n_to_p_eff_curr_drv_ratio << endl;
     cout << "g_tp.sram_cell.long_channel_leakage_reduction = " << g_tp.sram_cell.long_channel_leakage_reduction << endl;
     cout << "g_tp.sram_cell.I_off_n = " << g_tp.sram_cell.I_off_n << endl;
     cout << "g_tp.sram_cell.I_off_p = " << g_tp.sram_cell.I_off_p << endl;
     cout << "g_tp.sram_cell.I_g_on_n = " << g_tp.sram_cell.I_g_on_n << endl;
     cout << "g_tp.sram_cell.I_g_on_p = " << g_tp.sram_cell.I_g_on_p << endl;
     cout << "g_tp.cam_cell.Vdd = " << g_tp.cam_cell.Vdd << endl;
     cout << "g_tp.cam_cell.Vth = " << g_tp.cam_cell.Vth << endl;
     cout << "g_tp.cam_cell.l_phy = " << g_tp.cam_cell.l_phy << endl;
     cout << "g_tp.cam_cell.l_elec = " << g_tp.cam_cell.l_elec << endl;
     cout << "g_tp.cam_cell.t_ox = " << g_tp.cam_cell.t_ox << endl;
     cout << "g_tp.cam_cell.C_ox = " << g_tp.cam_cell.C_ox << endl;
     cout << "g_tp.cam_cell.C_g_ideal = " << g_tp.cam_cell.C_g_ideal << endl;
     cout << "g_tp.cam_cell.C_fringe = " << g_tp.cam_cell.C_fringe << endl;
     cout << "g_tp.cam_cell.C_overlap = " << g_tp.cam_cell.C_overlap << endl;
     cout << "g_tp.cam_cell.C_junc = " << g_tp.cam_cell.C_junc << endl;
     cout << "g_tp.cam_cell.C_junc_sidewall = " << g_tp.cam_cell.C_junc_sidewall << endl;
     cout << "g_tp.cam_cell.I_on_n = " << g_tp.cam_cell.I_on_n << endl;
     cout << "g_tp.cam_cell.I_on_p = " << g_tp.cam_cell.I_on_p << endl;
     cout << "g_tp.cam_cell.R_nch_on = " << g_tp.cam_cell.R_nch_on << endl;
     cout << "g_tp.cam_cell.R_pch_on = " << g_tp.cam_cell.R_pch_on << endl;
     cout << "g_tp.cam_cell.n_to_p_eff_curr_drv_ratio = " << g_tp.cam_cell.n_to_p_eff_curr_drv_ratio << endl;
     cout << "g_tp.cam_cell.long_channel_leakage_reduction = " << g_tp.cam_cell.long_channel_leakage_reduction << endl;
     cout << "g_tp.cam_cell.I_off_n = " << g_tp.cam_cell.I_off_n << endl;
     cout << "g_tp.cam_cell.I_off_p = " << g_tp.cam_cell.I_off_p << endl;
     cout << "g_tp.cam_cell.I_g_on_n = " << g_tp.cam_cell.I_g_on_n << endl;
     cout << "g_tp.cam_cell.I_g_on_p = " << g_tp.cam_cell.I_g_on_p << endl;
     cout << "g_tp.wire_local.pitch = " << g_tp.wire_local.pitch << endl;
     cout << "g_tp.wire_local.R_per_um = " << g_tp.wire_local.R_per_um << endl;
     cout << "g_tp.wire_local.C_per_um = " << g_tp.wire_local.C_per_um << endl;
     cout << "g_tp.wire_local.horiz_dielectric_constant = " << g_tp.wire_local.horiz_dielectric_constant << endl;
     cout << "g_tp.wire_local.vert_dielectric_constant = " << g_tp.wire_local.vert_dielectric_constant << endl;
     cout << "g_tp.wire_local.aspect_ratio = " << g_tp.wire_local.aspect_ratio << endl;
     cout << "g_tp.wire_local.ild_thickness = " << g_tp.wire_local.ild_thickness << endl;
     cout << "g_tp.wire_local.miller_value = " << g_tp.wire_local.miller_value << endl;
     cout << "g_tp.wire_inside_mat.pitch = " << g_tp.wire_inside_mat.pitch << endl;
     cout << "g_tp.wire_inside_mat.R_per_um = " << g_tp.wire_inside_mat.R_per_um << endl;
     cout << "g_tp.wire_inside_mat.C_per_um = " << g_tp.wire_inside_mat.C_per_um << endl;
     cout << "g_tp.wire_inside_mat.horiz_dielectric_constant = " << g_tp.wire_inside_mat.horiz_dielectric_constant << endl;
     cout << "g_tp.wire_inside_mat.vert_dielectric_constant = " << g_tp.wire_inside_mat.vert_dielectric_constant << endl;
     cout << "g_tp.wire_inside_mat.aspect_ratio = " << g_tp.wire_inside_mat.aspect_ratio << endl;
     cout << "g_tp.wire_inside_mat.ild_thickness = " << g_tp.wire_inside_mat.ild_thickness << endl;
     cout << "g_tp.wire_inside_mat.miller_value = " << g_tp.wire_inside_mat.miller_value << endl;
     cout << "g_tp.wire_outside_mat.pitch = " << g_tp.wire_outside_mat.pitch << endl;
     cout << "g_tp.wire_outside_mat.R_per_um = " << g_tp.wire_outside_mat.R_per_um << endl;
     cout << "g_tp.wire_outside_mat.C_per_um = " << g_tp.wire_outside_mat.C_per_um << endl;
     cout << "g_tp.wire_outside_mat.horiz_dielectric_constant = " << g_tp.wire_outside_mat.horiz_dielectric_constant << endl;
     cout << "g_tp.wire_outside_mat.vert_dielectric_constant = " << g_tp.wire_outside_mat.vert_dielectric_constant << endl;
     cout << "g_tp.wire_outside_mat.aspect_ratio = " << g_tp.wire_outside_mat.aspect_ratio << endl;
     cout << "g_tp.wire_outside_mat.ild_thickness = " << g_tp.wire_outside_mat.ild_thickness << endl;
     cout << "g_tp.wire_outside_mat.miller_value = " << g_tp.wire_outside_mat.miller_value << endl;
     cout << "g_tp.scaling_factor.logic_scaling_co_eff = " << g_tp.scaling_factor.logic_scaling_co_eff << endl;
     cout << "g_tp.scaling_factor.core_tx_density = " << g_tp.scaling_factor.core_tx_density << endl;
     cout << "g_tp.scaling_factor.long_channel_leakage_reduction = " << g_tp.scaling_factor.long_channel_leakage_reduction << endl;
     cout << "g_tp.sram.b_w = " << g_tp.sram.b_w << endl;
     cout << "g_tp.sram.b_h = " << g_tp.sram.b_h << endl;
     cout << "g_tp.sram.cell_a_w = " << g_tp.sram.cell_a_w << endl;
     cout << "g_tp.sram.cell_pmos_w = " << g_tp.sram.cell_pmos_w << endl;
     cout << "g_tp.sram.cell_nmos_w = " << g_tp.sram.cell_nmos_w << endl;
     cout << "g_tp.sram.Vitbpre = " << g_tp.sram.Vbitpre << endl;
     cout << "g_tp.cam.b_w = " << g_tp.cam.b_w << endl;
     cout << "g_tp.cam.b_h = " << g_tp.cam.b_h << endl;
     cout << "g_tp.cam.cell_a_w = " << g_tp.cam.cell_a_w << endl;
     cout << "g_tp.cam.cell_pmos_w = " << g_tp.cam.cell_pmos_w << endl;
     cout << "g_tp.cam.cell_nmos_w = " << g_tp.cam.cell_nmos_w << endl;
     cout << "g_tp.cam.Vitbpre = " << g_tp.cam.Vbitpre << endl;
     cout << "g_tp.ram_wl_stitching_overhead_ = " << g_tp.ram_wl_stitching_overhead_ << endl;
     cout << "g_tp.min_w_nmos_ = " << g_tp.min_w_nmos_ << endl;
     cout << "g_tp.max_w_nmos_ = " << g_tp.max_w_nmos_ << endl;
     cout << "g_tp.max_w_nmos_dec = " << g_tp.max_w_nmos_dec << endl;
     cout << "g_tp.unit_len_wire_del = " << g_tp.unit_len_wire_del << endl;
     cout << "g_tp.FO4 = " << g_tp.FO4 << endl;
     cout << "g_tp.kinv = " << g_tp.kinv << endl;
     cout << "g_tp.vpp = " << g_tp.vpp << endl;
     cout << "g_tp.w_sense_en = " << g_tp.w_sense_en << endl;
     cout << "g_tp.w_sense_n = " << g_tp.w_sense_n << endl;
     cout << "g_tp.w_sense_p = " << g_tp.w_sense_p << endl;
     cout << "g_tp.sense_delay = " << g_tp.sense_delay << endl;
     cout << "g_tp.sense_dy_power = " << g_tp.sense_dy_power << endl;
     cout << "g_tp.w_iso = " << g_tp.w_iso << endl;
     cout << "g_tp.w_poly_contact = " << g_tp.w_poly_contact << endl;
     cout << "g_tp.spacing_poly_to_contact = " << g_tp.spacing_poly_to_contact << endl;
     cout << "g_tp.spacing_poly_to_poly = " << g_tp.spacing_poly_to_poly << endl;
     cout << "g_tp.w_comp_inv_p1 = " << g_tp.w_comp_inv_p1 << endl;
     cout << "g_tp.w_comp_inv_n1 = " << g_tp.w_comp_inv_n1 << endl;
     cout << "g_tp.w_comp_inv_p2 = " << g_tp.w_comp_inv_p2 << endl;
     cout << "g_tp.w_comp_inv_n2 = " << g_tp.w_comp_inv_n2 << endl;
     cout << "g_tp.w_comp_inv_p3 = " << g_tp.w_comp_inv_p3 << endl;
     cout << "g_tp.w_comp_inv_n3 = " << g_tp.w_comp_inv_n3 << endl;
     cout << "g_tp.w_eval_inv_p = " << g_tp.w_eval_inv_p << endl;
     cout << "g_tp.w_eval_inv_n = " << g_tp.w_eval_inv_n << endl;
     cout << "g_tp.w_comp_n = " << g_tp.w_comp_n << endl;
     cout << "g_tp.w_comp_p = " << g_tp.w_comp_p << endl;
     cout << "g_tp.dram_cell_I_on = " << g_tp.dram_cell_I_on << endl;
     cout << "g_tp.dram_cell_Vdd = " << g_tp.dram_cell_Vdd << endl;
     cout << "g_tp.dram_cell_I_off_worst_case_len_temp = " << g_tp.dram_cell_I_off_worst_case_len_temp << endl;
     cout << "g_tp.dram_cell_C = " << g_tp.dram_cell_C << endl;
     cout << "g_tp.gm_sense_amp_latch = " << g_tp.gm_sense_amp_latch << endl;
     cout << "g_tp.w_nmos_b_mux = " << g_tp.w_nmos_b_mux << endl;
     cout << "g_tp.w_nmos_sa_mux = " << g_tp.w_nmos_sa_mux << endl;
     cout << "g_tp.w_pmos_bl_precharge = " << g_tp.w_pmos_bl_precharge << endl;
     cout << "g_tp.w_pmos_bl_eq = " << g_tp.w_pmos_bl_eq << endl;
     cout << "g_tp.MIN_GAP_BET_P_AND_N_DIFFS = " << g_tp.MIN_GAP_BET_P_AND_N_DIFFS << endl;
     cout << "g_tp.MIN_GAP_BET_SAME_TYPE_DIFFS = " << g_tp.MIN_GAP_BET_SAME_TYPE_DIFFS << endl;
     cout << "g_tp.HPOWERRAIL = " << g_tp.HPOWERRAIL << endl;
     cout << "g_tp.cell_h_def = " << g_tp.cell_h_def << endl;
     cout << "g_tp.chip_layout_overhead = " << g_tp.chip_layout_overhead << endl;
     cout << "g_tp.macro_layout_overhead = " << g_tp.macro_layout_overhead << endl;
     cout << "g_tp.sckt_co_eff = " << g_tp.sckt_co_eff << endl;
     cout << "g_tp.fringe_cap = " << g_tp.fringe_cap << endl;
     cout << "g_tp.h_dec = " << g_tp.h_dec << endl;
     
     cout << "g_tp.dram_acc.Vth = " << g_tp.dram_acc.Vth << endl;
     cout << "g_tp.dram_acc.l_phy = " << g_tp.dram_acc.l_phy << endl;
     cout << "g_tp.dram_acc.l_elec = " << g_tp.dram_acc.l_elec << endl;
     cout << "g_tp.dram_acc.C_g_ideal = " << g_tp.dram_acc.C_g_ideal << endl;
     cout << "g_tp.dram_acc.C_fringe = " << g_tp.dram_acc.C_fringe << endl;
     cout << "g_tp.dram_acc.C_junc = " << g_tp.dram_acc.C_junc << endl;
     cout << "g_tp.dram_acc.C_junc_sidewall = " << g_tp.dram_acc.C_junc_sidewall << endl;
     cout << "g_tp.dram_acc.I_on_n = " << g_tp.dram_acc.I_on_n << endl;
     cout << "g_tp.dram_wl.l_phy = " << g_tp.dram_wl.l_phy << endl;
     cout << "g_tp.dram_wl.l_elec = " << g_tp.dram_wl.l_elec << endl;
     cout << "g_tp.dram_wl.C_g_ideal = " << g_tp.dram_wl.C_g_ideal << endl;
     cout << "g_tp.dram_wl.C_fringe = " << g_tp.dram_wl.C_fringe << endl;
     cout << "g_tp.dram_wl.C_junc = " << g_tp.dram_wl.C_junc << endl;
     cout << "g_tp.dram_wl.C_junc_sidewall = " << g_tp.dram_wl.C_junc_sidewall << endl;
     cout << "g_tp.dram_wl.I_on_n = " << g_tp.dram_wl.I_on_n << endl;
     cout << "g_tp.dram_wl.R_nch_on = " << g_tp.dram_wl.R_nch_on << endl;
     cout << "g_tp.dram_wl.R_pch_on = " << g_tp.dram_wl.R_pch_on << endl;
     cout << "g_tp.dram_wl.n_to_p_eff_curr_drv_ratio = " << g_tp.dram_wl.n_to_p_eff_curr_drv_ratio << endl;
     cout << "g_tp.dram_wl.long_channel_leakage_reduction = " << g_tp.dram_wl.long_channel_leakage_reduction << endl;
     cout << "g_tp.dram_wl.I_off_n = " << g_tp.dram_wl.I_off_n << endl;
     cout << "g_tp.dram_wl.I_off_p = " << g_tp.dram_wl.I_off_p << endl;
     */  
}

unit_energy_t energylib_mcpat::get_unit_energy() {
    unit_energy_t unit_energy;
    
    switch(energy_model) {
        case KITFOX_MCPAT_ARRAYST: {
            unit_energy.read = McPAT_ArrayST->local_result.power.readOp.dynamic;
            unit_energy.write = McPAT_ArrayST->local_result.power.writeOp.dynamic;
            
            if(McPAT_ArrayST->l_ip.tag_w > 0) {
                if(McPAT_ArrayST->local_result.tag_array2) { // tag array exists
                    unit_energy.read_tag = McPAT_ArrayST->local_result.tag_array2->power.readOp.dynamic
                    +McPAT_ArrayST->local_result.tag_array2->power.readOp.short_circuit;
                    unit_energy.write_tag = McPAT_ArrayST->local_result.tag_array2->power.writeOp.dynamic
                    +McPAT_ArrayST->local_result.tag_array2->power.writeOp.short_circuit;
                }
                else {
                    unit_energy.read_tag = unit_energy.read;
                    unit_energy.write_tag = unit_energy.write;
                }
            }
            
            if(McPAT_ArrayST->l_ip.num_search_ports)
                unit_energy.search = McPAT_ArrayST->local_result.power.searchOp.dynamic
                +McPAT_ArrayST->local_result.power.searchOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_ArrayST->local_result.power.readOp.longer_channel_leakage
                                       +McPAT_ArrayST->local_result.power.readOp.gate_leakage)
                /clock_frequency;
            else
                unit_energy.leakage = (McPAT_ArrayST->local_result.power.readOp.leakage
                                       +McPAT_ArrayST->local_result.power.readOp.gate_leakage)
                /clock_frequency;
            break;
        }
        case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
            unit_energy.switching = McPAT_dep_resource_conflict_check->power.readOp.dynamic
            +McPAT_dep_resource_conflict_check->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_dep_resource_conflict_check->power.readOp.longer_channel_leakage
                                       +McPAT_dep_resource_conflict_check->power.readOp.gate_leakage)
                /clock_frequency;
            else
                unit_energy.leakage = (McPAT_dep_resource_conflict_check->power.readOp.leakage
                                       +McPAT_dep_resource_conflict_check->power.readOp.gate_leakage)
                /clock_frequency;
            break;
        }
        case KITFOX_MCPAT_FLASH_CONTROLLER: {
            unit_energy.switching = McPAT_FlashController->power_t.readOp.dynamic;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_FlashController->power_t.readOp.longer_channel_leakage
                                       +McPAT_FlashController->power_t.readOp.gate_leakage)
                /clock_frequency;
            else
                unit_energy.leakage = (McPAT_FlashController->power_t.readOp.leakage
                                       +McPAT_FlashController->power_t.readOp.gate_leakage)
                /clock_frequency;
            break;
        }
        case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
            //unit_energy.baseline = McPAT_FunctionalUnit->base_energy/clock_frequency*tech_p.sckt_co_eff;
            unit_energy.switching = McPAT_FunctionalUnit->base_energy/clock_frequency*tech_p.sckt_co_eff/scaling;
            unit_energy.switching += McPAT_FunctionalUnit->per_access_energy*tech_p.sckt_co_eff;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_FunctionalUnit->leakage*longer_channel_device_reduction(device_ty,McPAT_FunctionalUnit->coredynp.core_ty)
                                       +McPAT_FunctionalUnit->gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_FunctionalUnit->leakage+McPAT_FunctionalUnit->gate_leakage)/clock_frequency;
            unit_energy.leakage /= scaling;
            break;
        }
        case KITFOX_MCPAT_INST_DECODER: {
            unit_energy.switching = McPAT_inst_decoder->power.readOp.dynamic+McPAT_inst_decoder->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_inst_decoder->power.readOp.longer_channel_leakage
                                       +McPAT_inst_decoder->power.readOp.gate_leakage)
                /clock_frequency;
            else
                unit_energy.leakage = (McPAT_inst_decoder->power.readOp.leakage
                                       +McPAT_inst_decoder->power.readOp.gate_leakage)
                /clock_frequency;
            
            // x86 instruction decoder leakage does not scale correctly.
            if(core_p.x86)
                unit_energy.leakage *= (area_scaling/4.0);
            break;
        }
        case KITFOX_MCPAT_INTERCONNECT: {
            unit_energy.switching = McPAT_interconnect->power.readOp.dynamic
            +McPAT_interconnect->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_interconnect->power.readOp.longer_channel_leakage
                                       +McPAT_interconnect->power.readOp.gate_leakage)
                /clock_frequency;
            else
                unit_energy.leakage = (McPAT_interconnect->power.readOp.leakage
                                       +McPAT_interconnect->power.readOp.gate_leakage)
                /clock_frequency;
            break;
        }
        case KITFOX_MCPAT_MEMORY_CONTROLLER: {
            // Leakage is calculated here
            McPAT_MCFrontEnd->computeEnergy(true);
                    
            // Single clock time
            McPAT_MCFrontEnd->mcp.executionTime = (double)1.0/McPAT_MCFrontEnd->mcp.clockRate;
                    
            // Separate calculation for read and write energies.
            McPAT_MCFrontEnd->XML->sys.mc.memory_reads = 1;
            McPAT_MCFrontEnd->XML->sys.mc.memory_writes = 0;
            McPAT_MCFrontEnd->computeEnergy(false);
            unit_energy.read += (McPAT_MCFrontEnd->rt_power.readOp.dynamic+
                                 McPAT_MCFrontEnd->rt_power.readOp.short_circuit);
                    
            McPAT_MCFrontEnd->XML->sys.mc.memory_reads = 0;
            McPAT_MCFrontEnd->XML->sys.mc.memory_writes = 1;
            McPAT_MCFrontEnd->computeEnergy(false);
                    
            // McPAT MC saves power.writeOp in readOp
            unit_energy.write += (McPAT_MCFrontEnd->rt_power.readOp.dynamic+
                                  McPAT_MCFrontEnd->rt_power.readOp.short_circuit);
                    
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage += (McPAT_MCFrontEnd->rt_power.readOp.longer_channel_leakage+McPAT_MCFrontEnd->rt_power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage += (McPAT_MCFrontEnd->rt_power.readOp.leakage+McPAT_MCFrontEnd->rt_power.readOp.gate_leakage)/clock_frequency;

            // Leakage is calculated here
            McPAT_MCBackend->computeEnergy(true);
                    
            // Single clock time
            McPAT_MCBackend->mcp.executionTime = (double)1.0/McPAT_MCBackend->mcp.clockRate;
                    
            McPAT_MCBackend->mcp.reads = 1;
            McPAT_MCBackend->mcp.writes = 0;
            McPAT_MCBackend->computeEnergy(false);
            unit_energy.switching += (McPAT_MCBackend->rt_power.readOp.dynamic+
                                      McPAT_MCBackend->rt_power.readOp.short_circuit);
                    
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage += (McPAT_MCBackend->rt_power.readOp.longer_channel_leakage+McPAT_MCBackend->rt_power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage += (McPAT_MCBackend->rt_power.readOp.leakage+McPAT_MCBackend->rt_power.readOp.gate_leakage)/clock_frequency;

            // Leakage is calculated here
            McPAT_MCPHY->computeEnergy(true);
                    
            // Single clock time
            McPAT_MCPHY->mcp.executionTime = (double)1.0/McPAT_MCPHY->mcp.clockRate;
                    
            McPAT_MCPHY->mcp.reads = 1;
            McPAT_MCPHY->mcp.writes = 0;
            McPAT_MCPHY->computeEnergy(false);
            unit_energy.switching += (McPAT_MCPHY->rt_power.readOp.dynamic+
                                      McPAT_MCPHY->rt_power.readOp.short_circuit);
                    
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage += (McPAT_MCPHY->rt_power.readOp.longer_channel_leakage+McPAT_MCPHY->rt_power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage += (McPAT_MCPHY->rt_power.readOp.leakage+McPAT_MCPHY->rt_power.readOp.gate_leakage)/clock_frequency;

            /*switch(energy_submodel) {
                case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                    // Leakage is calculated here
                    McPAT_MCFrontEnd->computeEnergy(true);
                    
                    // Single clock time
                    McPAT_MCFrontEnd->mcp.executionTime = (double)1.0/McPAT_MCFrontEnd->mcp.clockRate;
                    
                    // Separate calculation for read and write energies.
                    McPAT_MCFrontEnd->XML->sys.mc.memory_reads = 1;
                    McPAT_MCFrontEnd->XML->sys.mc.memory_writes = 0;
                    McPAT_MCFrontEnd->computeEnergy(false);
                    unit_energy.read = McPAT_MCFrontEnd->rt_power.readOp.dynamic
                    +McPAT_MCFrontEnd->rt_power.readOp.short_circuit;
                    
                    McPAT_MCFrontEnd->XML->sys.mc.memory_reads = 0;
                    McPAT_MCFrontEnd->XML->sys.mc.memory_writes = 1;
                    McPAT_MCFrontEnd->computeEnergy(false);
                    
                    // McPAT MC saves power.writeOp in readOp
                    unit_energy.write = McPAT_MCFrontEnd->rt_power.readOp.dynamic
                    +McPAT_MCFrontEnd->rt_power.readOp.short_circuit;
                    
                    if(XML_interface.sys.longer_channel_device)
                        unit_energy.leakage = (McPAT_MCFrontEnd->rt_power.readOp.longer_channel_leakage+McPAT_MCFrontEnd->rt_power.readOp.gate_leakage)/clock_frequency;
                    else
                        unit_energy.leakage = (McPAT_MCFrontEnd->rt_power.readOp.leakage+McPAT_MCFrontEnd->rt_power.readOp.gate_leakage)/clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                    // Leakage is calculated here
                    McPAT_MCBackend->computeEnergy(true);
                    
                    // Single clock time
                    McPAT_MCBackend->mcp.executionTime = (double)1.0/McPAT_MCBackend->mcp.clockRate;
                    
                    McPAT_MCBackend->mcp.reads = 1;
                    McPAT_MCBackend->mcp.writes = 0;
                    McPAT_MCBackend->computeEnergy(false);
                    unit_energy.switching = McPAT_MCBackend->rt_power.readOp.dynamic
                    +McPAT_MCBackend->rt_power.readOp.short_circuit;
                    
                    if(XML_interface.sys.longer_channel_device)
                        unit_energy.leakage = (McPAT_MCBackend->rt_power.readOp.longer_channel_leakage+McPAT_MCBackend->rt_power.readOp.gate_leakage)/clock_frequency;
                    else
                        unit_energy.leakage = (McPAT_MCBackend->rt_power.readOp.leakage+McPAT_MCBackend->rt_power.readOp.gate_leakage)/clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                    // Leakage is calculated here
                    McPAT_MCPHY->computeEnergy(true);
                    
                    // Single clock time
                    McPAT_MCPHY->mcp.executionTime = (double)1.0/McPAT_MCPHY->mcp.clockRate;
                    
                    McPAT_MCPHY->mcp.reads = 1;
                    McPAT_MCPHY->mcp.writes = 0;
                    McPAT_MCPHY->computeEnergy(false);
                    unit_energy.switching = McPAT_MCPHY->rt_power.readOp.dynamic
                    +McPAT_MCPHY->rt_power.readOp.short_circuit;
                    
                    if(XML_interface.sys.longer_channel_device)
                        unit_energy.leakage = (McPAT_MCPHY->rt_power.readOp.longer_channel_leakage+McPAT_MCPHY->rt_power.readOp.gate_leakage)/clock_frequency;
                    else
                        unit_energy.leakage = (McPAT_MCPHY->rt_power.readOp.leakage+McPAT_MCPHY->rt_power.readOp.gate_leakage)/clock_frequency;
                    break;
                }
                default: break;
            }*/
            break;
        }
        case KITFOX_MCPAT_NOC: {
            // Leakage is calculated here
            McPAT_NoC->computeEnergy(true);
            
            // Single clock time
            McPAT_NoC->XML->sys.NoC[0].total_accesses = 1;
            McPAT_NoC->computeEnergy(false);
            unit_energy.switching = McPAT_NoC->rt_power.readOp.dynamic
            +McPAT_NoC->rt_power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_NoC->rt_power.readOp.longer_channel_leakage+McPAT_NoC->rt_power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_NoC->rt_power.readOp.leakage+McPAT_NoC->rt_power.readOp.gate_leakage)/clock_frequency;
            break;
        }
        case KITFOX_MCPAT_NIU_CONTROLLER: {
            unit_energy.switching = McPAT_NIUController->power_t.readOp.dynamic;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_NIUController->power_t.readOp.longer_channel_leakage+McPAT_NIUController->power_t.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_NIUController->power_t.readOp.leakage+McPAT_NIUController->power_t.readOp.gate_leakage)/clock_frequency;
            break;
        }
        case KITFOX_MCPAT_PCIE_CONTROLLER: {
            unit_energy.switching = McPAT_PCIeController->power_t.readOp.dynamic;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_PCIeController->power_t.readOp.longer_channel_leakage+McPAT_PCIeController->power_t.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_PCIeController->power_t.readOp.leakage+McPAT_PCIeController->power_t.readOp.gate_leakage)/clock_frequency;
            break;
        }
        case KITFOX_MCPAT_PIPELINE: {
            unit_energy.switching = McPAT_Pipeline->power.readOp.dynamic
            +McPAT_Pipeline->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_Pipeline->power.readOp.longer_channel_leakage+McPAT_Pipeline->power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_Pipeline->power.readOp.leakage+McPAT_Pipeline->power.readOp.gate_leakage)/clock_frequency;
            break;
        }
        case KITFOX_MCPAT_SELECTION_LOGIC: {
            unit_energy.switching = McPAT_selection_logic->power.readOp.dynamic
            +McPAT_selection_logic->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_selection_logic->power.readOp.longer_channel_leakage+McPAT_selection_logic->power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_selection_logic->power.readOp.leakage+McPAT_selection_logic->power.readOp.gate_leakage)/clock_frequency;
            break;
        }
        case KITFOX_MCPAT_UNDIFF_CORE: {
            unit_energy.switching = McPAT_UndiffCore->power.readOp.dynamic
            +McPAT_UndiffCore->power.readOp.short_circuit;
            
            if(XML_interface.sys.longer_channel_device)
                unit_energy.leakage = (McPAT_UndiffCore->power.readOp.longer_channel_leakage+McPAT_UndiffCore->power.readOp.gate_leakage)/clock_frequency;
            else
                unit_energy.leakage = (McPAT_UndiffCore->power.readOp.leakage+McPAT_UndiffCore->power.readOp.gate_leakage)/clock_frequency;
            //unit_energy.leakage = 0.0;
            break;
        }
        default: break;
    }
    
    //unit_energy.baseline *= energy_scaling;
    unit_energy.switching *= energy_scaling;
    unit_energy.read *= energy_scaling;
    unit_energy.read_tag *= energy_scaling;
    unit_energy.write *= energy_scaling;
    unit_energy.write_tag *= energy_scaling;
    unit_energy.search *= energy_scaling;
    unit_energy.leakage *= (energy_scaling*scaling);
    
    return unit_energy;
}

power_t energylib_mcpat::get_tdp_power(Kelvin MaxTemperature) {
    Kelvin current_temperature = (double)input_p.temp;
    Kelvin max_temperature = MaxTemperature;
    update_library_variable(KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE,&max_temperature,true);

    unit_energy_t unit_energy = get_unit_energy();
    energy_t tdp_energy;
    power_t tdp_power;
    
    switch(energy_model) {
        case KITFOX_MCPAT_ARRAYST: {
            tdp_energy.dynamic = TDP_duty_cycle.read*unit_energy.read*(McPAT_ArrayST->l_ip.num_rd_ports?McPAT_ArrayST->l_ip.num_rd_ports:McPAT_ArrayST->l_ip.num_rw_ports);
            tdp_energy.dynamic += TDP_duty_cycle.write*unit_energy.write*(McPAT_ArrayST->l_ip.num_wr_ports?McPAT_ArrayST->l_ip.num_wr_ports:McPAT_ArrayST->l_ip.num_rw_ports);
            
            if(McPAT_ArrayST->l_ip.num_search_ports)
                tdp_energy.dynamic += TDP_duty_cycle.search*unit_energy.search*(McPAT_ArrayST->l_ip.num_search_ports);
            
            if(McPAT_ArrayST->l_ip.tag_w > 0) {
                if(McPAT_ArrayST->local_result.tag_array2) { // tag array exists
                    tdp_energy.dynamic += TDP_duty_cycle.read_tag*unit_energy.read_tag*(McPAT_ArrayST->l_ip.num_rd_ports?McPAT_ArrayST->l_ip.num_rd_ports:McPAT_ArrayST->l_ip.num_rw_ports);
                    tdp_energy.dynamic += TDP_duty_cycle.write_tag*unit_energy.write_tag*(McPAT_ArrayST->l_ip.num_wr_ports?McPAT_ArrayST->l_ip.num_wr_ports:McPAT_ArrayST->l_ip.num_rw_ports);
                }
                else {
                    tdp_energy.dynamic += TDP_duty_cycle.read_tag*unit_energy.read*(McPAT_ArrayST->l_ip.num_rd_ports?McPAT_ArrayST->l_ip.num_rd_ports:McPAT_ArrayST->l_ip.num_rw_ports);
                    tdp_energy.dynamic += TDP_duty_cycle.write_tag*unit_energy.write*(McPAT_ArrayST->l_ip.num_wr_ports?McPAT_ArrayST->l_ip.num_wr_ports:McPAT_ArrayST->l_ip.num_rw_ports);
                }
            }
            break;
        }
        case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_FLASH_CONTROLLER: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
            //tdp_energy.dynamic = unit_energy.baseline;
            tdp_energy.dynamic += TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_INST_DECODER: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            
            // McPAT doesn't scale leakage power correctly for x86 decoders.
            if(core_p.x86)
                tdp_energy.dynamic *= area_scaling;
            break;
        }
        case KITFOX_MCPAT_INTERCONNECT: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_MEMORY_CONTROLLER: {
            tdp_energy.dynamic += TDP_duty_cycle.read*unit_energy.read;
            tdp_energy.dynamic += TDP_duty_cycle.write*unit_energy.write;
            tdp_energy.dynamic += TDP_duty_cycle.switching*unit_energy.switching;

            /*switch(energy_submodel) {
                case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                    tdp_energy.dynamic = TDP_duty_cycle.read*unit_energy.read;
                    tdp_energy.dynamic += TDP_duty_cycle.write*unit_energy.write;
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                    tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                    tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
                    break;
                }
                default: break;
            }*/
            break;
        }
        case KITFOX_MCPAT_NIU_CONTROLLER: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_NOC: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_PCIE_CONTROLLER: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_PIPELINE: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_SELECTION_LOGIC: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        case KITFOX_MCPAT_UNDIFF_CORE: {
            tdp_energy.dynamic = TDP_duty_cycle.switching*unit_energy.switching;
            break;
        }
        default: break;
    }
    
    tdp_energy.leakage = unit_energy.leakage;
    tdp_energy.total = tdp_energy.get_total();
    tdp_power = tdp_energy * clock_frequency;
    
    update_library_variable(KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE,&current_temperature,true);
    
    return tdp_power;
}

power_t energylib_mcpat::get_runtime_power(Second Time, Second Period, counter_t Counter) {
    unit_energy_t unit_energy = get_unit_energy();
    energy_t runtime_energy;
    power_t runtime_power;
    
    //runtime_energy.dynamic = unit_energy.baseline*(double)Counter.switching;
    //runtime_energy.dynamic = unit_energy.baseline*Period*clock_frequency;
    runtime_energy.dynamic += unit_energy.switching*(double)Counter.switching;
    runtime_energy.dynamic += unit_energy.read*(double)Counter.read;
    runtime_energy.dynamic += unit_energy.write*(double)Counter.write;
    runtime_energy.dynamic += unit_energy.read_tag*(double)Counter.read_tag;
    runtime_energy.dynamic += unit_energy.write_tag*(double)Counter.write_tag;
    runtime_energy.dynamic += unit_energy.search*(double)Counter.search;
    runtime_energy.leakage = unit_energy.leakage*Period*clock_frequency;
    runtime_energy.total = runtime_energy.get_total();
    runtime_power = runtime_energy/Period; // conversion from energy to power
    
#ifdef KITFOX_DEBUG
    cout << "KITFOX DEBUG (mcpat): pseudo component " << pseudo_component->name << "[ID=" << pseudo_component->id << "].runtime_power = " << runtime_power.get_total() << "W (energy = " << runtime_energy.get_total() << "J)" << endl;
    
    //double baseline_energy = unit_energy.baseline*Period*clock_frequency;
    //if(unit_energy.baseline > 0.0) cout << "\tbaseline (counterless) = " << baseline_energy/Period << " W (energy = " << baseline_energy << "J)" << endl;
    //baseline_energy = unit_energy.baseline*(double)Counter.switching;
    //if(unit_energy.baseline > 0.0) cout << "\tbaseline (countered) = " << baseline_energy/Period << " W (energy = " << baseline_energy << "J)" << endl;
    
    double read_energy = unit_energy.read*(double)Counter.read;
    if(unit_energy.read > 0.0) cout << "\tread = " << read_energy/Period << "W (energy = " << read_energy << "J)" << endl;
    
    double write_energy = unit_energy.write*(double)Counter.write;
    if(unit_energy.write > 0.0) cout << "\twrite = " << write_energy/Period << "W (energy = " << write_energy << "J)" << endl;
    
    double read_tag_energy = unit_energy.read_tag*(double)Counter.read_tag;
    if(unit_energy.read_tag > 0.0) cout << "\tread_tag = " << read_tag_energy/Period << "W (energy = " << read_tag_energy << "J)" << endl;
    
    double write_tag_energy = unit_energy.write_tag*(double)Counter.write_tag;
    if(unit_energy.write_tag > 0.0) cout << "\twrite_tag = " << write_tag_energy/Period << "W (energy = " << write_tag_energy << "J)" << endl;
    
    double search_energy = unit_energy.search*(double)Counter.search;
    if(unit_energy.search > 0.0) cout << "\tsearch = " << search_energy/Period << "W (energy = " << search_energy << "J)" << endl;
    
    double switching_energy = unit_energy.switching*(double)Counter.switching;
    if(unit_energy.switching > 0.0) cout << "\tswitching = " << switching_energy/Period << "W (energy = " << switching_energy << "J)" << endl;
    
    double leakage_energy = unit_energy.leakage*Period*clock_frequency;
    if(unit_energy.leakage > 0.0) cout << "\tleakage = " << leakage_energy/Period << "W (energy = " << leakage_energy << "J)" << endl;
#endif
    
    return runtime_power;
}

MeterSquare energylib_mcpat::get_area() {
    MeterSquare area = 0.0;
    
    switch(energy_model) {
        case KITFOX_MCPAT_ARRAYST: {
            area = McPAT_ArrayST->local_result.area;
            break;
        }
        case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
            area = McPAT_dep_resource_conflict_check->area.get_area();
            break;
        }
        case KITFOX_MCPAT_FLASH_CONTROLLER: {
            area = McPAT_FlashController->area.get_area();
            break;
        }
        case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
            area = McPAT_FunctionalUnit->area.get_area()/scaling;
            // McPAT doesn't correctly scaling functional unit area
            /*if((energy_submodel == KITFOX_MCPAT_FUNCTIONAL_UNIT_ALU)
               ||(energy_submodel == KITFOX_MCPAT_FUNCTIONAL_UNIT_MUL))
                area *= scaling;*/
            break;
        }
        case KITFOX_MCPAT_INST_DECODER: {
            area = McPAT_inst_decoder->area.get_area();
            break;
        }
        case KITFOX_MCPAT_INTERCONNECT: {
            area = McPAT_interconnect->area.get_area();
            break;
        }
        case KITFOX_MCPAT_MEMORY_CONTROLLER: {
            area += McPAT_MCFrontEnd->area.get_area();
            area += McPAT_MCBackend->area.get_area();
            area += McPAT_MCPHY->area.get_area();

            /*switch(energy_submodel) {
                case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                    area = McPAT_MCFrontEnd->area.get_area();
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                    area = McPAT_MCBackend->area.get_area();
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                    area = McPAT_MCPHY->area.get_area();
                    break;
                }
                default: break;
            }*/
            break;
        }
        case KITFOX_MCPAT_NIU_CONTROLLER: {
            area = McPAT_NIUController->area.get_area();
            break;
        }
        case KITFOX_MCPAT_NOC: {
            area = McPAT_NoC->area.get_area();
            break;
        }
        case KITFOX_MCPAT_PCIE_CONTROLLER: {
            area = McPAT_PCIeController->area.get_area();
            break;
        }
        case KITFOX_MCPAT_PIPELINE: {
            area = McPAT_Pipeline->area.get_area();
            break;
        }
        case KITFOX_MCPAT_SELECTION_LOGIC: {
            area = McPAT_selection_logic->area.get_area();
            break;
        }
        case KITFOX_MCPAT_UNDIFF_CORE: {
            area = McPAT_UndiffCore->area.get_area();
            break;
        }
        default: break;
    }
    
    return area*area_scaling*scaling*1e-12; // um^2 to m^2
}

int energylib_mcpat::get_mcpat_data_type(int ei_data_type) {
    int mcpat_data_type;
    switch(ei_data_type) {
        case KITFOX_DATA_TEMPERATURE: {
            mcpat_data_type = KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE;
            break;
        }
        case KITFOX_DATA_VOLTAGE: {
            mcpat_data_type = KITFOX_MCPAT_UPDATE_VAR_VOLTAGE;
            break;
        }
        case KITFOX_DATA_CLOCK_FREQUENCY: {
            mcpat_data_type = KITFOX_MCPAT_UPDATE_VAR_CLOCK_FREQUENCY;
            break;
        }
        default: {
            mcpat_data_type = KITFOX_MCPAT_UPDATE_VAR_UNKNOWN;
            break;
        }
    }
    return mcpat_data_type;
}

bool energylib_mcpat::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    int data_type = KITFOX_MCPAT_UPDATE_VAR_TYPES(Type);
    if(!IsLibraryVariable)
        data_type = get_mcpat_data_type(Type);
    
    switch(data_type) {
        case KITFOX_MCPAT_UPDATE_VAR_TEMPERATURE: {
            double temperature = *(double*)Value;
            // Leakage is defined for 1K temperature step in McPAT.
            input_p.temp = (unsigned int)temperature;
            // Limit the temperature range between 300~400K.
            if(input_p.temp < 300) { input_p.temp = 300; }
            else if(input_p.temp > 400) { input_p.temp = 400; }
            
            uca_org_t temp = init_interface(&input_p); // reset g_tp
            
            // Update leakage currents.
            // Leakage power scales linearly w.r.t to voltage because the leakage current values are hard-coded in technology.cc
            // Scale up the leakage currents so that leakage power follows the CV^2 rule.
            tech_p.peri_global.I_off_n = g_tp.peri_global.I_off_n*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_off_p = g_tp.peri_global.I_off_p*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_g_on_n = g_tp.peri_global.I_g_on_n*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_g_on_p = g_tp.peri_global.I_g_on_p*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.sram_cell.I_off_n = g_tp.sram_cell.I_off_n*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_off_p = g_tp.sram_cell.I_off_p*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_g_on_n = g_tp.sram_cell.I_g_on_n*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_g_on_p = g_tp.sram_cell.I_g_on_p*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.dram_wl.I_off_n = g_tp.dram_wl.I_off_n*(tech_p.dram_cell_Vdd-tech_p.dram_acc.Vth)/(g_tp.dram_cell_Vdd-g_tp.dram_acc.Vth);
            tech_p.dram_wl.I_off_p = g_tp.dram_wl.I_off_p*(tech_p.dram_cell_Vdd-tech_p.dram_acc.Vth)/(g_tp.dram_cell_Vdd-g_tp.dram_acc.Vth);
            tech_p.cam_cell.I_off_n = g_tp.cam_cell.I_off_n*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_off_p = g_tp.cam_cell.I_off_p*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_g_on_n = g_tp.cam_cell.I_g_on_n*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_g_on_p = g_tp.cam_cell.I_g_on_p*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            
            /*
            tech_p.peri_global.I_off_n = g_tp.peri_global.I_off_n;
            tech_p.peri_global.I_off_p = g_tp.peri_global.I_off_p;
            tech_p.peri_global.I_g_on_n = g_tp.peri_global.I_g_on_n;
            tech_p.peri_global.I_g_on_p = g_tp.peri_global.I_g_on_p;
            tech_p.sram_cell.I_off_n = g_tp.sram_cell.I_off_n;
            tech_p.sram_cell.I_off_p = g_tp.sram_cell.I_off_p;
            tech_p.sram_cell.I_g_on_n = g_tp.sram_cell.I_g_on_n;
            tech_p.sram_cell.I_g_on_p = g_tp.sram_cell.I_g_on_p;
            tech_p.dram_wl.I_off_n = g_tp.dram_wl.I_off_n;
            tech_p.dram_wl.I_off_p = g_tp.dram_wl.I_off_p;
            tech_p.cam_cell.I_off_n = g_tp.cam_cell.I_off_n;
            tech_p.cam_cell.I_off_p = g_tp.cam_cell.I_off_p;
            tech_p.cam_cell.I_g_on_n = g_tp.cam_cell.I_g_on_n;
            tech_p.cam_cell.I_g_on_p = g_tp.cam_cell.I_g_on_p;
            */
            
            updated = true;
            break;
        }
        case KITFOX_MCPAT_UPDATE_VAR_CLOCK_FREQUENCY: {
            clock_frequency = *(double*)Value;
            
            switch(energy_model) {
                case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
                    McPAT_dep_resource_conflict_check->coredynp.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_FLASH_CONTROLLER: {
                    McPAT_FlashController->fcp.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
                    McPAT_FunctionalUnit->clockRate = clock_frequency;
                    McPAT_FunctionalUnit->coredynp.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER: {
                    //clock_frequency *= 2.0; // DDR
                    McPAT_MCFrontEnd->mcp.clockRate = clock_frequency;
                    McPAT_MCBackend->mcp.clockRate = clock_frequency;
                    McPAT_MCPHY->mcp.clockRate = clock_frequency;
                    /*switch(energy_submodel) {
                        case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                            McPAT_MCFrontEnd->mcp.clockRate = clock_frequency;
                            break;
                        }
                        case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                            McPAT_MCBackend->mcp.clockRate = clock_frequency;
                            break;
                        }
                        case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                            McPAT_MCPHY->mcp.clockRate = clock_frequency;
                            break;
                        }
                        default: break;
                    }*/
                    break;
                }
                case KITFOX_MCPAT_NIU_CONTROLLER: {
                    McPAT_NIUController->niup.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_NOC: {
                    McPAT_NoC->nocdynp.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_PCIE_CONTROLLER: {
                    McPAT_PCIeController->pciep.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_PIPELINE: {
                    McPAT_Pipeline->coredynp.clockRate = clock_frequency;
                    break;
                }
                case KITFOX_MCPAT_UNDIFF_CORE: {
                    McPAT_UndiffCore->coredynp.clockRate = clock_frequency;
                    break;
                }
                default: break;
            }
            updated = true;
            break;
        }
        case KITFOX_MCPAT_UPDATE_VAR_VOLTAGE: {
            double voltage = *(double*)Value;
            
            // McPAT library should be using one of the following tech types
            tech_p.cam_cell.R_nch_on *= (voltage/tech_p.cam_cell.Vdd);
            tech_p.cam_cell.R_pch_on *= (voltage/tech_p.cam_cell.Vdd);
            tech_p.cam_cell.Vdd = voltage;
            tech_p.sram_cell.R_nch_on *= (voltage/tech_p.sram_cell.Vdd);
            tech_p.sram_cell.R_pch_on *= (voltage/tech_p.sram_cell.Vdd);
            tech_p.sram_cell.Vdd = voltage;
            tech_p.peri_global.R_nch_on *= (voltage/tech_p.peri_global.Vdd);
            tech_p.peri_global.R_pch_on *= (voltage/tech_p.peri_global.Vdd);
            tech_p.peri_global.Vdd = voltage;
            tech_p.dram_cell_Vdd = voltage;

            uca_org_t temp = init_interface(&input_p); // reset g_tp

            // Update leakage currents.
            // Leakage power scales linearly w.r.t to voltage because the leakage current values are hard-coded in technology.cc
            // Scale up the leakage currents so that leakage power follows the CV^2 rule.
            tech_p.peri_global.I_off_n = g_tp.peri_global.I_off_n*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_off_p = g_tp.peri_global.I_off_p*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_g_on_n = g_tp.peri_global.I_g_on_n*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.peri_global.I_g_on_p = g_tp.peri_global.I_g_on_p*(tech_p.peri_global.Vdd-tech_p.peri_global.Vth)/(g_tp.peri_global.Vdd-g_tp.peri_global.Vth);
            tech_p.sram_cell.I_off_n = g_tp.sram_cell.I_off_n*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_off_p = g_tp.sram_cell.I_off_p*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_g_on_n = g_tp.sram_cell.I_g_on_n*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.sram_cell.I_g_on_p = g_tp.sram_cell.I_g_on_p*(tech_p.sram_cell.Vdd-tech_p.sram_cell.Vth)/(g_tp.sram_cell.Vdd-g_tp.sram_cell.Vth);
            tech_p.dram_wl.I_off_n = g_tp.dram_wl.I_off_n*(tech_p.dram_cell_Vdd-tech_p.dram_acc.Vth)/(g_tp.dram_cell_Vdd-g_tp.dram_acc.Vth);
            tech_p.dram_wl.I_off_p = g_tp.dram_wl.I_off_p*(tech_p.dram_cell_Vdd-tech_p.dram_acc.Vth)/(g_tp.dram_cell_Vdd-g_tp.dram_acc.Vth);
            tech_p.cam_cell.I_off_n = g_tp.cam_cell.I_off_n*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_off_p = g_tp.cam_cell.I_off_p*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_g_on_n = g_tp.cam_cell.I_g_on_n*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);
            tech_p.cam_cell.I_g_on_p = g_tp.cam_cell.I_g_on_p*(tech_p.cam_cell.Vdd-tech_p.cam_cell.Vth)/(g_tp.cam_cell.Vdd-g_tp.cam_cell.Vth);

            updated = true;
            break;
        }
        case KITFOX_MCPAT_UPDATE_VAR_VPP: {
            double Vpp = *(double*)Value;
            
            tech_p.dram_wl.R_nch_on *= (Vpp/tech_p.vpp);
            tech_p.dram_wl.R_pch_on *= (Vpp/tech_p.vpp);
            tech_p.vpp = Vpp;
            updated = true;
            break;
        }
        default: {
            // Nothing to do
            return updated;
        }
    }
    
    switch(energy_model) {
        case KITFOX_MCPAT_ARRAYST: {
            McPAT_ArrayST->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_DEP_RESOURCE_CONFLICT_CHECK: {
            McPAT_dep_resource_conflict_check->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_FLASH_CONTROLLER: {
            McPAT_FlashController->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_FUNCTIONAL_UNIT: {
            McPAT_FunctionalUnit->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_INST_DECODER: {
            McPAT_inst_decoder->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_INTERCONNECT: {
            McPAT_interconnect->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_MEMORY_CONTROLLER: {
            McPAT_MCFrontEnd->update_energy(input_p,tech_p);
            McPAT_MCBackend->update_energy(input_p,tech_p);
            McPAT_MCPHY->update_energy(input_p,tech_p);
            /*switch(energy_submodel) {
                case KITFOX_MCPAT_MEMORY_CONTROLLER_FRONTEND: {
                    McPAT_MCFrontEnd->update_energy(input_p,tech_p);
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_BACKEND: {
                    McPAT_MCBackend->update_energy(input_p,tech_p);
                    break;
                }
                case KITFOX_MCPAT_MEMORY_CONTROLLER_PHY: {
                    McPAT_MCPHY->update_energy(input_p,tech_p);
                    break;
                }
                default: break;
            }*/
            break;
        }
        case KITFOX_MCPAT_NIU_CONTROLLER: {
            McPAT_NIUController->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_NOC: {
            McPAT_NoC->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_PCIE_CONTROLLER: {
            McPAT_PCIeController->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_PIPELINE: {
            McPAT_Pipeline->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_SELECTION_LOGIC: {
            McPAT_selection_logic->update_energy(input_p,tech_p);
            break;
        }
        case KITFOX_MCPAT_UNDIFF_CORE: {
            McPAT_UndiffCore->update_energy(input_p,tech_p);
            break;
        }
        default: break;
    }
    
    tech_p = g_tp;
    input_p = *g_ip;
    
    return updated;
}
