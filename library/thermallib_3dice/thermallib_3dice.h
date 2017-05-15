#ifndef __THERMALLIB_3DICE_H__
#define __THERMALLIB_3DICE_H__

#include "library.h"

extern "C" {
#include "3d-ice/include/analysis.h"
#include "3d-ice/include/channel.h"
#include "3d-ice/include/conventional_heat_sink.h"
#include "3d-ice/include/die.h"
#include "3d-ice/include/dimensions.h"
#include "3d-ice/include/floorplan.h"
#include "3d-ice/include/floorplan_element.h"
#include "3d-ice/include/ic_element.h"
#include "3d-ice/include/inspection_point.h"
#include "3d-ice/include/layer.h"
#include "3d-ice/include/macros.h"
#include "3d-ice/include/material.h"
#include "3d-ice/include/network_message.h"
#include "3d-ice/include/powers_queue.h"
#include "3d-ice/include/stack_element.h"
#include "3d-ice/include/system_matrix.h"
#include "3d-ice/include/thermal_cell.h"
#include "3d-ice/include/thermal_data.h"
#include "3d-ice/include/types.h"
}

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif

namespace libKitFox {
namespace libKitFox_3DICE {

class thermallib_3dice : public thermal_library_t {
public:
	thermallib_3dice(pseudo_component_t *PseudoComponent);
	~thermallib_3dice();

	virtual void initialize(void);
	virtual void calculate_temperature(Second Time, Second Period);
	virtual grid_t<Kelvin> get_thermal_grid(void);
	virtual Kelvin get_floorplan_temperature(Comp_ID ComponentID, int Type = KITFOX_TEMPERATURE_MAPPING_UNKNOWN);
	virtual Kelvin get_point_temperature(Meter X, Meter Y, Index Layer);
	virtual void map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower);
	virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);

private:
	StackDescription stkd;
	Analysis analysis;
	ThermalData tdata; 

	SimResult_t (*emulate) (ThermalData*, StackDescription*, Analysis*);
	SimResult_t sim_result;
	Error_t error; 

	int thermal_grid_mapping;
	unsigned num_source_layers;
};

} // namespace libKitFox_3DICE
} // namespace libKitFox
  
#endif
