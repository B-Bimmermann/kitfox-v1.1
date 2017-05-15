#ifndef __KITFOX_DEFS_H__
#define __KITFOX_DEFS_H__

#include <vector>
#include <iostream>
#include <climits>
#include <float.h>
#include <stdint.h>
#include <string.h>

namespace libKitFox
{
#define MAX_TIME DBL_MAX/2
#define MAX_TIME_PRECISION 1e-12 // 1 pico second
#define UNSPECIFIED_TIME 0.0
#define MAX_KITFOX_COMP_NAME_LENGTH 128
#define KITFOX_MPI_BUFFER_SIZE 512
#define INVALID_COMP_ID 0
#define MAX_KITFOX_COMPONENTS 10000
  
    typedef int Index;
    typedef uint64_t Comp_ID;
    typedef uint64_t Count;
    typedef double Meter;
    typedef double MeterSquare;
    typedef double MeterCube;
    typedef double Second;
    typedef double Hertz;
    typedef double Volt;
    typedef double Joule;
    typedef double Watt;
    typedef double Kelvin;
    typedef double Unitless;
    
    enum KITFOX_MSG_TAG_TYPE {
        KITFOX_MSG_TAG_UNKNOWN = 0,
        KITFOX_MSG_TAG_SERVER_READY,
        KITFOX_MSG_TAG_SERVER_CONNECT,
        KITFOX_MSG_TAG_SERVER_DISCONNECT,
        KITFOX_MSG_TAG_CLIENT_CONNECT,
        KITFOX_MSG_TAG_CLIENT_DISCONNECT,
        KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_REQ,
        KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_RES,
        KITFOX_MSG_TAG_GET_COMP_ID_RES,
        KITFOX_MSG_TAG_GET_COMP_NAME_RES,
        KITFOX_MSG_TAG_SYNCHRONIZE_DATA,
        KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES,
        KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_RES,
        KITFOX_MSG_TAG_PULL_DATA_RES,
        KITFOX_MSG_TAG_PUSH_DATA_RES,
        KITFOX_MSG_TAG_OVERWRITE_DATA_RES,
        KITFOX_MSG_TAG_CALCULATE_POWER_RES,
        KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_RES,
        KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_RES,
        KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_RES,
        KITFOX_MSG_TAG_PRIORITY_BOUND,
        KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_REQ,
        KITFOX_MSG_TAG_CALCULATE_POWER_REQ,
        KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_REQ,
        KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_REQ,
        KITFOX_MSG_TAG_GET_COMP_ID_REQ,
        KITFOX_MSG_TAG_GET_COMP_NAME_REQ,
        KITFOX_MSG_TAG_SYNCHRONIZE_DATA_REQ,
        KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_REQ,
        KITFOX_MSG_TAG_PULL_DATA_REQ,
        KITFOX_MSG_TAG_PUSH_DATA_REQ,
        KITFOX_MSG_TAG_OVERWRITE_DATA_REQ,
        NUM_KITFOX_MSG_TAG_TYPES
    };
    
    static const char *KITFOX_MSG_TAG_STR[] = {
        "KITFOX_MSG_TAG_UNKNOWN",
        "KITFOX_MSG_TAG_SERVER_READY",
        "KITFOX_MSG_TAG_SERVER_CONNECT",
        "KITFOX_MSG_TAG_SERVER_DISCONNECT",
        "KITFOX_MSG_TAG_CLIENT_CONNECT",
        "KITFOX_MSG_TAG_CLIENT_DISCONNECT",
        "KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_REQ",
        "KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_RES",
        "KITFOX_MSG_TAG_GET_COMP_ID_RES",
        "KITFOX_MSG_TAG_GET_COMP_NAME_RES",
        "KITFOX_MSG_TAG_SYNCHRONIZE_DATA",
        "KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES",
        "KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_RES",
        "KITFOX_MSG_TAG_PULL_DATA_RES",
        "KITFOX_MSG_TAG_PUSH_DATA_RES",
        "KITFOX_MSG_TAG_OVERWRITE_DATA_RES",
        "KITFOX_MSG_TAG_CALCULATE_POWER_RES",
        "KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_RES",
        "KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_RES",
        "KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_RES",
        "KITFOX_MSG_TAG_PRIORITY_BOUND",
        "KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_REQ",
        "KITFOX_MSG_TAG_CALCULATE_POWER_REQ",
        "KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_REQ",
        "KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_REQ",
        "KITFOX_MSG_TAG_GET_COMP_ID_REQ",
        "KITFOX_MSG_TAG_GET_COMP_NAME_REQ",
        "KITFOX_MSG_TAG_SYNCHRONIZE_DATA_REQ",
        "KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_REQ",
        "KITFOX_MSG_TAG_PULL_DATA_REQ",
        "KITFOX_MSG_TAG_PUSH_DATA_REQ",
        "KITFOX_MSG_TAG_OVERWRITE_DATA_REQ",
        "NUM_KITFOX_MSG_TAG_TYPES"
    };
    
    enum KITFOX_LIBRARY_MODEL_TYPE {
        KITFOX_LIBRARY_MODEL_UNKNOWN = 0,
        KITFOX_LIBRARY_MODEL_DRAMSIM,
        KITFOX_LIBRARY_MODEL_DSENT,
        KITFOX_LIBRARY_MODEL_INTSIM,
        KITFOX_LIBRARY_MODEL_MCPAT,
        KITFOX_LIBRARY_MODEL_TSV,
        KITFOX_LIBRARY_ENERGY_MODEL,
        KITFOX_LIBRARY_MODEL_3DICE,
        KITFOX_LIBRARY_MODEL_HOTSPOT,
        KITFOX_LIBRARY_MODEL_MICROFLUIDICS,
        KITFOX_LIBRARY_THERMAL_MODEL,
        KITFOX_LIBRARY_MODEL_FAILURE,
        KITFOX_LIBRARY_RELIABILITY_MODEL,
        KITFOX_LIBRARY_SENSOR_MODEL,
        NUM_KITFOX_LIBRARY_MODEL_TYPES
    };
    
    // IMPORTANT: If "enum KITFOX_DATA_TYPE" is modified,
    // update "static const int KITFOX_DATA_SIZE[]" at the bottom
    // of this file as well.
    enum KITFOX_DATA_TYPE {
        KITFOX_DATA_UNKNOWN = 0,
        KITFOX_DATA_INDEX,
        KITFOX_DATA_TIME,
        KITFOX_DATA_PERIOD,
        KITFOX_DATA_COUNT,
        KITFOX_DATA_COUNTER,
        KITFOX_DATA_POWER,
        KITFOX_DATA_TDP,
        KITFOX_DATA_AMBIENT_TEMPERATURE,
        KITFOX_DATA_TEMPERATURE,
        KITFOX_DATA_THERMAL_GRID,
        KITFOX_DATA_DIMENSION,
        KITFOX_DATA_LENGTH,
        KITFOX_DATA_AREA,
        KITFOX_DATA_VOLUME,
        KITFOX_DATA_VOLTAGE,
        KITFOX_DATA_THRESHOLD_VOLTAGE,
        KITFOX_DATA_CLOCK_FREQUENCY,
        KITFOX_DATA_FAILURE_RATE,
        NUM_KITFOX_DATA_TYPES
    };
    
    static const char *KITFOX_DATA_STR[] = {
        "KITFOX_DATA_UNKNOWN",
        "KITFOX_DATA_INDEX",
        "KITFOX_DATA_TIME",
        "KITFOX_DATA_PERIOD",
        "KITFOX_DATA_COUNT",
        "KITFOX_DATA_COUNTER",
        "KITFOX_DATA_POWER",
        "KITFOX_DATA_TDP",
        "KITFOX_DATA_AMBIENT_TEMPERATURE",
        "KITFOX_DATA_TEMPERATURE",
        "KITFOX_DATA_THERMAL_GRID",
        "KITFOX_DATA_DIMENSION",
        "KITFOX_DATA_LENGTH",
        "KITFOX_DATA_AREA",
        "KITFOX_DATA_VOLUME",
        "KITFOX_DATA_VOLTAGE",
        "KITFOX_DATA_THRESHOLD_VOLTAGE",
        "KITFOX_DATA_CLOCK_FREQUENCY",
        "KITFOX_DATA_FAILURE_RATE",
        "NUM_KITFOX_DATA_TYPE"
    };
    
    enum ei_DATA_QUEUE_TYPE {
        KITFOX_QUEUE_DISCRETE = 0,
        KITFOX_QUEUE_OPEN
    };
    
    enum KITFOX_QUEUE_ERROR_TYPE {
        KITFOX_QUEUE_ERROR_NONE = 0,
        KITFOX_QUEUE_ERROR_TIME_DISCONTINUOUS,
        KITFOX_QUEUE_ERROR_TIME_OUTORDER,
        KITFOX_QUEUE_ERROR_TIME_OVERLAP,
        KITFOX_QUEUE_ERROR_TIME_INVALID,
        KITFOX_QUEUE_ERROR_DATA_DUPLICATED,
        KITFOX_QUEUE_ERROR_DATA_INVALID,
        KITFOX_QUEUE_ERROR_DATA_TYPE_INVALID,
        KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT,
        NUM_KITFOX_QUEUE_ERROR_TYPES
    };
    
    static const char *KITFOX_QUEUE_ERROR_STR[] = {
        "KITFOX_QUEUE_ERROR_NONE",
        "KITFOX_QUEUE_ERROR_TIME_DISCONTINUOUS",
        "KITFOX_QUEUE_ERROR_TIME_OUTORDER",
        "KITFOX_QUEUE_ERROR_TIME_OVERLAP",
        "KITFOX_QUEUE_ERROR_TIME_INVALID",
        "KITFOX_QUEUE_ERROR_DATA_DUPLICATED",
        "KITFOX_QUEUE_ERROR_DATA_INVALID",
        "KITFOX_QUEUE_ERROR_DATA_TYPE_INVALID",
        "KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT",
        "NUM_KITFOX_QUEUE_ERROR_TYPES"
    };

    enum KITFOX_TEMPERATURE_MAPPING_TYPE {
        KITFOX_TEMPERATURE_MAPPING_UNKNOWN = 0,
        KITFOX_TEMPERATURE_MAPPING_MAX,
        KITFOX_TEMPERATURE_MAPPING_MIN,
        KITFOX_TEMPERATURE_MAPPING_AVERAGE,
        KITFOX_TEMPERATURE_MAPPING_CENTER,
        NUM_KITFOX_TEMPERATURE_MAPPING_TYPES
    };

    static const char *KITFOX_TEMPERATURE_MAPPING_STR[] = {
        "KITFOX_TEMPERATURE_MAPPING_UNKNOWN",
        "KITFOX_TEMPERATURE_MAPPING_MAX",
        "KITFOX_TEMPERATURE_MAPPING_MIN",
        "KITFOX_TEMPERATURE_MAPPING_AVERAGE",
        "KITFOX_TEMPERATURE_MAPPING_CENTER",
        "NUM_KITFOX_TEMPERATURE_MAPPING_TYPES"
    };

    /*
     Overspecified architectural counters for observed
     modules which library models can define and estimate.
     The use of counters may differ depending on library
     models.
     */
    class counter_t
    {
    public:
        counter_t() { clear(); }
        ~counter_t() {}
        
        /*
         Basic switching activity count that may be used
         to represent logics, interconnects, etc.
         */
        Count switching;
        /*
         Read and write access counts.
         For caches, these counts implicitly include tag
         array accesses.
         For fully associative caches or CAMs, these counts
         include implicit search accesses.
         */
        Count read;
        Count write;
        /*
         Tag-array only access counts.
         For caches, note that both read and write misses are
         read_tag, and write_tag is only used for some special
         units in the core but never with regular caches.
         Therefore, a cache miss is actually represented with
         multiple counter increments (i.e., read_tag, write, etc.),
         which is the correct implementation for cycle-accurate
         architecture simulations.
         */
        Count read_tag;
        Count write_tag;
        /*
         For fully associative caches or CAMs, the search count is
         used to represent the failure of searching an array line;
         miss. Otherwise, the search activity is implicitly included
         in the read or write count.
         */
        Count search;
        /*
         DRAM counts.
         */
         Count precharge;
         Count refresh;
         Count background_open_page_high;
         Count background_open_page_low;
         Count background_closed_page_high;
         Count background_closed_page_low;
        
        void clear()
        {
            switching = 0;
            read = 0;
            write = 0;
            read_tag = 0;
            write_tag = 0;
            search = 0;
            precharge = 0;
            refresh = 0;
            background_open_page_high = 0;
            background_open_page_low = 0;
            background_closed_page_high = 0;
            background_closed_page_low = 0;
        }
        void operator=(const counter_t &c)
        {
            switching = c.switching;
            read = c.read;
            write = c.write;
            read_tag = c.read_tag;
            write_tag = c.write_tag;
            search = c.search;
            precharge = c.precharge;
            refresh = c.refresh;
            background_open_page_high = c.background_open_page_high;
            background_open_page_low = c.background_open_page_low;
            background_closed_page_high = c.background_closed_page_high;
            background_closed_page_low = c.background_closed_page_low;
        }
        const counter_t operator+(const counter_t &c)
        {
            counter_t cnt;
            cnt.switching = switching + c.switching;
            cnt.read = read + c.read;
            cnt.write = write + c.write;
            cnt.read_tag = read_tag + c.read_tag;
            cnt.write_tag = write_tag + c.write_tag;
            cnt.search = search + c.search;
            cnt.precharge = precharge + c.precharge;
            cnt.refresh = refresh + c.refresh;
            cnt.background_open_page_high = background_open_page_high + c.background_open_page_high;
            cnt.background_open_page_low = background_open_page_low + c.background_open_page_low;
            cnt.background_closed_page_high = background_closed_page_high + c.background_closed_page_high;
            cnt.background_closed_page_low = background_closed_page_low + c.background_closed_page_low;
            return cnt;
        }
        const counter_t operator-(const counter_t &c)
        {
            counter_t cnt;
            cnt.switching = switching - c.switching;
            cnt.read = read - c.read;
            cnt.write = write - c.write;
            cnt.read_tag = read_tag - c.read_tag;
            cnt.write_tag = write_tag - c.write_tag;
            cnt.search = search - c.search;
            cnt.precharge = precharge - c.precharge;
            cnt.refresh = refresh - c.refresh;
            cnt.background_open_page_high = background_open_page_high - c.background_open_page_high;
            cnt.background_open_page_low = background_open_page_low - c.background_open_page_low;
            cnt.background_closed_page_high = background_closed_page_high - c.background_closed_page_high;
            cnt.background_closed_page_low = background_closed_page_low - c.background_closed_page_low;
            return cnt;
        }
        
        const counter_t operator*(const Count &c)
        {
            counter_t cnt;
            cnt.switching = switching*c;
            cnt.read = read*c;
            cnt.write = write*c;
            cnt.read_tag = read_tag*c;
            cnt.write_tag = write_tag*c;
            cnt.search = search*c;
            cnt.precharge = precharge*c;
            cnt.refresh = refresh*c;
            cnt.background_open_page_high = background_open_page_high*c;
            cnt.background_open_page_low = background_open_page_low*c;
            cnt.background_closed_page_high = background_closed_page_high*c;
            cnt.background_closed_page_low = background_closed_page_low*c;
            return cnt;
        }
        
        const counter_t operator/(const Count &c)
        {
            counter_t cnt;
            cnt.switching = switching/c;
            cnt.read = read/c;
            cnt.write = write/c;
            cnt.read_tag = read_tag/c;
            cnt.write_tag = write_tag/c;
            cnt.search = search/c;
            cnt.precharge = precharge/c;
            cnt.refresh = refresh/c;
            cnt.background_open_page_high = background_open_page_high/c;
            cnt.background_open_page_low = background_open_page_low/c;
            cnt.background_closed_page_high = background_closed_page_high/c;
            cnt.background_closed_page_low = background_closed_page_low/c;
            return cnt;
        }
    };
    
    /*
     Per-access energy dissipation with respect to
     architectural access count type. This assumes an access
     type always dissipates the same amount of energy regardless
     of architectural states. The manipulation of unit energy
     may differ depending on library models.
     */
    class unit_energy_t
    {
    public:
        unit_energy_t() { clear(); }
        ~unit_energy_t() {}
        
        Joule baseline; // Baseline offset energy dissipation
        Joule switching; // Energy dissipation of a switching activity
        Joule read; // Energy dissipation of a read access
        Joule write; // Energy dissipation of a write access
        Joule read_tag; // Energy dissipation of a tag read access
        Joule write_tag; // Energy dissipation of a tag write access
        Joule search; // Energy dissipation of a search miss
        Joule precharge; // Energy dissipation of DRAM precharge
        Joule refresh; // Energy dissipation of DRAM refresh
        Joule background_open_page_high; // Background energy dissipation of open DRAM bank (CLK high)
        Joule background_open_page_low; // Background energy dissipation of open DRAM bank (CLK low)
        Joule background_closed_page_high; // Background energy dissipation of closed DRAM bank
        Joule background_closed_page_low; // Background energy dissipation of closed DRAM bank
        Joule leakage; // Per-clock leakage energy dissipation
        
        void clear()
        {
            baseline = 0.0;
            switching = 0.0;
            read = 0.0;
            write = 0.0;
            read_tag = 0.0;
            write_tag = 0.0;
            search = 0.0;
            precharge = 0.0;
            refresh = 0.0;
            background_open_page_high = 0.0;
            background_open_page_low = 0.0;
            background_closed_page_high = 0.0;
            background_closed_page_low = 0.0;
            leakage = 0.0;
        }
        void operator=(const unit_energy_t &e)
        {
            baseline = e.baseline;
            switching = e.switching;
            read = e.read;
            write = e.write;
            read_tag = e.read_tag;
            write_tag = e.write_tag;
            search = e.search;
            precharge = e.precharge;
            refresh = e.refresh;
            background_open_page_high = e.background_open_page_high;
            background_open_page_low = e.background_open_page_low;
            background_closed_page_high = e.background_closed_page_high;
            background_closed_page_low = e.background_closed_page_low;
            leakage = e.leakage;
        }
        const unit_energy_t operator+(const unit_energy_t &e)
        {
            unit_energy_t energy;
            energy.baseline = baseline + e.baseline;
            energy.switching = switching + e.switching;
            energy.read = read + e.read;
            energy.write = write + e.write;
            energy.read_tag = read_tag + e.read_tag;
            energy.write_tag = write_tag + e.write_tag;
            energy.search = search + e.search;
            energy.precharge = precharge + e.precharge;
            energy.refresh = refresh + e.refresh;
            energy.background_open_page_high = background_open_page_high + e.background_open_page_high;
            energy.background_open_page_low = background_open_page_low + e.background_open_page_low;
            energy.background_closed_page_high = background_closed_page_high + e.background_closed_page_high;
            energy.background_closed_page_low = background_closed_page_low + e.background_closed_page_low;
            energy.leakage = leakage + e.leakage;
            return energy;
        }
        const unit_energy_t operator-(const unit_energy_t &e)
        {
            unit_energy_t energy;
            energy.baseline = baseline - e.baseline;
            energy.switching = switching - e.switching;
            energy.read = read - e.read;
            energy.write = write - e.write;
            energy.read_tag = read_tag - e.read_tag;
            energy.write_tag = write_tag - e.write_tag;
            energy.search = search - e.search;
            energy.precharge = precharge - e.precharge;
            energy.refresh = refresh - e.refresh;
            energy.background_open_page_high = background_open_page_high - e.background_open_page_high;
            energy.background_open_page_low = background_open_page_low - e.background_open_page_low;
            energy.background_closed_page_high = background_closed_page_high - e.background_closed_page_high;
            energy.background_closed_page_low = background_closed_page_low - e.background_closed_page_low;
            energy.leakage = leakage - e.leakage;
            return energy;
        }
    };
    
    /*
     Generic energy definition, independent of architectural
     access counters.
     */
    class energy_t
    {
    public:
        energy_t() { clear(); }
        ~energy_t() {}
        
        Joule total; // Total energy dissipation
        Joule dynamic; // Dynamic energy dissipation
        Joule leakage; // Leakage energy dissipation
        
        void clear()
        {
            total = 0.0;
            dynamic = 0.0;
            leakage = 0.0;
        }
        const Joule get_total()
        {
            if(total == 0.0)
                total = dynamic+leakage;
            return total;
        }
        void operator=(const energy_t &e)
        {
            total = e.total;
            dynamic = e.dynamic;
            leakage = e.leakage;
        }
        const energy_t operator+(const energy_t &e)
        {
            energy_t energy;
            energy.total = total + e.total;
            energy.dynamic = dynamic + e.dynamic;
            energy.leakage = leakage + e.leakage;
            return energy;
        }
        const energy_t operator-(const energy_t &e)
        {
            energy_t energy;
            energy.total = total - e.total;
            energy.dynamic = dynamic - e.dynamic;
            energy.leakage = leakage - e.leakage;
            return energy;
        }
    };
    
    /*
     Generic power definition, independent of architectural
     access counters.
     */
    class power_t
    {
    public:
        power_t() { clear(); }
        ~power_t() {}
        
        Watt total; // Total power dissipation
        Watt dynamic; // Dynamic power dissipation
        Watt leakage; // Leakage power dissipation
        
        void clear()
        {
            total = 0.0;
            dynamic = 0.0;
            leakage = 0.0;
        }
        const Watt get_total()
        {
            if(total == 0.0)
                total = dynamic+leakage;
            return total;
        }
        void operator=(const power_t &p)
        {
            total = p.total;
            dynamic = p.dynamic;
            leakage = p.leakage;
        }
        const power_t operator+(const power_t &p)
        {
            power_t power;
            power.total = total + p.total;
            power.dynamic = dynamic + p.dynamic;
            power.leakage = leakage + p.leakage;
            return power;
        }
        const power_t operator-(const power_t &p)
        {
            power_t power;
            power.total = total - p.total;
            power.dynamic = dynamic - p.dynamic;
            power.leakage = leakage - p.leakage;
            return power;
        }
    };
    
    /*
     Location and size information
     */
    class dimension_t
    {
    public:
        dimension_t() { clear(); }
        ~dimension_t() {}
        
        Index die_index; // die index if model indexes the die by number
        char die_name[MAX_KITFOX_COMP_NAME_LENGTH]; // die name if model indexes the die by name
        Meter left, bottom; // oriented at left-bottom corner
        Meter width, height; // x, y dim length
        MeterSquare area; // area
        
        const MeterSquare get_area()
        {
            if(area == 0.0) return (width*height);
            return area;
        }
        const Meter get_center_x() const
        {
            return left+width/2.0;
        }
        const Meter get_center_y() const
        {
            return bottom+height/2.0;
        }
        const Meter get_left() const
        {
            return left;
        }
        const Meter get_right() const
        {
            return left+width;
        }
        const Meter get_bottom() const
        {
            return bottom;
        }
        const Meter get_top() const
        {
            return bottom+height;
        }
        void clear()
        {
            die_index = 0;
            memset(die_name,0,MAX_KITFOX_COMP_NAME_LENGTH);
            left = 0.0;
            bottom = 0.0;
            width = 0.0;
            height = 0.0;
            area = 0.0;
        }
        void operator=(const dimension_t &d)
        {
            die_index = d.die_index;
            memcpy(die_name,d.die_name,MAX_KITFOX_COMP_NAME_LENGTH);
            left = d.left; bottom = d.bottom;
            width = d.width; height = d.height;
            area = d.area;
        }
    };
    
    template <typename T>
    class grid_t
    {
    public:
        grid_t() :
        grid_cell_width(0.0), grid_cell_height(0.0),
        x(0), y(0), z(0), count(0)
        {
        }
        grid_t(const grid_t<T> &g) :
        x(g.x), y(g.y), z(g.z), count(g.count)
        {
            element.reserve(count);
            copy(&g.element[0], &g.element[count-1], element.begin());
            //memcpy(&element[0], &g.element[0], count*sizeof(T));
        }
        grid_t(Meter CellWidth, Meter CellHeight, unsigned c, unsigned r, unsigned l) :
        grid_cell_width(CellWidth), grid_cell_height(CellHeight),
        x(c), y(r), z(l)
        {
            count = x*y*z;
            element.reserve(count);
        }
        ~grid_t()
        {
            element.clear();
        }
        
        const unsigned int size() const
        {
            if(count == 0)
                count = x*y*z;
            return count;
        }
        const unsigned int cols() const
        {
            return x;
        }
        const unsigned int rows() const
        {
            return y;
        }
        const unsigned int dies() const
        {
            return z;
        }
        grid_t<T>& operator=(const grid_t<T> &g)
        {
            x = g.x; y = g.y; z = g.z;
            count = g.count;
            element.reserve(count);
            copy(&g.element[0], &g.element[count-1], element.begin());
            //memcpy(&element[0], &g.element[0], count*sizeof(T));
            return *this;
        }
        T& operator[](unsigned int i)
        {
            return &element[i];
        }
        T& operator()(unsigned c, unsigned r, unsigned l)
        {
            return element[c +r*x +l*x*y];
        }
        void reserve(unsigned int c, unsigned int r = 1, unsigned int l = 1)
        {
            clear();
            x = c, y = r, z = l;
            count = x*y*z;
            element.reserve(count);
        }
        void clear()
        {
            element.clear();
        }
        
        Meter grid_cell_width, grid_cell_height;
        
    private:
        unsigned int x, y, z;
        unsigned int count;
        std::vector<T> element;
    };
    
    // power = energy * clock frequency
    inline power_t operator*(const energy_t &e, const Hertz &freq)
    {
        power_t power;
        power.dynamic = e.dynamic*freq;
        power.leakage = e.leakage*freq;
        power.total = power.dynamic + power.leakage;
        return power;
    }
    
    // power = energy / time
    inline power_t operator/(const energy_t &e, const Second &s)
    {
        power_t power;
        power.dynamic = e.dynamic / s;
        power.leakage = e.leakage / s;
        power.total = power.dynamic + power.leakage;
        return power;
    }
    
    // energy = power / clock frequency
    inline energy_t operator/(const power_t &p, const Hertz &freq)
    {
        energy_t energy;
        energy.dynamic = p.dynamic / freq;
        energy.leakage = p.leakage / freq;
        energy.total = energy.dynamic + energy.leakage;
        return energy;
    }
    
    // energy = power * time
    inline energy_t operator*(const power_t &p, const Second &s)
    {
        energy_t energy;
        energy.dynamic = p.dynamic * s;
        energy.leakage = p.leakage * s;
        energy.total = energy.dynamic + energy.leakage;
        return energy;
    }
    
    static const int KITFOX_DATA_SIZE[] = {
        0, // KITFOX_DATA_UNKNOWN
        sizeof(Index), // KITFOX_DATA_INDEX
        sizeof(Second), // KITFOX_DATA_TIME
        sizeof(Second), // KITFOX_DATA_PERIOD
        sizeof(Count), // KITFOX_DATA_COUNT
        sizeof(counter_t), // KITFOX_DATA_COUNTER
        sizeof(power_t), // KITFOX_DATA_POWER
        sizeof(power_t), // KITFOX_DATA_TDP
        sizeof(Kelvin), // KITFOX_DATA_AMBIENT_TEMPERATURE
        sizeof(Kelvin), // KITFOX_DATA_TEMPERATURE
        0, // KITFOX_DATA_THERMAL_GRID
        sizeof(dimension_t), // KITFOX_DATA_DIMENSION
        sizeof(Meter), // KITFOX_DATA_LENGTH
        sizeof(MeterSquare), // KITFOX_DATA_AREA
        sizeof(MeterCube), // KITFOX_DATA_VOLUME
        sizeof(Volt), // KITFOX_DATA_VOLTAGE
        sizeof(Volt), // KITFOX_DATA_THRESHOLD_VOLTAGE
        sizeof(Hertz), // KITFOX_DATA_CLOCK_FREQUENCY
        sizeof(Unitless), // KITFOX_DATA_FAILURE_RATE
        0 // NUM_KITFOX_DATA_TYPES
    };
    
} // namespace libKitFox

#endif
