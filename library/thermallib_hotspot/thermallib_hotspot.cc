#include <iostream>
#include <cstdlib>
#include "kitfox.h"
#include "thermallib_hotspot.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_HOTSPOT;

thermallib_hotspot::thermallib_hotspot(pseudo_component_t *PseudoComponent) :
    thermal_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_HOTSPOT),
    model(NULL),
    flp(NULL),
    temperature(NULL),
    power(NULL)
{}

thermallib_hotspot::~thermallib_hotspot() {
    delete model;
    delete flp;
    delete temperature;
    delete power;
}

void thermallib_hotspot::initialize(void) {
    thermal_config = default_thermal_config();
    package_config = default_package_config();
  
    try {
        // Clock frequency is not necessary for HotSpot.
        thermal_config.base_proc_freq = 0.0;
        // Sampling interval is dynamically adjusted in thermallib_hotspot::compute() function.
        thermal_config.sampling_intvl = 0.0;
    
        const char *thermal_analysis_str = pseudo_component->lookup_in_library("thermal_analysis");
        if(!strcasecmp(thermal_analysis_str,"transient"))
            thermal_analysis = KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT;
        else if(!strcasecmp(thermal_analysis_str,"steady")||!strcasecmp(thermal_analysis_str,"steady_state"))
            thermal_analysis = KITFOX_HOTSPOT_THERMAL_ANALYSIS_STEADY_STATE;
        else {
            pseudo_component->kitfox->error("hotspot","%s.library.thermal_analysis = %s is unknown",pseudo_component->name.c_str(),thermal_analysis_str);
        }
    
        thermal_config.grid_rows = pseudo_component->lookup_in_library("grid_rows");
        thermal_config.grid_cols = pseudo_component->lookup_in_library("grid_cols");
        thermal_config.ambient = pseudo_component->lookup_in_library("ambient_temperature",true);
        thermal_config.init_temp = pseudo_component->lookup_in_library("temperature",true);
    
        if(pseudo_component->exists_in_library("thermal_threshold"))
            thermal_config.thermal_threshold = pseudo_component->lookup_in_library("thermal_threshold");
        else
            thermal_config.thermal_threshold = 273.15+81.8;
        if(pseudo_component->exists_in_library("chip_thickness"))
            thermal_config.t_chip = pseudo_component->lookup_in_library("chip_thickness");
        else
            thermal_config.t_chip = 0.15e-3;
        if(pseudo_component->exists_in_library("chip_thermal_conductivity"))
            thermal_config.k_chip = pseudo_component->lookup_in_library("chip_thermal_conductivity");
        else
            thermal_config.k_chip = 100.0;
        if(pseudo_component->exists_in_library("chip_heat"))
            thermal_config.p_chip = pseudo_component->lookup_in_library("chip_heat");
        else
            thermal_config.p_chip = 1.75e6;
        if(pseudo_component->exists_in_library("heatsink_convection_capacitance"))
            thermal_config.c_convec = pseudo_component->lookup_in_library("heatsink_convection_capacitance");
        else
            thermal_config.c_convec = 140.4;
        if(pseudo_component->exists_in_library("heatsink_convection_resistance"))
            thermal_config.r_convec = pseudo_component->lookup_in_library("heatsink_convection_resistance");
        else
            thermal_config.r_convec = 0.1;
        if(pseudo_component->exists_in_library("heatsink_side"))
            thermal_config.s_sink = pseudo_component->lookup_in_library("heatsink_side");
        else
            thermal_config.s_sink = 60e-3;
        if(pseudo_component->exists_in_library("heatsink_thickness"))
            thermal_config.t_sink = pseudo_component->lookup_in_library("heatsink_thickness");
        else
            thermal_config.t_sink = 6.9e-3;
        if(pseudo_component->exists_in_library("heatsink_thermal_conductivity"))
            thermal_config.k_sink = pseudo_component->lookup_in_library("heatsink_thermal_conductivity");
        else
            thermal_config.k_sink = 400.0;
        if(pseudo_component->exists_in_library("heatsink_heat"))
            thermal_config.p_sink = pseudo_component->lookup_in_library("heatsink_heat");
        else
            thermal_config.p_sink = 3.55e6;
        if(pseudo_component->exists_in_library("spreader_side"))
            thermal_config.s_spreader = pseudo_component->lookup_in_library("spreader_side");
        else
            thermal_config.s_spreader = 30e-3;
        if(pseudo_component->exists_in_library("spreader_thickness"))
            thermal_config.t_spreader = pseudo_component->lookup_in_library("spreader_thickness");
        else
            thermal_config.t_spreader = 1e-3;
        if(pseudo_component->exists_in_library("spreader_thermal_conductivity"))
            thermal_config.k_spreader = pseudo_component->lookup_in_library("spreader_thermal_conductivity");
        else
            thermal_config.k_spreader = 400.0;
        if(pseudo_component->exists_in_library("spreader_heat"))
            thermal_config.p_spreader = pseudo_component->lookup_in_library("spreader_heat");
        else
            thermal_config.p_spreader = 3.55e6;
        if(pseudo_component->exists_in_library("interface_thickness"))
            thermal_config.t_interface = pseudo_component->lookup_in_library("interface_thickness");
        else
            thermal_config.t_interface = 20e-6;
        if(pseudo_component->exists_in_library("interface_thermal_conductivity"))
            thermal_config.k_interface = pseudo_component->lookup_in_library("interface_thermal_conductivity");
        else
            thermal_config.k_interface = 4.0;
        if(pseudo_component->exists_in_library("interface_heat"))
            thermal_config.p_interface = pseudo_component->lookup_in_library("interface_heat");
        else
            thermal_config.p_interface = 4.0e6;
        if(pseudo_component->exists_in_library("metal_layers"))
            thermal_config.n_metal = pseudo_component->lookup_in_library("metal_layers");
        else
            thermal_config.n_metal = 8;
        if(pseudo_component->exists_in_library("metal_layer_thickness"))
            thermal_config.t_metal = pseudo_component->lookup_in_library("metal_layer_thickness");
        else
            thermal_config.t_metal = 10e-6;
        if(pseudo_component->exists_in_library("c4_thickness"))
            thermal_config.t_c4 = pseudo_component->lookup_in_library("c4_thickness");
        else
            thermal_config.t_c4 = 0.0001;
        if(pseudo_component->exists_in_library("c4_side"))
            thermal_config.s_c4 = pseudo_component->lookup_in_library("c4_side");
        else
            thermal_config.s_c4 = 20e-6;
        if(pseudo_component->exists_in_library("c4_pads"))
            thermal_config.n_c4 = pseudo_component->lookup_in_library("c4_pads");
        else
            thermal_config.n_c4 = 400;
        if(pseudo_component->exists_in_library("substrate_side"))
            thermal_config.s_sub = pseudo_component->lookup_in_library("substrate_side");
        else
            thermal_config.s_sub = 0.021;
        if(pseudo_component->exists_in_library("substrate_thickness"))
            thermal_config.t_sub = pseudo_component->lookup_in_library("substrate_thickness");
        else
            thermal_config.t_sub = 0.001;
        if(pseudo_component->exists_in_library("solder_side"))
            thermal_config.s_solder = pseudo_component->lookup_in_library("solder_side");
        else
            thermal_config.s_solder = 0.021;
        if(pseudo_component->exists_in_library("solder_thickness"))
            thermal_config.t_solder = pseudo_component->lookup_in_library("solder_thickness");
        else
            thermal_config.t_solder = 0.00094;
        if(pseudo_component->exists_in_library("pcb_side"))
            thermal_config.s_pcb = pseudo_component->lookup_in_library("pcb_side");
        else
            thermal_config.s_pcb = 0.1;
        if(pseudo_component->exists_in_library("pcb_thickness"))
            thermal_config.t_pcb = pseudo_component->lookup_in_library("pcb_thickness");
        else
            thermal_config.t_pcb = 0.002;
        if(pseudo_component->exists_in_library("secondary_convection_resistance"))
            thermal_config.r_convec_sec = pseudo_component->lookup_in_library("secondary_convection_resistance");
        else
            thermal_config.r_convec_sec = 50.0;
        if(pseudo_component->exists_in_library("secondary_convection_capacitance"))
            thermal_config.c_convec_sec = pseudo_component->lookup_in_library("secondary_convection_capacitance");
        else
            thermal_config.c_convec_sec = 40.0;
        // How are the modes indexed?
        if(pseudo_component->exists_in_library("leakage_mode"))
            thermal_config.leakage_mode = pseudo_component->lookup_in_library("leakage_mode");
        else
            thermal_config.leakage_mode = 0;
        if(pseudo_component->exists_in_library("secondary_model"))
            thermal_config.model_secondary = (bool)pseudo_component->lookup_in_library("secondary_model");
        else
            thermal_config.model_secondary = false;
        if(pseudo_component->exists_in_library("dtm_used"))
            thermal_config.dtm_used = (bool)pseudo_component->lookup_in_library("dtm_used");
        else
            thermal_config.dtm_used = false;
        if(pseudo_component->exists_in_library("block_omit_lateral"))
            thermal_config.block_omit_lateral = (bool)pseudo_component->lookup_in_library("block_omit_lateral");
        else
            thermal_config.block_omit_lateral = false;
        if(pseudo_component->exists_in_library("leakage_used"))
            thermal_config.leakage_used = (bool)pseudo_component->lookup_in_library("leakage_used");
        else
            thermal_config.leakage_used = false;
        if(pseudo_component->exists_in_library("package_model_used"))
            thermal_config.package_model_used = (bool)pseudo_component->lookup_in_library("package_model_used");
        else
            thermal_config.package_model_used = false;
        
        // HotSpot error checks (copied from HotSpot code)
        if((thermal_config.t_chip <= 0)||(thermal_config.s_sink <= 0)||
           (thermal_config.t_sink <= 0)||(thermal_config.s_spreader <= 0)||
           (thermal_config.t_spreader <= 0)||(thermal_config.t_interface <= 0)) {
            pseudo_component->kitfox->error("hotspot","%s.library should have chip and packpage dimension greater than zero",pseudo_component->name.c_str());
        }
        if((thermal_config.t_metal <= 0)||(thermal_config.n_metal <= 0)||
           (thermal_config.t_c4 <= 0)||(thermal_config.s_c4 <= 0)||
           (thermal_config.n_c4 <= 0)||(thermal_config.s_sub <= 0)||
           (thermal_config.t_sub <= 0)||(thermal_config.s_solder <= 0)||
           (thermal_config.t_solder <= 0)||(thermal_config.s_pcb <= 0)||
           (thermal_config.t_solder <= 0)||(thermal_config.r_convec_sec <= 0)||
           (thermal_config.c_convec_sec <= 0)) {
            pseudo_component->kitfox->error("hotspot","%s.library should have secondary heat transfer layer dimension should be greater than zero",pseudo_component->name.c_str());
        }
        if(thermal_config.leakage_used == 1) {
            pseudo_component->kitfox->error("hotspot","%s.library does not support transient leakage iteration in this release -- all transient results are without thermal-leakage loop",pseudo_component->name.c_str());
        }
        if((thermal_config.model_secondary == 1)&&(!strcasecmp(thermal_config.model_type, BLOCK_MODEL_STR))) {
            pseudo_component->kitfox->error("hotspot","%s.library supports secondary heat transfer path only in the grid mode",pseudo_component->name.c_str());
        }
        if((thermal_config.thermal_threshold < 0)||(thermal_config.c_convec < 0)||
           (thermal_config.r_convec < 0)|| (thermal_config.ambient < 0)) {
             //||(thermal_config.base_proc_freq < 0)||(thermal_config.sampling_intvl < 0)) 
            pseudo_component->kitfox->error("hotspot","%s.library has invalid thermal simulation parameters",pseudo_component->name.c_str());
        }
        if((thermal_config.grid_rows <= 0)||(thermal_config.grid_cols <= 0)||
           (thermal_config.grid_rows & (thermal_config.grid_rows-1))||
           (thermal_config.grid_cols & (thermal_config.grid_cols-1))) {
            pseudo_component->kitfox->error("hotspot","%s.library must have grid rows and columns as the power of two",pseudo_component->name.c_str());
        }
        if(thermal_config.package_model_used) {
            if(pseudo_component->exists_in_library("natural_convection"))
                package_config.natural_convec = pseudo_component->lookup_in_library("natural_convection");
            else
                package_config.natural_convec = false;
            if(pseudo_component->exists_in_library("flow_type")) {
                const char *flow_type_str = pseudo_component->lookup_in_library("flow_type");
                if(!strcasecmp(flow_type_str,"side"))
                    package_config.flow_type = 0;
                else if(!strcasecmp(flow_type_str,"top"))
                    package_config.flow_type = 1;
                else {
                    pseudo_component->kitfox->error("hotspot","%s.library.flow_type = %s is unknown",pseudo_component->name.c_str(),flow_type_str);
                }
            }
            else
                package_config.flow_type = 0;
            if(pseudo_component->exists_in_library("sink_type")) {
                const char *sink_type_str = pseudo_component->lookup_in_library("sink_type");
                if(!strcasecmp(sink_type_str,"fin_channel"))
                    package_config.sink_type = 0;
                else if(!strcasecmp(sink_type_str,"pinfin"))
                    package_config.sink_type = 1;
                else {
                    pseudo_component->kitfox->error("hotspot","%s.library.sink_type = %s is unknown",pseudo_component->name.c_str(),sink_type_str);
                }
            }
            else
                package_config.sink_type = 0;
            if(pseudo_component->exists_in_library("fin_height"))
                package_config.fin_height = pseudo_component->lookup_in_library("fin_height");
            else
                package_config.fin_height = 0.03;
            if(pseudo_component->exists_in_library("fin_width"))
                package_config.fin_width = pseudo_component->lookup_in_library("fin_width");
            else
                package_config.fin_width = 0.001;
            if(pseudo_component->exists_in_library("channel_width"))
                package_config.channel_width = pseudo_component->lookup_in_library("channel_width");
            else
                package_config.channel_width = 0.002;
            if(pseudo_component->exists_in_library("pin_height"))
                package_config.pin_height = pseudo_component->lookup_in_library("pin_height");
            else
                package_config.pin_height = 0.02;
            if(pseudo_component->exists_in_library("pin_diameter"))
                package_config.pin_diam = pseudo_component->lookup_in_library("pin_diameter");
            else
                package_config.pin_diam = 0.002;
            if(pseudo_component->exists_in_library("pin_distance"))
                package_config.pin_dist = pseudo_component->lookup_in_library("pin_distance");
            else
                package_config.pin_dist = 0.005;
            if(pseudo_component->exists_in_library("fan_radius"))
                package_config.fan_radius = pseudo_component->lookup_in_library("fan_radius");
            else
                package_config.fan_radius = 0.03;
            if(pseudo_component->exists_in_library("motor_radius"))
                package_config.motor_radius = pseudo_component->lookup_in_library("motor_radius");
            else
                package_config.motor_radius = 0.01;
            if(pseudo_component->exists_in_library("rpm"))
                package_config.rpm = pseudo_component->lookup_in_library("rpm");
            else
                package_config.rpm = 1000;
            
            if(!package_config.natural_convec)
                calculate_flow(&convection,&package_config,&thermal_config);
            else
                calc_natural_convec(&convection,&package_config,&thermal_config,
                                    thermal_config.ambient+SMALL_FOR_CONVEC);

            thermal_config.r_convec = convection.r_th;
            
            if(thermal_config.r_convec<R_CONVEC_LOW||thermal_config.r_convec>R_CONVEC_HIGH) {
                pseudo_component->kitfox->error("hotspot","%s.library has unrealistic heatsink convection resistance -- double-check package settings",pseudo_component->name.c_str());
            }
        }

        const char *model_type_str = pseudo_component->lookup_in_library("model_type");
        if(!strcasecmp(model_type_str,"block"))
            strcpy(thermal_config.model_type,"block");
        else if(!strcasecmp(model_type_str,"grid"))
            strcpy(thermal_config.model_type,"grid");
        else {
            pseudo_component->kitfox->error("hotspot","%s.library.model_type = %s is unknown",pseudo_component->name.c_str(),model_type_str);
        }
        
        if(!strcasecmp(thermal_config.model_type,"grid")) {
            const char *grid_map_mode_str = pseudo_component->lookup_in_library("grid_map_mode");
            if(!strcasecmp(grid_map_mode_str,"center"))
                strcpy(thermal_config.grid_map_mode,"center");
            else if(!strcasecmp(grid_map_mode_str,"avg")||!strcasecmp(grid_map_mode_str,"average"))
                strcpy(thermal_config.grid_map_mode,"avg");
            else if(!strcasecmp(grid_map_mode_str,"min")||!strcasecmp(grid_map_mode_str,"minimum"))
                strcpy(thermal_config.grid_map_mode,"min");
            else if(!strcasecmp(grid_map_mode_str,"max")||!strcasecmp(grid_map_mode_str,"maximum"))
                strcpy(thermal_config.grid_map_mode,"max");
            else {
                pseudo_component->kitfox->error("hotspot","%s.library.grid_map_mode = %s is unknown",pseudo_component->name.c_str(),grid_map_mode_str);
            }
        }
        
        flp = flp_alloc_init_mem(get_floorplan_counts());
        
        for(unsigned i = 0; i < get_floorplan_counts(); i++) {
            Comp_ID floorplan_id = get_floorplan_id(i); // 'i'th floorplan's ID
            const char *floorplan_name = get_floorplan_name(floorplan_id);

            dimension_t floorplan_dimension;
            if(pseudo_component->kitfox->pull_data(floorplan_id,0.0,UNSPECIFIED_TIME,KITFOX_DATA_DIMENSION,&floorplan_dimension) != KITFOX_QUEUE_ERROR_NONE) {
                pseudo_component->kitfox->error("hotspot","%s.library.floorplan = %s is invalid",pseudo_component->name.c_str(),floorplan_name);
            }
            flp->units[i].width = floorplan_dimension.width;
            flp->units[i].height = floorplan_dimension.height;
            flp->units[i].leftx = floorplan_dimension.left;
            flp->units[i].bottomy = floorplan_dimension.bottom;
            flp->units[i].layer = floorplan_dimension.die_index;
            strcpy(flp->units[i].name,floorplan_name);
        }
        
        // Connectivity between floorplans
        for(unsigned i = 0; i < flp->n_units; i++) {
            for(unsigned j = 0; j < flp->n_units; j++) {
                flp->wire_density[i][j] = 1.0;
            }
        }
        
        flp_translate(flp,0,0);
        
        //model = alloc_RC_model(&thermal_config,flp); // Manually done below
        
        model = (RC_model_t*)calloc(1,sizeof(RC_model_t));
        
        // Grid model supports manually configuring layer stacks (possibly 3D)
        if(!strcasecmp(thermal_config.model_type,"grid")) {
            // The following is the part of alloc_grid_model()
            grid_model_t *grid_model = (grid_model_t*)calloc(1,sizeof(grid_model_t));

            grid_model->config = thermal_config;
            grid_model->rows = thermal_config.grid_rows;
            grid_model->cols = thermal_config.grid_cols;
            
            if(!strcasecmp(thermal_config.grid_map_mode,"avg"))
                grid_model->map_mode = GRID_AVG;
            else if(!strcasecmp(thermal_config.grid_map_mode,"min"))
                grid_model->map_mode = GRID_MIN;
            else if(!strcasecmp(thermal_config.grid_map_mode,"max"))
                grid_model->map_mode = GRID_MAX;
            else
                grid_model->map_mode = GRID_CENTER;
            
            // Count the number of layers
            unsigned num_layers = 0;
            if(pseudo_component->exists_in_library("layer"))
                num_layers = pseudo_component->lookup_in_library("layer").getLength();
                 
            // Create HotSpot layers
            if(num_layers > 0) {
                if(num_layers < DEFAULT_CHIP_LAYERS) {
                    pseudo_component->kitfox->error("hotspot","%s.library has fewer layers than the default",pseudo_component->name.c_str());
                }
                grid_model->n_layers = num_layers;
            }
            else
                grid_model->n_layers = DEFAULT_CHIP_LAYERS;
            
            if(grid_model->config.model_secondary) {
                grid_model->n_layers += SEC_CHIP_LAYERS;
                grid_model->layers = (layer_t*)calloc(grid_model->n_layers+DEFAULT_PACK_LAYERS+SEC_PACK_LAYERS,sizeof(layer_t));
            }
            else
                grid_model->layers = (layer_t*)calloc(grid_model->n_layers+DEFAULT_PACK_LAYERS,sizeof(layer_t));

            // Initialize layers from the top to bottom.
            for(unsigned l = num_layers; l > 0; l--) {
                Setting &layer_setting = pseudo_component->lookup_in_library("layer")[l-1];
                int layer_index = num_layers-l;//layer_setting["index"];
                grid_model->layers[layer_index].no = layer_index;
                grid_model->layers[layer_index].has_lateral = (bool)layer_setting["lateral_heat_flow"];
                grid_model->layers[layer_index].has_power = (bool)layer_setting["is_source_layer"];
                grid_model->layers[layer_index].sp = layer_setting["heat"];
                grid_model->layers[layer_index].k = layer_setting["resistance"];
                grid_model->layers[layer_index].k = 1.0/grid_model->layers[layer_index].k;
                grid_model->layers[layer_index].thickness = layer_setting["thickness"];
                
                if(grid_model->layers[layer_index].has_power) { // Source layer
                    vector<int> grid_flp_index; // Use flp index of default flps created above
                    for(unsigned i = 0; i < get_floorplan_counts(); i++) {
                        if(flp->units[i].layer == layer_index) {
                            grid_flp_index.push_back(get_blk_index(flp,flp->units[i].name));
                        }
                    }
                    
                    if(grid_flp_index.size() == 0) {
                        pseudo_component->kitfox->error("hotspot","%s.layer[%u] is not floorplanned",pseudo_component->name.c_str(),layer_index);
                    }
                    grid_model->layers[layer_index].flp = flp_alloc_init_mem(grid_flp_index.size());
                    
                    // Copy default flp information to layer flps.
                    for(unsigned flp_index = 0; flp_index < grid_flp_index.size(); flp_index++) {
                        grid_model->layers[layer_index].flp->units[flp_index] = flp->units[grid_flp_index[flp_index]];
                    }
                    
                    // TODO: wire density
                    for(unsigned i = 0; i < grid_model->layers[layer_index].flp->n_units; i++) {
                        for(unsigned j = 0; j < grid_model->layers[layer_index].flp->n_units; j++) {
                            grid_model->layers[layer_index].flp->wire_density[i][j] = 1.0;
                        }
                    }
                    
                    flp_translate(grid_model->layers[layer_index].flp,0,0);
                    grid_flp_index.clear();
                }
                else { // Non-source layer
                    // HotSpot needs non-source layers also floorplanned.
                    // Temporarily mark layer # as referred layer index to copy flps layers.
                    grid_model->layers[layer_index].no = num_layers-(layer_setting.getParent()[(const char*)layer_setting["use_floorplan_layer"]]).getIndex()-1;;
                }
            }
            
            if(num_layers > 0) {
                for(unsigned l = 0; l < num_layers; l++) {
                    if(!grid_model->layers[l].has_power) {// Non-source layer
                        // Referred source layer index
                        unsigned ref_source_layer_index = grid_model->layers[l].no;
                        
                        // If flp is already created for non-source layer,
                        // or source layer doesn't have flps, throw an error.
                        if(grid_model->layers[l].flp||!grid_model->layers[ref_source_layer_index].flp) {
                            pseudo_component->kitfox->error("hotspot","%s can't create floorplan on layer[%u]",pseudo_component->name.c_str(),l);
                        }
                        
                        grid_model->layers[l].no = l;
                        grid_model->layers[l].flp = grid_model->layers[ref_source_layer_index].flp;
                    }
                }
                
                for(unsigned l = 0; l < grid_model->n_layers; l++) {
                    if(l == 0) {
                        grid_model->width = get_total_width(grid_model->layers[l].flp);
                        grid_model->height = get_total_height(grid_model->layers[l].flp);
                    }
                    else {
                        if(grid_model->width != get_total_width(grid_model->layers[l].flp)) {
                            pseudo_component->kitfox->error("hotspot","%s has different width across package layers",pseudo_component->name.c_str());
                        }
                        if(grid_model->height != get_total_height(grid_model->layers[l].flp)) {
                            pseudo_component->kitfox->error("hotspot","%s has different height across package layers",pseudo_component->name.c_str());
                        }
                    }
                    
                    grid_model->has_lcf = true; // to indicate grid layers are manually configured
                    grid_model->layers[l].b2gmap = new_b2gmap(grid_model->rows, grid_model->cols);
                    grid_model->layers[l].g2bmap = (glist_t *) calloc(grid_model->layers[l].flp->n_units,sizeof(glist_t));
                }
            }
            else {
                grid_model->has_lcf = false;
                grid_model->base_n_units = flp->n_units;
                populate_default_layers(grid_model,flp);
            }
            
            append_package_layers(grid_model);
            
            for(unsigned l = 0; l < grid_model->n_layers; l++) {
                if(grid_model->layers[l].flp) {
                    grid_model->total_n_blocks += grid_model->layers[l].flp->n_units;
                }
            }
            
            grid_model->last_steady = new_grid_model_vector(grid_model);
            grid_model->last_trans = new_grid_model_vector(grid_model);
            
            model->type = GRID_MODEL;
            model->grid = grid_model;
            model->config = &model->grid->config;
        }
        else { // BLOCK_MODEL
            model->type = BLOCK_MODEL;
            model->block = alloc_block_model(&thermal_config,flp);
            model->config = &model->block->config;
        }
        
        populate_R_model(model,flp);
        
        if(thermal_analysis == KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT)
            populate_C_model(model,flp);
        
        temperature = hotspot_vector(model);
        power = hotspot_vector(model);
        
        // set initial temperature
        if(model->type == GRID_MODEL) {
            int base = 0;
            for(unsigned l = 0; l < model->grid->n_layers; l++) {
                for(unsigned i = 0; i < flp->n_units; i++) {
                    temperature[base+i] = thermal_config.init_temp;
                }
                
                if(model->grid->layers[l].flp) {
                    base += model->grid->layers[l].flp->n_units;
                }
            }
            if(base != model->grid->total_n_blocks) {
                pseudo_component->kitfox->error("hotspot","%s.library failed to tally total_n_blocks",pseudo_component->name.c_str());
            }
            
            for(unsigned i = 0; i < (model->config->model_secondary?(EXTRA_SEC):(EXTRA)); i++) {
                temperature[base+i] = thermal_config.init_temp;
            }
        }
        else { // BLOCK_MODEL
            for(unsigned l = 0; l < NL; l++) {
                for(unsigned i = 0; i < flp->n_units; i++) {
                    temperature[i+l*flp->n_units] = thermal_config.init_temp;
                }
            }
            
            for(unsigned i = 0; i < EXTRA; i++) {
                temperature[i+NL*flp->n_units] = thermal_config.init_temp;
            }
        }
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}

string thermallib_hotspot::get_flp_name(Comp_ID ComponentID) {
    const char *flp_name = get_floorplan_name(ComponentID);
    if(flp_name == NULL) {
        pseudo_component->kitfox->error("hotspot","pseudo component [ID=%lu] is not a floorplan of %s",ComponentID,pseudo_component->name.c_str());
    }
    return string(flp_name);
}

void thermallib_hotspot::calculate_temperature(Second Time, Second Period) {
    model->config->sampling_intvl = Period;
  
    if(thermal_analysis == KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT) // Transient
        compute_temp(model,power,temperature,model->config->sampling_intvl);
    else // Steady state
        steady_state_temp(model,power,temperature);
}

grid_t<Kelvin> thermallib_hotspot::get_thermal_grid(void) {
    grid_t<Kelvin> thermal_grid;
  
    if(model->type == GRID_MODEL) {
        thermal_grid.reserve(thermal_config.grid_cols,thermal_config.grid_rows,model->grid->n_layers);
    
        for(unsigned l = 0; l < model->grid->n_layers; l++) {
            for(unsigned r = 0; r < model->grid->config.grid_rows; r++) {
                for(unsigned c = 0; c < model->grid->config.grid_cols; c++) {
                    // HotSpot indexes row from bottom to top,
                    // which results in the bottom line to be printed at the top
                    // when dumping out the grid.
                    if(thermal_analysis == KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT)
                        thermal_grid(c,model->grid->config.grid_rows-r-1,l) = model->grid->last_trans->cuboid[l][c][r];
                    else
                        thermal_grid(c,model->grid->config.grid_rows-r-1,l) = model->grid->last_steady->cuboid[l][c][r];
                }
            }
        }
    }
    else {
        thermal_grid.reserve(get_total_width(flp)/thermal_config.grid_cols,get_total_height(flp)/thermal_config.grid_rows,1);

        double cell_width = get_total_width(flp)/thermal_grid.cols();
        double cell_height = get_total_height(flp)/thermal_grid.rows();
    
        for(unsigned i = 0; i < flp->n_units; i++) {
            unsigned bottom = tolerant_floor((flp->units[i].bottomy)/cell_height);
            unsigned top = tolerant_ceil((flp->units[i].bottomy+flp->units[i].height)/cell_height);
            unsigned left = tolerant_floor((flp->units[i].leftx)/cell_width);
            unsigned right = tolerant_ceil((flp->units[i].leftx+flp->units[i].width)/cell_width);
      
            for(unsigned r = bottom; r <= top; r++) {
                for(unsigned c = left; c <= right; c++) {
                    // HotSpot indexes row from bottom to top,
                    // which results in the bottom line to be printed at the top
                    // when dumping out the grid.
                    thermal_grid(c,thermal_config.grid_rows-r-1,0) = temperature[i];
                }
            }
        }
    }
    return thermal_grid;
}

Kelvin thermallib_hotspot::get_floorplan_temperature(Comp_ID ComponentID, int Type) {
    Kelvin floorplan_temperature = 0.0;

    if(Type != KITFOX_TEMPERATURE_MAPPING_UNKNOWN) {
        if(((Type == KITFOX_TEMPERATURE_MAPPING_CENTER)&&strcasecmp(thermal_config.grid_map_mode,"center"))||
           ((Type == KITFOX_TEMPERATURE_MAPPING_AVERAGE)&&strcasecmp(thermal_config.grid_map_mode,"avg"))||
           ((Type == KITFOX_TEMPERATURE_MAPPING_MAX)&&strcasecmp(thermal_config.grid_map_mode,"max"))||
           ((Type == KITFOX_TEMPERATURE_MAPPING_MIN)&&strcasecmp(thermal_config.grid_map_mode,"min"))) {
            pseudo_component->kitfox->warning("hotspot","%s uses only temperature setting defined in input config",pseudo_component->name.c_str());
        }
    }

    if(model->type == GRID_MODEL) { // Grid model
        unsigned base = 0;
        for(unsigned l = 0; l < model->grid->n_layers; l++) {
            if(model->grid->layers[l].has_power) { // Source layer
                unsigned flp_index = get_blk_index(model->grid->layers[l].flp,(char*)get_flp_name(ComponentID).c_str());
                if(flp_index != -1) {
                    floorplan_temperature = temperature[base+flp_index];
                    break;
                }
            }
            base += model->grid->layers[l].flp->n_units;
        }
    }
    else { // Block model
        unsigned flp_index = get_blk_index(flp,(char*)get_flp_name(ComponentID).c_str());
        if(flp_index != -1) {
            floorplan_temperature = temperature[flp_index];
        }
    }
  
    if(floorplan_temperature == 0.0) {
        pseudo_component->kitfox->error("hotspot","%s doesn't have valid temperature for floorplan [Comp_ID=%lu]",pseudo_component->name.c_str(),ComponentID);
    }
  
    return floorplan_temperature;
}

Kelvin thermallib_hotspot::get_point_temperature(Meter x, Meter y, Index layer) {
    Kelvin point_temperature = 0.0;
  
    if(model->type == GRID_MODEL) {// Grid model
        // HotSpot indexes row from bottom to top,
        // which results in the bottom line to be printed at the top
        // when dumping out the grid.
        unsigned row_index = model->grid->rows-1-(unsigned)(y/(model->grid->height/model->grid->rows));
        unsigned col_index = (unsigned)(x/(model->grid->width/model->grid->cols));
    
        if(thermal_analysis == KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT) // Transient
            point_temperature = model->grid->last_trans->cuboid[layer][col_index][row_index];
        else // Steady state
            point_temperature = model->grid->last_steady->cuboid[layer][col_index][row_index];
    }
    else { // Block model
        if(layer == 0) { // 2D
            for(unsigned i = 0; i < flp->n_units; i++) {
                if((x >= flp->units[i].leftx)&&(x <= (flp->units[i].leftx+flp->units[i].width))) {
                    if((y >= flp->units[i].bottomy)&&(y <= (flp->units[i].bottomy+flp->units[i].height))) {
                        point_temperature = temperature[i];
                        break;
                    }
                }
            }
        }
    }
  
    if(point_temperature == 0.0) {
        pseudo_component->kitfox->error("hotspot","%s doesn't have valid point temperature at the location[X=%lf,Y=%lf,Layer=%u]",pseudo_component->name.c_str(),x,y,layer);
    }
    return point_temperature;
}

void thermallib_hotspot::map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower) {
    bool mapped = false;
  
    if(model->type == GRID_MODEL) { // Grid model
        unsigned base = 0;
        for(unsigned l = 0; l < model->grid->n_layers; l++) {
            if(model->grid->layers[l].has_power) { // Source layer
                unsigned flp_index = get_blk_index(model->grid->layers[l].flp,(char*)get_flp_name(ComponentID).c_str());
                if(flp_index != -1) {
                    power[base+flp_index] = PartitionPower.get_total();
                    mapped = true;
                    break;
                }
            }
            base += model->grid->layers[l].flp->n_units;
        }
    }
    else { // Block model
        unsigned flp_index = get_blk_index(flp,(char*)get_flp_name(ComponentID).c_str());
        if(flp_index != -1) {
            power[flp_index] = PartitionPower.get_total();
            mapped = true;
        }
    }
  
    if(!mapped) {
        pseudo_component->kitfox->error("hotspot","%s doesn't have floorplan [Comp_ID=%lu]",pseudo_component->name.c_str(),ComponentID);
    }
}

bool thermallib_hotspot::update_library_variable(int Type, void *Value, bool IsLibraryVariable) {
    bool updated = false;
    if(IsLibraryVariable) {
        pseudo_component->kitfox->error("hotspot","%s doesn't have library-specific variable",pseudo_component->name.c_str());
    }
    
    switch(Type) {
        case KITFOX_DATA_CLOCK_FREQUENCY: {
            thermal_config.base_proc_freq = *(Hertz*)Value;
            updated = true;
            break;
        }
        case KITFOX_DATA_VOLTAGE:
        case KITFOX_DATA_THRESHOLD_VOLTAGE:
        case KITFOX_DATA_TEMPERATURE:
        case KITFOX_DATA_TDP:
        case KITFOX_DATA_POWER: {
        	break;
        }
        default: {
            // Nothing to do
            break;
        }
    }
    return updated;
}

