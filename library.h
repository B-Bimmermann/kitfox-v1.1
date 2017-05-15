#ifndef __KITFOX_LIBRARY_H__
#define __KITFOX_LIBRARY_H__

#include <map>
#include <libconfig.h++>
#include "kitfox-defs.h"

namespace libKitFox
{
    class pseudo_component_t;

    class model_library_t
    {
    public:
        model_library_t(pseudo_component_t *PseudoComponent, int Type);
        virtual ~model_library_t() {}
      
        // A model library must support runtime update of parameter variables
        // This function is also used as a callback function when new data are
        // pushed into pseudo component queues.
        virtual bool update_library_variable(int Type, void *Value, bool isLibraryVariable = false) = 0;
      
        // Initializes a subclass model.
        virtual void initialize(void) = 0;
      
        // Type: energy, thermal, reliability, or sensor model.
        int type;
    
    protected:
        // Pseudo component that the model library is used at.
        pseudo_component_t *pseudo_component;
    };
  
    class energy_library_t : public model_library_t
    {
    public:
        energy_library_t(pseudo_component_t *PseudoComponent, int Type);
        virtual ~energy_library_t() {}
      
        // Calculate unit energy per access cycle.
        virtual unit_energy_t get_unit_energy(void) = 0;
        // Calculate TDP power w.r.t (user defined) max duty cycle.
        virtual power_t get_tdp_power(Kelvin MaxTemperature = 373.0) = 0;
        // Calculate runtime power.
        virtual power_t get_runtime_power(Second Time, Second Period, counter_t Counter) = 0;
        // Calculate area (if available).
        virtual MeterSquare get_area(void) = 0;
      
        // Type: specific energy library model (i.e., McPAT)
        int type;
    };
  
    class thermal_library_t : public model_library_t
    {
    public:
        thermal_library_t(pseudo_component_t *PseudoComponent, int Type);
        virtual ~thermal_library_t() {}
      
        // Calculate transient or steady-state temperature.
        virtual void calculate_temperature(Second Time, Second Period) = 0;
        // Returns 3D temperature grid stack of power source layers.
        virtual grid_t<Kelvin> get_thermal_grid(void) = 0;
        // Returns representative floorplan temperature.
        virtual Kelvin get_floorplan_temperature(Comp_ID ComponentID, int Type = KITFOX_TEMPERATURE_MAPPING_UNKNOWN) = 0;
        // Returns point temperature.
        virtual Kelvin get_point_temperature(Meter X, Meter Y, Index Layer) = 0;
        // Map floorplan power.
        virtual void map_floorplan_power(Comp_ID ComponentID, power_t FloorplanPower) = 0;
      
        // Add floorplan -- must be used by a thermal_library model
        void add_floorplan(std::string ComponentName, Comp_ID ComponentID);
        // Returns total number of floorplans 
        const Index get_floorplan_counts(void);
        // Returns a pseudo component ID of component_name
        const Comp_ID get_floorplan_id(std::string ComponentName);
        const char* get_floorplan_name(Comp_ID ComponentID);
        // Returns ith pseudo component ID in the map.
        const Comp_ID get_floorplan_id(unsigned i);
      
        // Type: specific thermal library model (i.e., HotSpot)
        int type;

        // Generate thermal grid?
        bool generate_thermal_grid;
   
    private:
        // Thermal library maintains the floorplan ID-name pairs.
        std::map<std::string,Comp_ID> floorplan_name_map;
        std::map<Comp_ID,std::string> floorplan_id_map;
    };
  
  class reliability_library_t : public model_library_t
  {
  public:
      reliability_library_t(pseudo_component_t *PseudoComponent, int Type);
      virtual ~reliability_library_t() {}
      
      // calculate failure rate
      virtual Unitless get_failure_rate(Second Time, Second Period, Kelvin Temperature, Volt Vdd, Hertz ClockFrequency) = 0;
    
      // Type: specific reliability library model.
      int type;
  };
  
  class sensor_library_t : public model_library_t
  {
  public:
      sensor_library_t(pseudo_component_t *PseudoComponent, int Type);
      virtual ~sensor_library_t() {}
    
      // Sense data.
      virtual void sense(Second Time, Second Period, void *Data) = 0;
      
      // Type: specific sensor library model.
      int type;
  };
} // namespace libKitFox

#endif
