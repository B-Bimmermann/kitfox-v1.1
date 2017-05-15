#ifndef __THERMALLIB_HOTSPOT_H__
#define __THERMALLIB_HOTSPOT_H__

#include <map>
#include "library.h"

extern "C" {
#include "HotSpot-5.02/flp.h"
#include "HotSpot-5.02/npe.h"
#include "HotSpot-5.02/package.h"
#include "HotSpot-5.02/shape.h"
#include "HotSpot-5.02/temperature.h"
#include "HotSpot-5.02/temperature_block.h"
#include "HotSpot-5.02/temperature_grid.h"
#include "HotSpot-5.02/util.h"
}

namespace libKitFox {
namespace libKitFox_HOTSPOT {

enum KITFOX_HOTSPOT_THERMAL_ANALYSIS_TYPES
{
	KITFOX_HOTSPOT_THERMAL_ANALYSIS_UNKNOWN = 0,
	KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT,
	KITFOX_HOTSPOT_THERMAL_ANALYSIS_STEADY_STATE,
	NUM_KITFOX_HOTSPOT_THERMAL_ANALYSIS_TYPES
};

static const char *KITFOX_HOTSPOT_THERMAL_ANALYSIS_STR[] =
{
	"KITFOX_HOTSPOT_THERMAL_ANALYSIS_UNKNOWN",
	"KITFOX_HOTSPOT_THERMAL_ANALYSIS_TRANSIENT",
	"KITFOX_HOTSPOT_THERMAL_ANALYSIS_STEADY_STATE",
	"NUM_KITFOX_HOTSPOT_THERMAL_ANALYSIS_TYPES"
};

class thermallib_hotspot : public thermal_library_t {
public:
	thermallib_hotspot(pseudo_component_t *PseudoComponent);
	~thermallib_hotspot();
	
	virtual void initialize(void);
	virtual void calculate_temperature(Second Time, Second Period);
	virtual grid_t<Kelvin> get_thermal_grid(void);
	virtual Kelvin get_floorplan_temperature(Comp_ID ComponentID, int Type = KITFOX_TEMPERATURE_MAPPING_UNKNOWN);
	virtual Kelvin get_point_temperature(Meter X, Meter Y, Index Layer);
	virtual void map_floorplan_power(Comp_ID ComponentID, power_t PartitionPower);
	virtual bool update_library_variable(int Type, void *Value, bool IsLibraryVariable = false);
	
private:
	std::string get_flp_name(Comp_ID ComponentID);
	
	thermal_config_t thermal_config; // Thermal configuration
	package_config_t package_config; // Package configuration (if used)
	convection_t convection; // Airflow parameters
	RC_model_t *model; // RC model
	flp_t *flp;	// Thermal floorplan
	
	double *temperature, *power; // Temperature and power grid array
	
	int thermal_analysis; // thermal analysis type
};

} // namespace libKitFox_HOTSPOT
} // namespace libKitFox

#endif
