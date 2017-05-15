#include "kitfox.h"
#include "thermallib_3dice.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;
using namespace libKitFox_3DICE;

thermallib_3dice::thermallib_3dice(pseudo_component_t *PseudoComponent) :
    thermal_library_t(PseudoComponent,KITFOX_LIBRARY_MODEL_3DICE)
{}

thermallib_3dice::~thermallib_3dice() {}

void thermallib_3dice::initialize() {
    init_stack_description(&stkd);
    init_analysis(&analysis);

    try {
        if(pseudo_component->exists_in_library("grid_map_mode")) {
            const char *grid_map_mode_str = pseudo_component->lookup_in_library("grid_map_mode");
            if(!strcasecmp(grid_map_mode_str,"avg")||!strcasecmp(grid_map_mode_str,"average"))
                thermal_grid_mapping = KITFOX_TEMPERATURE_MAPPING_AVERAGE;
            else if(!strcasecmp(grid_map_mode_str,"center"))
                thermal_grid_mapping = KITFOX_TEMPERATURE_MAPPING_CENTER;
            else if(!strcasecmp(grid_map_mode_str,"max")||!strcasecmp(grid_map_mode_str,"maximum"))
                thermal_grid_mapping = KITFOX_TEMPERATURE_MAPPING_MAX;
            else if(!strcasecmp(grid_map_mode_str,"min")||!strcasecmp(grid_map_mode_str,"minimum"))
                thermal_grid_mapping = KITFOX_TEMPERATURE_MAPPING_MIN;
            else {
                pseudo_component->kitfox->error("3d-ice","%s.library.grid_map_mode = %s is unknown",pseudo_component->name.c_str(),grid_map_mode_str);
            }
        }
        else 
            thermal_grid_mapping = KITFOX_TEMPERATURE_MAPPING_AVERAGE;

        // Stack description - Dimensions
        stkd.Dimensions = alloc_and_init_dimensions();

        double ambient_temperature = pseudo_component->lookup_in_library("ambient_temperature",true);

        double chip_width = pseudo_component->lookup_in_library("chip_width");
        double chip_height = pseudo_component->lookup_in_library("chip_height");


        if((chip_width > 1.0)||(chip_height > 1.0)) {
            pseudo_component->kitfox->error("3d-ice","%s.library.chip_width or %s.library.chip_height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
          }

        // In 3D-ICE, Length is x dimension and Width is y dimension.
        // Whereas, HotSpot uses Width as x dimension and Height as y dimension.
        // KitFox follows the HotSpot notation.
        stkd.Dimensions->Chip.Length = chip_width*1e6;
        stkd.Dimensions->Chip.Width = chip_height*1e6;

        int grid_rows = pseudo_component->lookup_in_library("grid_rows");
        int grid_columns = pseudo_component->lookup_in_library("grid_columns");

        stkd.Dimensions->Grid.NRows = grid_rows;
        stkd.Dimensions->Grid.NColumns = grid_columns;
        
        // Length = x dim = Columns
        stkd.Dimensions->Cell.ChannelLength = stkd.Dimensions->Cell.FirstWallLength
        = stkd.Dimensions->Cell.LastWallLength = stkd.Dimensions->Cell.WallLength
        = stkd.Dimensions->Chip.Length/stkd.Dimensions->Grid.NColumns;
        // Width = y dim = Rows
        stkd.Dimensions->Cell.Width = stkd.Dimensions->Chip.Width/stkd.Dimensions->Grid.NRows;

        Setting &material_setting = pseudo_component->lookup_in_library("material");
       
        if(material_setting.getLength() == 0) {
            pseudo_component->kitfox->error("3d-ice","%s.library.material is not defined",pseudo_component->name.c_str());
        }
        
        // Materials description
        for(unsigned i = 0; i < material_setting.getLength(); i++) {
            Setting &each_material_setting = material_setting[i];
            Material *material = alloc_and_init_material();
            material->Id = (char*)each_material_setting.getName();
            if(find_material_in_list(stkd.MaterialsList,material->Id)) {
                pseudo_component->kitfox->error("3d-ice","%s.library.material.%s is already defined",pseudo_component->name.c_str(),material->Id);
            }
            
            // Ugly linked list of materials in 3D-ICE
            if(!stkd.MaterialsList)
                stkd.MaterialsList = material;
            else {
                Material *last_material = stkd.MaterialsList;
                while(last_material->Next) {
                    last_material = last_material->Next;
                }
                last_material->Next = material;
            }
            
            material->ThermalConductivity = each_material_setting["thermal_conductivity"];
            material->VolumetricHeatCapacity = each_material_setting["volumetric_heat_capacity"];
        }
        
        // Create conventional heat sink if defined.
        if(pseudo_component->exists_in_library("conventional_heat_sink")) {
            stkd.ConventionalHeatSink = alloc_and_init_conventional_heat_sink();
            stkd.ConventionalHeatSink->AmbientHTC = pseudo_component->lookup_in_library("conventional_heat_sink")["heat_transfer_coefficient"];
            stkd.ConventionalHeatSink->AmbientTemperature = ambient_temperature;
        }
        
        // Stack description - Channel
        Setting &microchannel_setting = pseudo_component->lookup_in_library("microchannel");
        const char *microchannel_type_str = microchannel_setting["type"];
        if(!strcasecmp(microchannel_type_str,"2rm")) {
            stkd.Channel = alloc_and_init_channel();
            stkd.Channel->ChannelModel = TDICE_CHANNEL_MODEL_MC_2RM;
            stkd.Channel->Height = microchannel_setting["height"];
            if(stkd.Channel->Height > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Channel->Length = microchannel_setting["channel_length"];
            if(stkd.Channel->Length > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.channel_length is unrealistic; 1um = 1e-5",pseudo_component->name.c_str());
            }
            stkd.Channel->Height *= 1e6;
            stkd.Channel->Length *= 1e6;
            
            double microchannel_wall_length = microchannel_setting["wall_length"];
            if(microchannel_wall_length > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel_wall_length is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            microchannel_wall_length *= 1e6;
            stkd.Channel->Pitch = stkd.Channel->Length + microchannel_wall_length;
            stkd.Channel->NChannels = (stkd.Dimensions->Chip.Length/stkd.Channel->Pitch)+0.5;
            stkd.Channel->Porosity = stkd.Channel->Length / stkd.Channel->Pitch;
            stkd.Channel->NLayers = NUM_LAYERS_CHANNEL_2RM;
            stkd.Channel->SourceLayerOffset = SOURCE_OFFSET_CHANNEL_2RM;
            
            double coolant_heat_transfer_coefficient = 0.0;
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient"))
                coolant_heat_transfer_coefficient = microchannel_setting["coolant_heat_transfer_coefficient"];

            stkd.Channel->Coolant.HTCSide = coolant_heat_transfer_coefficient;
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_side")) 
                stkd.Channel->Coolant.HTCSide = microchannel_setting["coolant_heat_transfer_coefficient_side"];

            stkd.Channel->Coolant.HTCTop = coolant_heat_transfer_coefficient;                
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_top"))
                stkd.Channel->Coolant.HTCTop = microchannel_setting["coolant_heat_transfer_coefficient_top"];

            stkd.Channel->Coolant.HTCBottom = coolant_heat_transfer_coefficient;                
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_bottom"))
                stkd.Channel->Coolant.HTCBottom = microchannel_setting["coolant_heat_transfer_coefficient_bottom"];
                
            stkd.Channel->Coolant.VHC = microchannel_setting["coolant_volumetric_heat_capacity"];
            
            stkd.Channel->Coolant.TIn = ambient_temperature;
            if(microchannel_setting.exists("coolant_incoming_temperature"))
                stkd.Channel->Coolant.TIn = microchannel_setting["coolant_incoming_temperature"];
                
            stkd.Channel->Coolant.FlowRate = microchannel_setting["coolant_flow_rate"];
            stkd.Channel->Coolant.FlowRate = FLOW_RATE_FROM_MLMIN_TO_UM3SEC(stkd.Channel->Coolant.FlowRate);

            const char *microchannel_wall_material_str = microchannel_setting["wall_material"];
            stkd.Channel->WallMaterial = find_material_in_list(stkd.MaterialsList,(char*)microchannel_wall_material_str);
            if(!stkd.Channel->WallMaterial) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.wall_material = %s is unknown",pseudo_component->name.c_str(),microchannel_wall_material_str);
            }
            else
                stkd.Channel->WallMaterial->Used++;
        }
        else if(!strcasecmp(microchannel_type_str,"4rm")) {
            stkd.Channel = alloc_and_init_channel();
            stkd.Channel->ChannelModel = TDICE_CHANNEL_MODEL_MC_4RM;
            stkd.Channel->Height = microchannel_setting["height"];
            if(stkd.Channel->Height > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Channel->Height *= 1e6;
            
            stkd.Dimensions->Cell.ChannelLength = microchannel_setting["channel_length"];
            if(stkd.Dimensions->Cell.ChannelLength > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.channel_length is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Dimensions->Cell.WallLength = microchannel_setting["wall_length"];
            if(stkd.Dimensions->Cell.WallLength > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.wall_length is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Dimensions->Cell.ChannelLength *= 1e6;
            stkd.Dimensions->Cell.WallLength *= 1e6;
            if(microchannel_setting.exists("first_wall_length")) {
                stkd.Dimensions->Cell.FirstWallLength = microchannel_setting["first_wall_length"];
                if(stkd.Dimensions->Cell.FirstWallLength > 1.0) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.first_wall_length is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
                }
                stkd.Dimensions->Cell.FirstWallLength *= 1e6;
            }
            else 
                stkd.Dimensions->Cell.FirstWallLength = stkd.Dimensions->Cell.WallLength;
            if(microchannel_setting.exists("last_wall_length")) {
                stkd.Dimensions->Cell.LastWallLength = microchannel_setting["last_wall_length"];
                if(stkd.Dimensions->Cell.LastWallLength > 1.0) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.last_wall_length is unrealistic",pseudo_component->name.c_str());
                }
                stkd.Dimensions->Cell.LastWallLength *= 1e6;
            }
            else
                stkd.Dimensions->Cell.LastWallLength = stkd.Dimensions->Cell.WallLength;
            
            CellDimension_t ratio = (stkd.Dimensions->Chip.Length - stkd.Dimensions->Cell.FirstWallLength - stkd.Dimensions->Cell.LastWallLength - stkd.Dimensions->Cell.ChannelLength)/(stkd.Dimensions->Cell.ChannelLength + stkd.Dimensions->Cell.WallLength);
            
            if(ratio != (int)ratio) {
                pseudo_component->kitfox->error("3d-ice","%s.library has cell dimensions that do not fit the chip length",pseudo_component->name.c_str());
            }
            
            stkd.Dimensions->Grid.NColumns = 2*ratio+3;
            
            if((stkd.Dimensions->Grid.NColumns&1) == 0) {
                pseudo_component->kitfox->error("3d-ice","%s.library must have odd number of columns when channel is declared",pseudo_component->name.c_str());
            }
            
            if(stkd.Dimensions->Grid.NColumns < 3) {
                pseudo_component->kitfox->error("3d-ice","%s.library doesn't have enought grid columns",pseudo_component->name.c_str());
            }
            
            stkd.Channel->NChannels = (stkd.Dimensions->Grid.NColumns-1)/2;
            stkd.Channel->NLayers = NUM_LAYERS_CHANNEL_4RM;
            stkd.Channel->SourceLayerOffset = SOURCE_OFFSET_CHANNEL_4RM;
            
            double coolant_heat_transfer_coefficient = microchannel_setting["coolant_heat_transfer_coefficient"];
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_side")) 
                stkd.Channel->Coolant.HTCSide = microchannel_setting["coolant_heat_transfer_coefficient_side"];
            else
                stkd.Channel->Coolant.HTCSide = coolant_heat_transfer_coefficient;
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_top"))
                stkd.Channel->Coolant.HTCTop = microchannel_setting["coolant_heat_transfer_coefficient_top"];
            else
                stkd.Channel->Coolant.HTCTop = coolant_heat_transfer_coefficient;
            if(microchannel_setting.exists("coolant_heat_transfer_coefficient_bottom"))
                stkd.Channel->Coolant.HTCBottom = microchannel_setting["coolant_heat_transfer_coefficient_bottom"];
            else
                stkd.Channel->Coolant.HTCBottom = coolant_heat_transfer_coefficient;
            stkd.Channel->Coolant.VHC = microchannel_setting["coolant_volumetric_heat_capacity"];
            if(microchannel_setting.exists("coolant_incoming_temperature"))
                stkd.Channel->Coolant.TIn = microchannel_setting["coolant_incoming_temperature"];
            else
                stkd.Channel->Coolant.TIn = ambient_temperature;
            stkd.Channel->Coolant.FlowRate = microchannel_setting["coolant_flow_rate"];
            stkd.Channel->Coolant.FlowRate = FLOW_RATE_FROM_MLMIN_TO_UM3SEC(stkd.Channel->Coolant.FlowRate);
            
            const char *microchannel_wall_material_str = microchannel_setting["wall_material"];
            stkd.Channel->WallMaterial = find_material_in_list(stkd.MaterialsList,(char*)microchannel_wall_material_str);
            if(!stkd.Channel->WallMaterial) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.wall_material = %s is unknown",pseudo_component->name.c_str(),microchannel_wall_material_str);
            }
            else
                stkd.Channel->WallMaterial->Used++;
        }
        else if(!strcasecmp(microchannel_type_str,"pinfin")) {
            stkd.Channel = alloc_and_init_channel();
            
            const char *channel_model_str = microchannel_setting["channel_model"];
            if(!strcasecmp(channel_model_str,"inline"))
                stkd.Channel->ChannelModel = TDICE_CHANNEL_MODEL_PF_INLINE;
            else if(!strcasecmp(channel_model_str,"staggered"))
                stkd.Channel->ChannelModel = TDICE_CHANNEL_MODEL_PF_STAGGERED;
            else {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.channel_model = %s is unknown",pseudo_component->name.c_str(),channel_model_str);
            }

            stkd.Channel->Height = microchannel_setting["height"];
            if(stkd.Channel->Height > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Channel->Height *= 1e6;
            
            stkd.Channel->Pitch = microchannel_setting["pin_pitch"];
            if(stkd.Channel->Pitch > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.pin_pitch is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            stkd.Channel->Pitch *= 1e6;
            
            double pin_diameter = microchannel_setting["pin_diameter"];
            if(pin_diameter > 1.0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.pin_diameter is unrealistic; 1um = 1e-6",pseudo_component->name.c_str());
            }
            pin_diameter *= 1e6;
            stkd.Channel->Porosity = POROSITY(pin_diameter,stkd.Channel->Pitch);
            stkd.Channel->NLayers = NUM_LAYERS_CHANNEL_2RM;
            stkd.Channel->SourceLayerOffset = SOURCE_OFFSET_CHANNEL_2RM;
            stkd.Channel->Coolant.DarcyVelocity = microchannel_setting["darcy_velocity"];
            stkd.Channel->Coolant.HTCSide = 0.0;
            stkd.Channel->Coolant.HTCTop = 0.0;
            stkd.Channel->Coolant.HTCBottom = 0.0;
            stkd.Channel->Coolant.VHC = microchannel_setting["coolant_volumetric_heat_capacity"];
            if(microchannel_setting.exists("coolant_incoming_temperature"))
                stkd.Channel->Coolant.TIn = microchannel_setting["coolant_incoming_temperature"];
            else
                stkd.Channel->Coolant.TIn = ambient_temperature;
            
            const char *pin_material_str = microchannel_setting["pin_material"];
            stkd.Channel->WallMaterial = find_material_in_list(stkd.MaterialsList,(char*)pin_material_str);
            if(!stkd.Channel->WallMaterial) {
                pseudo_component->kitfox->error("3d-ice","%s.library.microchannel.pin_material = %d is unknown",pseudo_component->name.c_str(),pin_material_str);
            }
            else
                stkd.Channel->WallMaterial->Used++;
        }
        else {
            pseudo_component->kitfox->error("3d-ice","%s.library.microchannel_type = %s is unknonw",pseudo_component->name.c_str(),microchannel_type_str);
        }
        
        // Stack description - Die
        Setting &die_setting = pseudo_component->lookup_in_library("die");
        for(unsigned i = 0; i < die_setting.getLength(); i++) {
            Setting &each_die_setting = die_setting[i];
            
            Die *die = alloc_and_init_die();
            // Ugly linked list of Dies in 3D-ICE
            if(!stkd.DiesList)
                stkd.DiesList = die;
            else {
                Die *last_die = stkd.DiesList;
                while(last_die->Next)
                    last_die = last_die->Next;
                last_die->Next = die;
            }
            
            die->Id = (char*)each_die_setting.getName();
            bool is_source_layer = false;
            
            Setting &layer_setting = each_die_setting["layer"];
            for(unsigned j = 0; j < layer_setting.getLength(); j++) {
                Setting &each_layer_setting = layer_setting[j];
                
                Layer *layer = alloc_and_init_layer();
                // Ugly linked list of Layers in 3D-ICE
                if(!die->TopLayer)
                    die->TopLayer = layer;
                else {
                    Layer *last_layer = die->TopLayer;
                    while(last_layer->Prev)
                        last_layer = last_layer->Prev;
                    JOIN_ELEMENTS(layer,last_layer);
                }
                die->BottomLayer = layer;
                
                if(die->SourceLayer)
                    die->SourceLayerOffset++;
                    
                layer->Height = each_layer_setting["height"];
                if(layer->Height > 1.0) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.die.%s.layer.%s.height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str(),die->Id,each_layer_setting.getName());
                }
                layer->Height *= 1e6;
                
                const char *layer_material_str = each_layer_setting["material"];
                layer->Material = find_material_in_list(stkd.MaterialsList,(char*)layer_material_str);
                if(!layer->Material) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.die.%s.layer.%s.material = %s is unknown",pseudo_component->name.c_str(),die->Id,each_layer_setting.getName(),layer_material_str);
                }
                else
                    layer->Material->Used++;
                    
                die->NLayers++;
                
                is_source_layer = each_layer_setting["is_source_layer"];
                if(is_source_layer) {
                    if(die->SourceLayer) {
                        pseudo_component->kitfox->error("3d-ice","%s.library.die.%s can't have multiple source layers",pseudo_component->name.c_str(),die->Id);
                    }
                    else
                        die->SourceLayer = layer;
                }
            }
        }
        
        // Stack description - Stack
        Setting &stack_setting = pseudo_component->lookup_in_library("stack");
        unsigned num_layers = 0;
        unsigned num_channels = 0;
        unsigned num_dies = 0;
        double stack_height = 0.0;
        
        for(unsigned i = 0; i < stack_setting.getLength(); i++) {
            Setting &each_stack_setting = stack_setting[i];
            
            StackElement *stack_element = alloc_and_init_stack_element();
            stack_element->Id = (char*)each_stack_setting.getName();
            
            const char *stack_type_str = each_stack_setting["type"];
            if(!strcasecmp(stack_type_str,"layer")) {
                num_layers++;
                Layer *layer = alloc_and_init_layer();
                layer->Height = each_stack_setting["height"];
                if(layer->Height > 1.0) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s.height is unrealistic; 1um = 1e-6",pseudo_component->name.c_str(),stack_element->Id);
                }
                layer->Height *= 1e6;
                
                const char *stack_material_str = each_stack_setting["material"];
                layer->Material = find_material_in_list(stkd.MaterialsList,(char*)stack_material_str);
                if(!layer->Material) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s.material = %s is unknown",pseudo_component->name.c_str(),stack_element->Id,stack_material_str);
                }
                else
                    layer->Material->Used++;
                    
                stack_element->Type = TDICE_STACK_ELEMENT_LAYER;
                stack_element->Pointer.Layer = layer;
                stack_element->NLayers = 1;
            }
            else if(!strcasecmp(stack_type_str,"channel")) {
                num_channels++;
                if(!stkd.Channel) {
                    pseudo_component->kitfox->error("3d-ice","%s.library has no channel defined",pseudo_component->name.c_str());
                }
                
                stack_element->Type = TDICE_STACK_ELEMENT_CHANNEL;
                stack_element->Pointer.Channel = stkd.Channel;
                stack_element->NLayers = stkd.Channel->NLayers;
            }
            else if(!strcasecmp(stack_type_str,"die")) {
                num_dies++;
                stack_element->Type = TDICE_STACK_ELEMENT_DIE;
                const char *stack_die_str = each_stack_setting["die"];
                stack_element->Pointer.Die = find_die_in_list(stkd.DiesList,(char*)stack_die_str);
                if(!stack_element->Pointer.Die) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s.die = %s is unknown",pseudo_component->name.c_str(),stack_element->Id,stack_die_str);
                }
                
                stack_element->Pointer.Die->Used++;
                stack_element->NLayers = stack_element->Pointer.Die->NLayers;
                
                stack_element->Floorplan = alloc_and_init_floorplan();
                for(unsigned j = 0; j < get_floorplan_counts(); j++) {
                    Comp_ID floorplan_id = get_floorplan_id(j); // 'j'th floorplan's ID
                    const char *floorplan_name =get_floorplan_name(floorplan_id);
                   
                    dimension_t floorplan_dimension;
                    int error = pseudo_component->kitfox->pull_data(floorplan_id,0.0,UNSPECIFIED_TIME,KITFOX_DATA_DIMENSION,&floorplan_dimension);
                    
                    // 3D-ICE uses the char name to indicate the die. The floorplan belongs to this stack element.
                    if(!strcasecmp(floorplan_dimension.die_name,stack_element->Id)) {
                        ICElement *icelement;
                        icelement = alloc_and_init_ic_element();
                        icelement->SW_X = floorplan_dimension.left*1e6; // convert to um
                        icelement->SW_Y = (floorplan_dimension.bottom)*1e6;// convert to um
                        icelement->Length = floorplan_dimension.width*1e6; // convert to um
                        icelement->Width = floorplan_dimension.height*1e6; // convert to um
                        
                        align_to_grid(stkd.Dimensions,icelement);
                        
                        if(check_location(stkd.Dimensions,icelement)) {
                            pseudo_component->kitfox->error("3d-ice","%s.library.dimension is out of chip dimension",pseudo_component->name.c_str());
                        }
                        
                        FloorplanElement *floorplan_element = alloc_and_init_floorplan_element();
                        floorplan_element->Id = (char*)floorplan_name;
                        floorplan_element->NICElements = 1;
                        floorplan_element->ICElementsList = icelement;
                        floorplan_element->EffectiveSurface = icelement->EffectiveLength*icelement->EffectiveWidth;
                        floorplan_element->PowerValues = alloc_and_init_powers_queue();
                        
                        for(FloorplanElement *flp_element = stack_element->Floorplan->ElementsList;
                            flp_element&&(flp_element != floorplan_element); flp_element = flp_element->Next) {
                            for(ICElement *ic_element = flp_element->ICElementsList;
                                ic_element&&(ic_element != icelement); ic_element = ic_element->Next) {
                                if(check_intersection(ic_element,icelement)) {
                                    pseudo_component->kitfox->error("3d-ice","%s.library has overlapping floorplans between %s(left=%lf,bottom=%lf,width=%lf,height=%lf) and %s(left=%lf,bottom=%lf,width=%lf,height=%lf",pseudo_component->name.c_str(),flp_element->Id,ic_element->SW_X,ic_element->SW_Y,ic_element->Length,ic_element->Width,floorplan_element->Id,icelement->SW_X,icelement->SW_Y,icelement->Length,icelement->Width);
                                }
                            }
                        }
                        
                        if(!stack_element->Floorplan->ElementsList) {
                            stack_element->Floorplan->ElementsList = floorplan_element;
                            stack_element->Floorplan->NElements = 1;
                        }
                        else {
                            FloorplanElement *last_floorplan_element = stack_element->Floorplan->ElementsList;
                            while(last_floorplan_element->Next) {
                                last_floorplan_element = last_floorplan_element->Next;
                            }
                            last_floorplan_element->Next = floorplan_element;
                            stack_element->Floorplan->NElements++;
                        }
                    }
                }
                 
                if(!stack_element->Floorplan->ElementsList) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s doesn't have floorplans",pseudo_component->name.c_str(),stack_element->Id);
                }
            }
            else {
                pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s.type = %s is unknown",pseudo_component->name.c_str(),stack_element->Id,stack_type_str);
            }
            
            if(!stkd.TopStackElement) {
                if(stack_element->Type == TDICE_STACK_ELEMENT_CHANNEL) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack can't define a channel as the top-most stack element",pseudo_component->name.c_str());
                }
                else {
                    stkd.TopStackElement = stack_element;
                    stkd.BottomStackElement = stack_element;
                }
            }
            else {
                if(find_stack_element_in_list(stkd.BottomStackElement,stack_element->Id)) {
                    pseudo_component->kitfox->error("3d-ice","%s.library.stack.%s is already defined",pseudo_component->name.c_str(),stack_element->Id);
                }
                else if((stkd.BottomStackElement->Type == TDICE_STACK_ELEMENT_CHANNEL)&&
                   (stack_element->Type == TDICE_STACK_ELEMENT_CHANNEL)) {
                   pseudo_component->kitfox->error("3d-ice","%s.library.stack can't have two consecutive channels",pseudo_component->name.c_str());
                }
                else {
                    JOIN_ELEMENTS(stack_element, stkd.BottomStackElement);
                    stkd.BottomStackElement = stack_element;
                }
            }
        }
        
        if(num_dies == 0) {
            pseudo_component->kitfox->error("3d-ice","%s.library.stack must have at least one die",pseudo_component->name.c_str());
        }
        if(stkd.BottomStackElement->Type == TDICE_STACK_ELEMENT_CHANNEL) {
            pseudo_component->kitfox->error("3d-ice","%s.library.stack can't define the channel as the bottom-most stack element",pseudo_component->name.c_str());
        }
        if(!stkd.ConventionalHeatSink&&!stkd.Channel) {
            pseudo_component->kitfox->error("3d-ice","%s.library has neither conventional_heat_sink or microchannel defined",pseudo_component->name.c_str());
        }
        for(Material *material = stkd.MaterialsList; material; material = material->Next) {
            if(material->Used == 0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.material.%s is defined but never used",pseudo_component->name.c_str(),material->Id);
            }
        }
        for(Die *die = stkd.DiesList; die; die = die->Next) {
            if(die->Used == 0) {
                pseudo_component->kitfox->error("3d-ice","%s.library.die.%s is defined but never used",pseudo_component->name.c_str(),die->Id);
            }
        }
        if(stkd.ConventionalHeatSink) {
            if(stkd.TopStackElement->Type == TDICE_STACK_ELEMENT_LAYER)
                stkd.ConventionalHeatSink->TopLayer = stkd.TopStackElement->Pointer.Layer;
            else
                stkd.ConventionalHeatSink->TopLayer = stkd.TopStackElement->Pointer.Die->TopLayer;
        }
        
        CellIndex_t layer_index = 0;
        for(StackElement *stack_element = stkd.BottomStackElement; stack_element; stack_element = stack_element->Next) {
            stack_element->Offset = layer_index;
            layer_index += stack_element->NLayers;
        }
        
        stkd.Dimensions->Grid.NLayers = layer_index;
        stkd.Dimensions->Grid.NCells = get_number_of_layers(stkd.Dimensions)*get_number_of_rows(stkd.Dimensions)*get_number_of_columns(stkd.Dimensions);
        if(stkd.Dimensions->Grid.NCells > INT32_MAX) {
            pseudo_component->kitfox->error("3d-ice","%s.library has too many grid cells",pseudo_component->name.c_str());
        }
        if(!stkd.Channel)
            compute_number_of_connections(stkd.Dimensions,num_channels,TDICE_CHANNEL_MODEL_NONE);
        else
            compute_number_of_connections(stkd.Dimensions,num_channels,stkd.Channel->ChannelModel);
        
        // Analysis description
        if(pseudo_component->exists_in_library("temperature"))
            analysis.InitialTemperature = pseudo_component->lookup_in_library("temperature");
        else
            analysis.InitialTemperature = ambient_temperature;
        
        analysis.StepTime = 0.02;
        analysis.SlotTime = 0.02;
        analysis.SlotLength = 1;
        const char *thermal_analysis_type_str = pseudo_component->lookup_in_library("thermal_analysis_type");
        if(!strcasecmp(thermal_analysis_type_str,"transient")) {
            analysis.AnalysisType = TDICE_ANALYSIS_TYPE_TRANSIENT;
            emulate = &emulate_step;
        }
        else if(!strcasecmp(thermal_analysis_type_str,"steady")||!strcasecmp(thermal_analysis_type_str,"steady_state")) {
            analysis.AnalysisType = TDICE_ANALYSIS_TYPE_STEADY;
            emulate = &emulate_steady;
        }
        else {
            pseudo_component->kitfox->error("3d-ice","%s.library.thermal_analysis_type = %s is unknown",pseudo_component->name.c_str(),thermal_analysis_type_str);
        }
        
        num_source_layers = 0;
        StackElement *stack_element = stkd.BottomStackElement;
        while(stack_element) {
            if(stack_element->Type == TDICE_STACK_ELEMENT_DIE) {
                InspectionPoint *stack_inspection_point = alloc_and_init_inspection_point();
                stack_inspection_point->Type = TDICE_OUTPUT_TYPE_TMAP;
                stack_inspection_point->Instant = TDICE_OUTPUT_INSTANT_FINAL;
                stack_inspection_point->StackElement = stack_element;
                add_inspection_point_to_analysis(&analysis,stack_inspection_point);
            }
            
            if(stack_element->Floorplan) {
                // Assign source layer index to floorplan
                FloorplanElement *floorplan_element = stack_element->Floorplan->ElementsList;
                while(floorplan_element) {
                    Comp_ID floorplan_id = get_floorplan_id(floorplan_element->Id);
                    dimension_t floorplan_dimension;
                    int error = pseudo_component->kitfox->pull_data(floorplan_id,0.0,KITFOX_DATA_DIMENSION,&floorplan_dimension);
                    floorplan_dimension.die_index = num_source_layers;
                    error = pseudo_component->kitfox->overwrite_data(floorplan_id,0.0,MAX_TIME,KITFOX_DATA_DIMENSION,&floorplan_dimension);
                    floorplan_element = floorplan_element->Next;
                }
                num_source_layers++;
            }
            stack_element = stack_element->Next;
        }
        
        /*
        // DEBUG MATERIAL
        cout << endl;
        Material *m = stkd.MaterialsList;
        while(m) {
            cout << "material.Id = " << m->Id << endl;
            cout << "material.Used = " << m->Used << endl;
            cout << "material.VolumetricHeatCapacity = " << m->VolumetricHeatCapacity << endl;
            cout << "material.ThermalConductivity = " << m->ThermalConductivity << endl;
            m = m->Next;
        }
        cout << endl; 
        
        // DEBUG CONVENTIONAL_HEAT_SINK
        ConventionalHeatSink *h = stkd.ConventionalHeatSink;
        if(h) {
            cout << "conventional_heat_sink.AmbientHTC = " << h->AmbientHTC << endl;
            cout << "conventional_heat_sink.AmbientTemperature = " << h->AmbientTemperature << endl;
            
            Layer *l = h->TopLayer;
            while(l) {
                cout << "layer.Height = " << l->Height << endl;
                cout << "layer.Material = " << l->Material->Id << endl;
                l = l->Prev;
            }
        }
        cout << endl; 
        
        // DEBUG MICROCHANNEL
        Channel *c = stkd.Channel;
        if(c) {
            cout << "channel.ChannelModel = " << c->ChannelModel << endl;
            cout << "channel.Height = " << c->Height << endl;
            cout << "channel.Length = " << c->Length << endl;
            cout << "channel.Pitch = " << c->Pitch << endl;
            cout << "channel.Porosity = " << c->Porosity << endl;
            cout << "channel.NChannels = " << c->NChannels << endl;
            cout << "channel.NLayers = " << c->NLayers << endl;
            cout << "channel.SourceLayerOffset = " << c->SourceLayerOffset << endl;
            cout << "channel.Coolant.HTCSide = " << c->Coolant.HTCSide << endl;
            cout << "channel.Coolant.HTCTop = " << c->Coolant.HTCTop << endl;
            cout << "channel.Coolant.HTCBottom = " << c->Coolant.HTCBottom << endl;
            cout << "channel.Coolant.VHC = " << c->Coolant.VHC << endl;
            cout << "channel.Coolant.FlowRate = " << c->Coolant.FlowRate << endl;
            cout << "channel.Coolant.DarcyVelocity = " << c->Coolant.DarcyVelocity << endl;
            cout << "channel.Coolant.TIn = " << c->Coolant.TIn << endl;
            cout << "channel.WallMaterial = " << c->WallMaterial->Id << endl;
        }
        cout << endl;
        
        // DEBUG DIE
        Die *d = stkd.DiesList;
        while(d) {
            cout << "die.Id = " << d->Id << endl;
            cout << "die.Used = " << d->Used << endl;
            cout << "die.NLayers = " << d->NLayers << endl;
            cout << "die.SourceLayerOffset = " << d->SourceLayerOffset << endl;
            
            Layer *l = d->TopLayer;
            while(l) {
                cout << "layer.Height = " << l->Height << endl;
                cout << "layer.Material = " << l->Material->Id << endl;
                l = l->Prev;
            }
            
            d = d->Next;
        }
        cout << endl;
        
        // DEBUG DIMENSIONS
        if(stkd.Dimensions) {
            cout << "Dimensions.Cell.FirstWallLength = " << stkd.Dimensions->Cell.FirstWallLength << endl;
            cout << "Dimensions.Cell.WallLength = " << stkd.Dimensions->Cell.WallLength << endl;
            cout << "Dimensions.Cell.ChannelLength = " << stkd.Dimensions->Cell.ChannelLength << endl;
            cout << "Dimensions.Cell.LastWallLength = " << stkd.Dimensions->Cell.LastWallLength << endl;
            cout << "Dimensions.Cell.Width = " << stkd.Dimensions->Cell.Width << endl;
            cout << "Dimensions.Grid.NLayers = " << stkd.Dimensions->Grid.NLayers << endl;
            cout << "Dimensions.Grid.NRows = " << stkd.Dimensions->Grid.NRows << endl;
            cout << "Dimensions.Grid.NColumns = " << stkd.Dimensions->Grid.NColumns << endl;
            cout << "Dimensions.Grid.NCells = " << stkd.Dimensions->Grid.NCells << endl;
            cout << "Dimensions.Grid.NConnections = " << stkd.Dimensions->Grid.NConnections << endl;
            cout << "Dimensions.Chip.Length = " << stkd.Dimensions->Chip.Length << endl;
            cout << "Dimensions.Chip.Width = " << stkd.Dimensions->Chip.Width << endl;
        }
        cout << endl;
        
        // DEBUG STACK_ELEMENT
        StackElement *s = stkd.TopStackElement;
        while(s) {
            cout << "stack.Id = " << s->Id << endl;
            cout << "stack.Type = " << s->Type << endl;
            cout << "stack.NLayers = " << s->NLayers << endl;
            cout << "stack.Offset = " << s->Offset << endl;
            
            if(s->Type == TDICE_STACK_ELEMENT_DIE) {
                cout << "stack.Floorplan.NElements = " << s->Floorplan->NElements << endl;
                
                FloorplanElement *f = s->Floorplan->ElementsList;
                while(f) {
                    cout << "floorplan.Id = " << f->Id << endl;
                    cout << "floorplan.NICElements = " << f->NICElements << endl;
                    cout << "floorplan.EffectiveSurface = " << f->EffectiveSurface << endl;
                    cout << "floorplan.PowerValues.Length = " << f->PowerValues->Length << endl;
                    
                    ICElement *i = f->ICElementsList;
                    while(i) {
                        cout << "icelement.SW_X = " << i->SW_X << endl;
                        cout << "icelement.SW_Y = " << i->SW_Y << endl;
                        cout << "icelement.Length = " << i->Length << endl;
                        cout << "icelement.Width = " << i->Width << endl;
                        cout << "icelement.EffectiveLength = " << i->EffectiveLength << endl;
                        cout << "icelement.EffectiveWidth = " << i->EffectiveWidth << endl;
                        cout << "icelement.SW_Row = " << i->SW_Row << endl;
                        cout << "icelement.SW_Column = " << i->SW_Column << endl;
                        cout << "icelement.NE_Row = " << i->NE_Row << endl;
                        cout << "icelement.NE_Column = " << i->NE_Column << endl;
                        i = i->Next;
                    }
                    
                    PowerNode *p = f->PowerValues->Head;
                    while(p) {
                        cout << "power_value.Value " << p->Value << endl;
                        p = p->Next;
                    }
                    
                    f = f->Next;
                }
            }
            
            s = s->Prev;
        }
        cout << endl;
        */

        init_thermal_data(&tdata);
        error = fill_thermal_data(&tdata,&stkd,&analysis);
        if(error != TDICE_SUCCESS) {
            pseudo_component->kitfox->error("3d-ice","%s failed at initialization",pseudo_component->name.c_str());
        }
    }
    catch(SettingTypeException e) {
        cout << "KITFOX ERROR (libconfig): " << e.getPath() << " has incorrect type" << endl;
        exit(EXIT_FAILURE);
    }
}


void thermallib_3dice::calculate_temperature(Second Time, Second Period) {
    if(!tdata.Temperatures) {
        pseudo_component->kitfox->error("3d-ice","%s can't compute temperature - data structure is invalid",pseudo_component->name.c_str());
    }
    
    if(analysis.StepTime != Period) {
        analysis.SlotTime = Time;
        analysis.SlotLength = Period;
        analysis.StepTime = Period;
        
        Destroy_SuperMatrix_Store(&tdata.SLUMatrix_A);
        Destroy_SuperMatrix_Store(&tdata.SLUMatrix_B);
        Destroy_CompCol_Permuted(&tdata.SLUMatrix_A_Permuted);
        Destroy_SuperNode_Matrix(&tdata.SLUMatrix_L);
        Destroy_CompCol_Matrix(&tdata.SLUMatrix_U);
        free_system_matrix(&tdata.SM_A);
        init_system_matrix(&tdata.SM_A);
        dCreate_Dense_Matrix(&tdata.SLUMatrix_B,tdata.Size,1,tdata.Temperatures,tdata.Size,SLU_DN,SLU_D,SLU_GE);
        fill_thermal_cell_stack_description(tdata.ThermalCells,&analysis,&stkd);

        if(alloc_system_matrix(&tdata.SM_A,tdata.Size,get_number_of_connections(stkd.Dimensions)) == TDICE_FAILURE) {
            pseudo_component->kitfox->error("3d-ice","%s failed at allocating system matrix",pseudo_component->name.c_str());
        }
        else
            fill_system_matrix_stack_description(tdata.SM_A,tdata.ThermalCells,&stkd);
            
        dCreate_CompCol_Matrix(&tdata.SLUMatrix_A,tdata.Size,tdata.Size,tdata.SM_A.NNz,tdata.SM_A.Values,(int*)tdata.SM_A.RowIndices,(int*)tdata.SM_A.ColumnPointers,SLU_NC,SLU_D,SLU_GE);
        get_perm_c(tdata.SLU_Options.ColPerm,&tdata.SLUMatrix_A,tdata.SLU_PermutationMatrixC);
        sp_preorder(&tdata.SLU_Options,&tdata.SLUMatrix_A,tdata.SLU_PermutationMatrixC,tdata.SLU_Etree,&tdata.SLUMatrix_A_Permuted);
        dgstrf(&tdata.SLU_Options,&tdata.SLUMatrix_A_Permuted,sp_ienv(2),sp_ienv(1),tdata.SLU_Etree,NULL,0,tdata.SLU_PermutationMatrixC,tdata.SLU_PermutationMatrixR,&tdata.SLUMatrix_L,&tdata.SLUMatrix_U,&tdata.SLU_Stat,&tdata.SLU_Info);
    }
    else {
        analysis.SlotTime = Time;
        analysis.SlotLength = Period;
        analysis.StepTime = Period;
    }

    error = fill_sources_stack_description(tdata.Sources,tdata.ThermalCells,&stkd);
  
    if(error == TDICE_FAILURE) {
        pseudo_component->kitfox->error("3d-ice","%s failed at accessing stack information",pseudo_component->name.c_str());
        free_thermal_data(&tdata);
        exit(EXIT_FAILURE);
    }

    if(analysis.AnalysisType == TDICE_ANALYSIS_TYPE_TRANSIENT)
        fill_system_vector(stkd.Dimensions,tdata.Temperatures,tdata.Sources,tdata.ThermalCells,tdata.Temperatures);
    else if(analysis.AnalysisType == TDICE_ANALYSIS_TYPE_STEADY)
        fill_system_vector_steady(stkd.Dimensions,tdata.Temperatures,tdata.Sources);

    dgstrs(NOTRANS,&tdata.SLUMatrix_L,&tdata.SLUMatrix_U,tdata.SLU_PermutationMatrixC,tdata.SLU_PermutationMatrixR,&tdata.SLUMatrix_B,&tdata.SLU_Stat,&tdata.SLU_Info);

    if(tdata.SLU_Info < 0) {
        pseudo_component->kitfox->error("3d-ice","%s failed at computing temperature",pseudo_component->name.c_str());
        free_thermal_data(&tdata);
        exit(EXIT_FAILURE);
    }
}


grid_t<Kelvin> thermallib_3dice::get_thermal_grid(void) {
    grid_t<Kelvin> thermal_grid(stkd.Dimensions->Chip.Length/stkd.Dimensions->Grid.NColumns*1e-6,
                                stkd.Dimensions->Chip.Width/stkd.Dimensions->Grid.NRows*1e-6,
                                stkd.Dimensions->Grid.NColumns,stkd.Dimensions->Grid.NRows,num_source_layers);

    unsigned layer_offset = 0;    
    InspectionPoint *inspection_point = analysis.InspectionPointListFinal;
    while(inspection_point) {
        if(inspection_point->Type == TDICE_OUTPUT_TYPE_TMAP) {  // thermal map from the bottom to top of the stack.
            StackElement *stack_element = inspection_point->StackElement;
            double *temperature = tdata.Temperatures;
            temperature += get_cell_offset_in_stack(stkd.Dimensions,get_source_layer_offset(stack_element),0,0);
            for(unsigned row_index = FIRST_ROW_INDEX; row_index <= LAST_ROW_INDEX(stkd.Dimensions); row_index++) {
                for(unsigned column_index = FIRST_COLUMN_INDEX; column_index <= LAST_COLUMN_INDEX(stkd.Dimensions); column_index++) {
                    thermal_grid(column_index,row_index,layer_offset) = *temperature++;
                }
            }
            layer_offset++;            
        }
        inspection_point = inspection_point->Next;
    }
    return thermal_grid;
}


Kelvin thermallib_3dice::get_floorplan_temperature(Comp_ID ComponentID, int Type) {
    Kelvin temperature = 0.0;
    
    if(Type == KITFOX_TEMPERATURE_MAPPING_UNKNOWN)
        Type = thermal_grid_mapping; 
    
    StackElement *stack_element = stkd.BottomStackElement;
    while(stack_element) {
        if(stack_element->Floorplan) {
            const char *floorplan_name = get_floorplan_name(ComponentID);
            FloorplanElement *floorplan_element = find_floorplan_element_in_list(stack_element->Floorplan->ElementsList,(char*)floorplan_name);
            if(floorplan_element) {
                Temperature_t *temperatures = tdata.Temperatures;
                temperatures += get_cell_offset_in_stack(stkd.Dimensions,get_source_layer_offset(stack_element),0,0);
                switch(Type) {
                    case KITFOX_TEMPERATURE_MAPPING_MAX: {
                        temperature = get_max_temperature_floorplan_element(floorplan_element,stkd.Dimensions,temperatures);
                        break;
                    }
                    case KITFOX_TEMPERATURE_MAPPING_MIN: {
                        temperature = get_min_temperature_floorplan_element(floorplan_element,stkd.Dimensions,temperatures);
                        break;
                    }
                    case KITFOX_TEMPERATURE_MAPPING_CENTER: {
                        // Assume there is only a single ICElement per Floorplan.
                        Tcell *tcell = alloc_and_init_tcell();
                        align_tcell(tcell,floorplan_element->ICElementsList->SW_X+floorplan_element->ICElementsList->Length/2.0,
                                          floorplan_element->ICElementsList->SW_Y+floorplan_element->ICElementsList->Width/2.0,
                                    stkd.Dimensions);
                        temperature = *(temperatures+get_cell_offset_in_stack(stkd.Dimensions,get_source_layer_offset(stack_element),tcell->RowIndex,tcell->ColumnIndex));
                        free_tcell(tcell); //delete tcell;
                        break;
                    }
                    default: { // KITFOX_TEMPERATURE_MAPPING_AVERAGE
                        temperature = get_avg_temperature_floorplan_element(floorplan_element,stkd.Dimensions,temperatures);
                        break; // Switch statement break
                    }
                }
                break; // While loop break
            }
        }
        stack_element = stack_element->Next;
    }
    
    return temperature;
}


Kelvin thermallib_3dice::get_point_temperature(Meter X, Meter Y, Index Layer) {
    Kelvin temperature = 0.0;
    unsigned layer_offset = 0;
    
    StackElement *stack_element = stkd.BottomStackElement;
    while(stack_element) {
        if(stack_element->Floorplan) {
            if(Layer == layer_offset) {
                Temperature_t *temperatures = tdata.Temperatures;
                Tcell *tcell = alloc_and_init_tcell();
                align_tcell(tcell,X*1e6,Y*1e6,stkd.Dimensions);
                temperature = *(temperatures+get_cell_offset_in_stack(stkd.Dimensions,get_source_layer_offset(stack_element),tcell->RowIndex,tcell->ColumnIndex));
                free_tcell(tcell); //delete tcell;
                break;
            }
            else
                layer_offset++;
        }
        stack_element = stack_element->Next;
    }
    
    return temperature;
}
    

void thermallib_3dice::map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower) {
    bool power_mapped = false;
    
    StackElement *stack_element = stkd.BottomStackElement;
    while(stack_element) {
        if(stack_element->Floorplan) {
            const char *floorplan_name = get_floorplan_name(ComponentID);
            FloorplanElement *floorplan_element = find_floorplan_element_in_list(stack_element->Floorplan->ElementsList,(char*)floorplan_name);
            if(floorplan_element) {
                if(floorplan_element->PowerValues->Head) {
                    pseudo_component->kitfox->error("3d-ice","%s is mapping the power values multiple times to the floorplan %s",pseudo_component->name.c_str(),floorplan_name);
                }
                else {
                    put_into_powers_queue(floorplan_element->PowerValues,PartitionPower.get_total());
                    power_mapped = true;
                    break;
                }
            }
        }
        stack_element = stack_element->Next;
    }
    
    if(!power_mapped) {
        pseudo_component->kitfox->error("3d-ice","%s can't map the power value to a floorplan %s",pseudo_component->name.c_str(),get_floorplan_name(ComponentID));
    }
}


bool thermallib_3dice::update_library_variable(int Type, void *Value, bool isLibraryVariable) {
    bool updated = false;
    // NOTHING TO DO
    /*
    if(isLibraryVariable) {
    }
    else {
    }
    */
    return updated;
}

