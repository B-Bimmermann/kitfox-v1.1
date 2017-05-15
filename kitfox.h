#ifndef __KITFOX_H__
#define __KITFOX_H__

#include <vector>
#include <map>
#include <set>
#include <pthread.h>
#include <mpi.h>
#include <stdarg.h>
#include <libconfig.h++>
#include <sys/time.h>
#include "kitfox-defs.h"
#include "component.h"
#include "configuration.h"
#include "communicator/kitfox-server.h"

namespace libKitFox
{
//class configuration_t;
//class model_library_t;
//class pseudo_component_t;
//class server_t;

class kitfox_t
{
public:
    kitfox_t(MPI_Comm *KitFoxComm = NULL, MPI_Comm *InterComm = NULL);
    ~kitfox_t();
    
    // Create a pseudo component based on libconfig::Setting
    pseudo_component_t* create_pseudo_component(libconfig::Setting &ConfigSetting);
    // Initialized a pseudo component
    void init_pseudo_component(pseudo_component_t *PseudoComponent);
    
    // Connect KitFox servers across MPI ranks.
    void connect();
    // Create and initialize pseudo components.
    void configure(const char *ConfigFile);
    // Start server listen loop.
    void listen();
    
    // This function is called only when KitFox servers try to connect pseudo components across MPI ranks.
    const Comp_ID connect_remote_component(Comp_ID ComponentID, std::string ComponentName);
    // Find a component with the name and return its component ID.
    // INVALID_Comp_ID(=-1) is returned if a component is not found.
    const Comp_ID get_component_id(std::string ComponentName, bool UseServer = true);
    // Find a component with the ID and return its name.
    std::string get_component_name(Comp_ID ComponentID, bool UseServer = true);
    
    // Synchronize data among pseudo copmonents.
    const int synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, bool UseServer = true);
    //const int synchronize_data(Second Time, Second Period, int DataType, bool SyncGlobal = true);
    template <typename T> const int synchronize_and_pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data);
    template <typename T> const int push_and_synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data);
    template <typename T> const int overwrite_and_synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data);
    
    // Pulls data from a pseudo component queue.
    template <typename T> const int pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer = true);
    template <typename T> const int pull_data(Comp_ID ComponentID, Second Time, int DataType, T *Data, bool UseServer = true);
    
    // Pushes data into a pseudo component queue.
    template <typename T> const int push_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer = true);
    template <typename T> const int push_data(Comp_ID ComponentID, Second Time, int DataType, T *Data, bool UseServer = true);
    
    // Overwrites data in a pseudo component queue.
    template <typename T> const int overwrite_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer = true);
    template <typename T> const int overwrite_data(Comp_ID ComponentID, Second time, int DataType, T *Data, bool UseServer = true);
    
    // Computational functions
    void calculate_power(Comp_ID ComponentID, Second Time, Second Period, counter_t Counter, bool IsTDP = false, bool UseServer = false);
    void calculate_temperature(Comp_ID ComponentID, Second Time, Second Period, bool DoPowerSync = true, bool UseServer = false);
    void calculate_failure_rate(Comp_ID ComponentID, Second Time, Second Period, bool DoDataSync = true, bool UseServer = false);
    
    bool update_library_variable(Comp_ID ComponentID, int VariableType, void *Value, bool IsLibraryVariable = false, bool UseServer = false, int VariableSize = 0, bool Wait = true);
    
    // Error function
    void error(const char *str1, const char *str2, ...);
    // Warning function
    void warning(const char *str1, const char *str2, ...);
    
    int rank; // KitFox MPI rank
    // Start and end component IDs that can be assigned to pseudo components by this KitFox 
    std::pair<Comp_ID,Comp_ID> component_id_range;

    // Cast and return libraries
    energy_library_t* get_energy_library(Comp_ID ComponentID);
    thermal_library_t* get_thermal_library(Comp_ID ComponentID);
    reliability_library_t* get_reliability_library(Comp_ID ComponentID);
    
private:
    // Read config file.
    void read_config(const char *ConfigFile);
    // A new component ID is kitfox_t::id + component_count.
    const Comp_ID get_new_comp_id(std::string ComponentName);
    // Check if the pseudo component is a local component.
    const bool is_local_component(Comp_ID ComponentID);
    const bool is_local_component(std::string ComponentName);
    // Return a pointer to a pseudo component
    pseudo_component_t* get_pseudo_component(Comp_ID ComponentID);
    pseudo_component_t* get_pseudo_component(std::string ComponentName);
    
    configuration_t *configuration; // Configuration
    std::map<Comp_ID,pseudo_component_t*> component; // Component map
    std::vector<pseudo_component_t*> remote_component; // Remote component map
    std::vector<pseudo_component_t*> root_component;
    std::set<pseudo_component_t*> leaf_component;
    std::map<std::string,Comp_ID> name_ID_map; // Name-ID map
    Comp_ID component_count; // Created components counts
    
    // MPI Communicator between KitFox
    MPI_Comm *KITFOX_COMM;
    // MPI Inter-communicator between KitFox server and client simulators
    MPI_Comm *INTER_COMM;
    // KitFox server
    server_t *server;
    
    // pthread mutexes
    pthread_mutex_t mutex_pseudo_component;
};

// This function synchronizes the data of DataType and returns the data.
template <typename T>
const int kitfox_t::synchronize_and_pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data)
{
    int queue_error = synchronize_data(ComponentID,Time,Period,DataType);
    if(queue_error != KITFOX_QUEUE_ERROR_NONE) { return queue_error; }
    queue_error = pull_data(ComponentID,Time,Period,DataType,Data);
    return queue_error;
}

// This function inserts the data of DataType to the queue of the pseudo component with ComponentID and synchronize the data across other linked pseudo components.
template <typename T>
const int kitfox_t::push_and_synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data)
{
    int queue_error = push_data(ComponentID,Time,Period,DataType,Data);
    if(queue_error != KITFOX_QUEUE_ERROR_NONE) { return queue_error; }
    queue_error = synchronize_data(ComponentID,Time,Period,DataType);
    return queue_error;
}

// This function overwrites the data of DataType to the queue of the pseudo component with ComponentID and synchronize the data across other linked pseudo components.
template <typename T>
const int kitfox_t::overwrite_and_synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data)
{
    int queue_error = overwrite_data(ComponentID,Time,Period,DataType,Data);
    if(queue_error != KITFOX_QUEUE_ERROR_NONE) { return queue_error; }
    queue_error = synchronize_data(ComponentID,Time,Period,DataType);
    return queue_error;
}

// This functions finds a pseudo component with ComponentID and returns its data that matches both "Time" and "Period".
template <typename T>
const int kitfox_t::pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer)
{
    int queue_error = KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT;
    if(component.count(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        (*Data) = pseudo_component->queue.get<T>(Time,Period,DataType);
        queue_error = pseudo_component->queue.get_error(DataType);
    }
    else if(UseServer&&server) {
        queue_error = server->pull_data(ComponentID,Time,Period,DataType,Data);
    }
    return queue_error;
}

// This functions finds a pseudo component with ComponentID and returns its data that matches only "Time".
template <typename T>
const int kitfox_t::pull_data(Comp_ID ComponentID, Second Time, int DataType, T *Data, bool UseServer)
{
    return pull_data<T>(ComponentID,Time,UNSPECIFIED_TIME,DataType,Data,UseServer);
}

// This functions finds a pseudo component with ComponentID and inserts a new data associated with "Time" and "Period".
template <typename T>
const int kitfox_t::push_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer)
{
    int queue_error = KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT;
    if(component.count(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        pseudo_component->queue.push_back<T>(Time,Period,DataType,*Data);
        queue_error = pseudo_component->queue.get_error(DataType);
    }
    else if(UseServer&&server) {
        queue_error = server->push_data(ComponentID,Time,Period,DataType,Data);
    }
    return queue_error;
}

// This functions finds a pseudo component with ComponentID and inserts a new data associated with only "Time".
template <typename T>
const int kitfox_t::push_data(Comp_ID ComponentID, Second Time, int DataType, T *Data, bool UseServer)
{
    return push_data<T>(ComponentID,Time,UNSPECIFIED_TIME,DataType,Data,UseServer);
}

// This function finds a pseudo component with ComponentID and replaces the data that matches both "Time" and "Period".
template <typename T>
const int kitfox_t::overwrite_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, T *Data, bool UseServer)
{
    int queue_error = KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT;
    if(component.count(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        pseudo_component->queue.overwrite<T>(Time,Period,DataType,*Data);
        queue_error = pseudo_component->queue.get_error(DataType);
    }
    else if(UseServer&&server) {
        queue_error = server->overwrite_data(ComponentID,Time,Period,DataType,Data);
    }
    return queue_error;
}

// This function finds a pseudo component with ComponentID and replaces the data that matches only "Time".
template <typename T>
const int kitfox_t::overwrite_data(Comp_ID ComponentID, Second Time, int DataType, T *Data, bool UseServer)
{
    return overwrite_data<T>(ComponentID,Time,UNSPECIFIED_TIME,DataType,Data,UseServer);
}

} // namespace KitFox
#endif


