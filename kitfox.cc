#include <iostream>
#include <cstdlib>
#include <string.h>
#include <mpi.h>
#include <libconfig.h++>
#include "kitfox.h"
#include "configuration.h"
#include "library.h"
#include "component.h"
#include "threader.h"
#include "communicator/kitfox-server.h"
#include "communicator/kitfox-client.h"


using namespace std;
using namespace libconfig;
using namespace libKitFox;


// Include header files of library models (if compiled)
#ifdef ENERGYLIB_DRAMSIM_I
#include "energylib_dramsim.h"
using namespace libKitFox_DRAMSIM;
#endif
#ifdef ENERGYLIB_DSENT_I
#include "energylib_dsent.h"
using namespace libKitFox_DSENT;
#endif
#ifdef ENERGYLIB_INTSIM_I
#include "energylib_intsim.h"
using namespace libKitFox_INTSIM;
#endif
#ifdef ENERGYLIB_MCPAT_I
#include "energylib_mcpat.h"
using namespace libKitFox_MCPAT;
#endif
#ifdef ENERGYLIB_TSV_I
#include "energylib_tsv.h"
using namespace libKitFox_TSV;
#endif
#ifdef THERMALLIB_3DICE_I
#include "thermallib_3dice.h"
using namespace libKitFox_3DICE;
#endif
#ifdef THERMALLIB_HOTSPOT_I
#include "thermallib_hotspot.h"
using namespace libKitFox_HOTSPOT;
#endif
#ifdef THERMALLIB_MICROFLUIDICS_I
#include "thermallib_microfluidics.h"
using namespace libKitFox_MICROFLUIDICS;
#endif
#ifdef RELIABILITYLIB_FAILURE_I
#include "reliabilitylib_failure.h"
using namespace libKitFox_FAILURE;
#endif


kitfox_t::kitfox_t(MPI_Comm *KitFoxComm, MPI_Comm *InterComm) :
    component_count(0), 
    server(NULL), 
    rank(0), 
    KITFOX_COMM(KitFoxComm), 
    INTER_COMM(InterComm)
{
    if(KITFOX_COMM&&INTER_COMM) { // MPI mode KitFox 
        MPI_Comm_rank(*KITFOX_COMM, &rank);
#ifdef KITFOX_MPI_DEBUG
        cout << "KitFox [LP=" << rank << "]: created" << endl;
#endif
    }
    else { // Non-MPI mode KitFox 
#ifdef KITFOX_DEBUG
        cout << "KitFox (non-MPI) is created" << endl;
#endif
    }
  
    pthread_mutex_init(&mutex_pseudo_component, NULL);
}


kitfox_t::~kitfox_t()
{
    while(component.size()) { // Remove local pseudo components.
        delete component.begin()->second;
        component.erase(component.begin());
    }
    while(remote_component.size()) { // Remove placeholder copies of remote pseudo components.
        delete (*remote_component.begin());
        remote_component.erase(remote_component.begin());
    }
    
    // Remove configuration and server.
    delete configuration;
    delete server;
    
    pthread_mutex_destroy(&mutex_pseudo_component);
}


// Error function
void kitfox_t::error(const char *str1, const char *str2, ...)
{
    char buf[1024];
	va_list args;
	va_start(args, str2);
	vsprintf(buf, str2, args);
	va_end(args);

    cout << "KITFOX ERROR ";
    if(str1) { cout << "(" << str1 << ") "; }
    if(KITFOX_COMM) { cout << "[LP=" << rank << "]"; }
    cout << ": " << buf << endl;
    exit(EXIT_FAILURE);
}


// Warning function
void kitfox_t::warning(const char *str1, const char *str2, ...)
{
    char buf[1024];
	va_list args;
	va_start(args, str2);
	vsprintf(buf, str2, args);
	va_end(args);
    
    cout << "KITFOX WARNING ";
    if(str1) { cout << "(" << str1 << ") "; }
    if(KITFOX_COMM) { cout << "[LP=" << rank << "]"; }
    cout << ": " << buf << endl;
}


// Read and parse config_file using libconfig. Actual KitFox initialization is processed with configure() function.
void kitfox_t::read_config(const char *ConfigFile)
{
    try {
        // Load "config_file".
        configuration = new configuration_t();
        configuration->readFile(ConfigFile);
    }
    catch(FileIOException e) {
        error("libconfig", "failed to read config file %s", ConfigFile);
    }
    catch(ParseException e) {
        error("libconfig", "failed to parse config file %s (line %d)", ConfigFile, e.getLine());
    }

    configuration->setAutoConvert(true); // Enable auto-conversion of data types.
  
    try {
        // The KitFox assigns IDs to pseudo components within a range of [start, end].
        // KitFox across multiple MPI ranks must have discrete range of pseudo component IDs.
        if(configuration->exists("component_id")) {
            component_id_range.first = configuration->lookup("component_id.start");
            component_id_range.second = configuration->lookup("component_id.end");

            if(component_id_range.first == INVALID_COMP_ID) {
                error("libconfig", "component_id.start must be greater than 0");
            }
            
            if(component_id_range.second <= component_id_range.first) {
                error("libconfig", "(component_id.end - component_id.start) must be greater than 0 in %s", configuration->getFileName().c_str());
            }
        }
        else {
            component_id_range.first = MAX_KITFOX_COMPONENTS*rank+1;
            component_id_range.second = MAX_KITFOX_COMPONENTS*(rank+1)-1;
        }
    }
    catch(SettingNotFoundException e) {
        error("libconfig", "%s is not defined in %s", e.getPath(), configuration->getFileName().c_str());
    }
    catch(SettingTypeException e) {
        error("libconfig", "%s has incorrect type in %s", e.getPath(), configuration->getFileName().c_str());
    }
}


// Fully connect KitFox across MPI ranks. It must be called only when the parallel-KitFox is used.
void kitfox_t::connect()
{
    if(!KITFOX_COMM) { error(NULL, "MPI communicator is not defined"); }
    
    // Create a server. The server is a contact point between KitFox as well as to client simulators.
    server = new server_t(KITFOX_COMM, INTER_COMM, this);
    
    // Barrier: All MPI ranks should have created servers.
    MPI_Barrier(*KITFOX_COMM);
    
    // Cross-connect KitFox servers across MPI ranks.
    int size; MPI_Comm_size(*KITFOX_COMM, &size);
    for(unsigned r = 0; r < size; r++) { server->connect_server(r, size); }
}


void *pthread_init_pseudo_component(void *p)
{
    pseudo_component_t *pseudo_component = (pseudo_component_t*)p;
    pseudo_component->kitfox->init_pseudo_component(pseudo_component);
    pthread_exit((void*)p);
}


// Create and initialize pseudo components.
void kitfox_t::configure(const char *ConfigFile)
{
    // Read and parse "config_file"
    read_config(ConfigFile); 

    // Barrier: All KitFox should have been connected with each other across MPI ranks.
    if(KITFOX_COMM) { MPI_Barrier(*KITFOX_COMM); }
  
    try {
        // KitFox must define pseudo components.
        Setting &component_list = configuration->lookup("component");
        
        // Scan the component list defined in the configuration, and create pseudo components.
        for(unsigned c = 0; c < component_list.getLength(); c++) {
            // Pseudo component IDs are sequentially assigned here.
            pseudo_component_t *pseudo_component = create_pseudo_component(component_list[c]);
            if(!pseudo_component->is_remote) { root_component.push_back(pseudo_component); }
        }
        
        // After all pseudo components are created, initialize their library models.
        threader_t threader(this);
        for(unsigned c = 0; c < component_list.getLength(); c++) {
            // Note that configuration->getCompPath() returns the correct pseudo component name delimited with dots,
            // whereas component_list[c].getPath() returns the path of the libconfig::Setting.
            string component_name = configuration->getCompPath(component_list[c]);
            
            // Initialize the local components.
            pseudo_component_t *pseudo_component = get_pseudo_component(component_name);
            if(pseudo_component == NULL) { error(NULL, "initializing pseudo component %s failed", component_name.c_str()); }
            
            // Initialize the pseudo component.
            if(pseudo_component->is_parallelizable) {
                threader.create(&pthread_init_pseudo_component, pseudo_component);
            }
            else {
                init_pseudo_component(pseudo_component);
            }
        }
        threader.join();
    }
    catch(SettingNotFoundException e) {
        error("libconfig", "%s is not defined in %s", e.getPath(), configuration->getFileName().c_str());
    }
    catch(SettingTypeException e) {
        error("libconfig", "%s has incorrect type in %s", e.getPath(), configuration->getFileName().c_str());
    }
    
    // KitFox is ready. Launch a server.
    if(server) { server->listen(); }
}


// Recursively create pseudo components in the architecture hierarchy tree from the top to bottom.
pseudo_component_t* kitfox_t::create_pseudo_component(Setting &ConfigSetting)
{
    pseudo_component_t *pseudo_component = NULL;
  
    try {
        if(!ConfigSetting.exists("remote")||!bool(ConfigSetting["remote"])) { // Local component
            string component_name = configuration->getCompPath(ConfigSetting);
            pseudo_component = new pseudo_component_t(get_new_comp_id(component_name), this, component_name, &ConfigSetting);
            
            // Create the manually defined queues.
            if(pseudo_component->exists("queue")) {
                Setting &queue_list = pseudo_component->lookup("queue");
                for(unsigned i = 0; i < queue_list.getLength(); i++) {
                    unsigned queue_size = pseudo_component->queue_size;
                    const char *queue_name_str;
                    if(queue_list.isGroup()) {
                        queue_name_str = queue_list[i].getName();
                        if(queue_list[i].exists("size")) queue_size = queue_list[i]["size"];
                    }
                    else {
                        assert(queue_list.isArray());
                        queue_name_str = (const char*)queue_list[i];
                    }
                    
                    if(!strcasecmp(queue_name_str, "KITFOX_DATA_COUNTER")) {
                        pseudo_component->queue.create<counter_t>(queue_size, KITFOX_DATA_COUNTER, KITFOX_QUEUE_DISCRETE, counter_t());
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_POWER")) {
                        pseudo_component->queue.create<power_t>(queue_size, KITFOX_DATA_POWER, KITFOX_QUEUE_DISCRETE, power_t());
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_TDP")) {
                        pseudo_component->queue.create<power_t>(queue_size, KITFOX_DATA_TDP, KITFOX_QUEUE_OPEN, power_t());
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_TEMPERATURE")) {
                        pseudo_component->queue.create<Kelvin>(queue_size, KITFOX_DATA_TEMPERATURE, KITFOX_QUEUE_DISCRETE, 0.0);
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_DIMENSION")) {
                        pseudo_component->queue.create<dimension_t>(queue_size, KITFOX_DATA_DIMENSION, KITFOX_QUEUE_OPEN, dimension_t());
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_VOLTAGE")) {
                        pseudo_component->queue.create<Volt>(queue_size, KITFOX_DATA_VOLTAGE, KITFOX_QUEUE_OPEN, 0.0);
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_THRESHOLD_VOLTAGE")) {
                        pseudo_component->queue.create<Volt>(queue_size, KITFOX_DATA_THRESHOLD_VOLTAGE, KITFOX_QUEUE_OPEN, 0.0);
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_CLOCK_FREQUENCY")) {
                        pseudo_component->queue.create<Hertz>(queue_size, KITFOX_DATA_CLOCK_FREQUENCY, KITFOX_QUEUE_OPEN, 0.0);
                    }
                    else if(!strcasecmp(queue_name_str, "KITFOX_DATA_FAILURE_RATE")) {
                        pseudo_component->queue.create<Unitless>(queue_size, KITFOX_DATA_FAILURE_RATE, KITFOX_QUEUE_DISCRETE, 0.0);
                    }
                    else { error(NULL, "unknown data type %s to create a data queue", queue_name_str); }
                }
            }
            
            // Create default data queues (if not manually created).
            // These are the key data that are most commonly used by various library models.
            // TODO: But, these are not always or not the only used by the models.
            
            // Discrete queues store time-sampled data. When a new queue entry is inserted at time "t" with period "p",
            // the data is treated valid during (t-p,t] time range as if it is a continous data.
            // Therefore, the discrete queue is CONTINUOUS TO THE LEFT.
            pseudo_component->queue.create<power_t>(pseudo_component->queue_size, KITFOX_DATA_POWER, KITFOX_QUEUE_DISCRETE, power_t());
            pseudo_component->queue.create<Kelvin>(pseudo_component->queue_size, KITFOX_DATA_TEMPERATURE, KITFOX_QUEUE_DISCRETE, 0.0);
            
            // Open queues store data that is valid for indefinite period of time. For example, voltage is not typically
            // sampled data like power but is constant until some control mechanism changes it. So, when a new entry is
            // inserted into the queue, it is unknown how long the value would be valid.
            // Therefore, the open queue is CONTINUOUS TO THE RIGHT.
            pseudo_component->queue.create<power_t>(pseudo_component->queue_size, KITFOX_DATA_TDP, KITFOX_QUEUE_OPEN, power_t());
            pseudo_component->queue.create<Volt>(pseudo_component->queue_size, KITFOX_DATA_VOLTAGE, KITFOX_QUEUE_OPEN, 0.0);
            pseudo_component->queue.create<Volt>(pseudo_component->queue_size, KITFOX_DATA_THRESHOLD_VOLTAGE, KITFOX_QUEUE_OPEN, 0.0);
            pseudo_component->queue.create<Hertz>(pseudo_component->queue_size, KITFOX_DATA_CLOCK_FREQUENCY, KITFOX_QUEUE_OPEN, 0.0);
            pseudo_component->queue.create<dimension_t>(pseudo_component->queue_size, KITFOX_DATA_DIMENSION, KITFOX_QUEUE_OPEN, dimension_t());
            
            // Store a pseudo component.
            component.insert(pair<Comp_ID, pseudo_component_t*>(pseudo_component->id, pseudo_component));
            
#ifdef KITFOX_DEBUG
            cout << "KITFOX DEBUG ";
            if(KITFOX_COMM) cout << "[LP=" << rank << "]";
            cout << ": pseudo component " << pseudo_component->name << " [ID="
            << pseudo_component->id << "] is created" << endl;
#endif            
        }
        else { // Remote component
            // This pseudo component is merely a placeholder.
            // The placeholder only tells if the actual pseudo component is remotely located and what the Comp_ID is, 
            // and it doesn't do any functional work in this MPI's KitFox.
            string component_name = configuration->getCompPath(ConfigSetting);
            pseudo_component = new pseudo_component_t(INVALID_COMP_ID, this, component_name, &ConfigSetting, /*remote*/true);
            remote_component.push_back(pseudo_component);                        
        }
        
        leaf_component.insert(pseudo_component);
        
        // Continue creating pseudo components down the tree
        if(ConfigSetting.exists("component")) {
            Setting &component_list = ConfigSetting["component"];
            if(component_list.getLength()) { leaf_component.erase(pseudo_component); }
            for(unsigned c = 0; c < component_list.getLength(); c++) {
                pseudo_component_t *subset_component = create_pseudo_component(component_list[c]);
                // If pseudo_component is local and subset_component is remote, use server to find remote subset_component ID.
                // If pseudo_component is remote and subset_component is local, other KitFox will visit to ask subset_component ID.
                if(!pseudo_component->is_remote&&subset_component->is_remote) {
                    if(!server) { error(NULL, "no KitFox-MPI server is created to connect remote pseudo component"); }
                    
                    string subset_path = configuration->getCompPath(component_list[c]);
                    Comp_ID remote_Comp_ID = server->connect_remote_component(pseudo_component->id, subset_path);
                    
                    if(remote_Comp_ID == INVALID_COMP_ID) { error(NULL, "no pseudo component %s is found in remote KitFox", subset_path.c_str()); }
                    subset_component->id = remote_Comp_ID;
                }
                pseudo_component->add_subset(subset_component);
            }
        }
    }
    catch(SettingNotFoundException e) {
        error("libconfig", "%s is not defined in %s", e.getPath(), configuration->getFileName().c_str());
    }
    catch(SettingTypeException e) {
        error("libconfig", "%s has incorrect type in %s", e.getPath(), configuration->getFileName().c_str());
    }
  
    return pseudo_component;
}


// Recursively initialize pseudo components from the bottom to top.
void kitfox_t::init_pseudo_component(pseudo_component_t *PseudoComponent)
{
    // Initialize subset components first.
    threader_t threader(this);
    for(unsigned c = 0; c < PseudoComponent->get_subset_count(); c++) {
        pseudo_component_t *subset_component = PseudoComponent->get_subset(c);

        if(subset_component->is_parallelizable) {
            threader.create(&pthread_init_pseudo_component, subset_component);
        }
        else {
            init_pseudo_component(subset_component);
        }
    }
    threader.join();

    // No need to initialize a remote component.
    if(PseudoComponent->is_remote) { return; }
    
#ifdef KITFOX_DEBUG
    cout << "KITFOX DEBUG ";
    if(KITFOX_COMM) cout << "[LP=" << rank << "]";
    cout << ": pseudo component " << PseudoComponent->name << " [ID="
    << PseudoComponent->id << "] initialization: " << endl;
#endif
    
    try {
        // Initialize library model.
        if(PseudoComponent->exists("library")) {
#ifdef KITFOX_DEBUG
            cout << "\t\tlibrary = ";
#endif
            const char *model_name_str = PseudoComponent->lookup_in_library("model");
            
            if(!strcasecmp(model_name_str, "none")||!strcasecmp(model_name_str, "n/a")) {
                ; // Nothing to do
#ifdef KITFOX_DEBUG
                cout << "none" << endl;
#endif
            }
#ifdef ENERGYLIB_DRAMSIM_I
            else if(!strcasecmp(model_name_str, "dramsim")) {
                PseudoComponent->model_library = new energylib_dramsim(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "energy_library (dramsim)" << endl;
#endif
            }
#endif
#ifdef ENERGYLIB_DSENT_I
            else if(!strcasecmp(model_name_str, "dsent")) {
                PseudoComponent->model_library = new energylib_dsent(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "energy_library (dsent)" << endl;
#endif
            }
#endif
#ifdef ENERGYLIB_INTSIM_I
            else if(!strcasecmp(model_name_str, "intsim")) {
                PseudoComponent->model_library = new energylib_intsim(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "energy_library (intsim)" << endl;
#endif
            }
#endif
#ifdef ENERGYLIB_MCPAT_I
            else if(!strcasecmp(model_name_str, "mcpat")) {
                PseudoComponent->model_library = new energylib_mcpat(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "energy_library (mcpat)" << endl;
#endif
            }
#endif
#ifdef ENERGYLIB_TSV_I
            else if(!strcasecmp(model_name_str, "tsv")) {
                PseudoComponent->model_library = new energylib_tsv(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "energy_library (tsv)" << endl;
#endif
            }
#endif
#ifdef THERMALLIB_3DICE_I
            else if(!strcasecmp(model_name_str, "3d-ice")||!strcasecmp(model_name_str, "3dice")) {
                PseudoComponent->model_library = new thermallib_3dice(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "thermal_library (3d-ice)" << endl;
#endif
            }
#endif
#ifdef THERMALLIB_HOTSPOT_I
            else if(!strcasecmp(model_name_str, "hotspot")) {
                PseudoComponent->model_library = new thermallib_hotspot(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "thermal_library (hotspot)" << endl;
#endif
            }
#endif
#ifdef THERMALLIB_MICROFLUIDICS_I
            else if(!strcasecmp(model_name_str, "microfluidics")) {
                PseudoComponent->model_library = new thermallib_microfluidics(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "thermal_library (microfluidics)" << endl;
#endif
            }
#endif
#ifdef RELIABILITYLIB_FAILURE_I
            else if(!strcasecmp(model_name_str, "failure")) {
                PseudoComponent->model_library = new reliabilitylib_failure(PseudoComponent);
#ifdef KITFOX_DEBUG
                cout << "reliability_library (failure) " << endl;
#endif
            }
#endif
            else {
                error("libconfig", "%s.library.model %s is unknown", PseudoComponent->name.c_str(), model_name_str);
            }
        }
        
        // Initialize the default queues.
        // KITFOX_DATA_TEMPERATURE
        if(PseudoComponent->exists_in_library("temperature", /*recursive lookup?*/true)) {
            Kelvin temperature = PseudoComponent->lookup_in_library("temperature", true);
            PseudoComponent->queue.overwrite<Kelvin>(0.0, MAX_TIME, KITFOX_DATA_TEMPERATURE, temperature);
        }
        
        // KITFOX_DATA_TDP
        synchronize_data(PseudoComponent->id, 0.0, MAX_TIME, KITFOX_DATA_TDP);
        
        // KITFOX_DATA_VOLTAGE
        if(PseudoComponent->exists_in_library("voltage", true)) {
            Volt voltage = PseudoComponent->lookup_in_library("voltage", true);
            PseudoComponent->queue.overwrite<Volt>(0.0, MAX_TIME, KITFOX_DATA_VOLTAGE, voltage);
        }

        // KITFOX_DATA_THRESHOLD_VOLTAGE
        if(PseudoComponent->exists_in_library("threshold_voltage", true)) {
            Volt threshold_voltage = PseudoComponent->lookup_in_library("threshold_voltage", true);
            PseudoComponent->queue.overwrite<Volt>(0.0, MAX_TIME, KITFOX_DATA_THRESHOLD_VOLTAGE, threshold_voltage);
        }

        // KITFOX_DATA_CLOCK_FREQUENCY
        if(PseudoComponent->exists_in_library("clock_frequency", true)) {
            Hertz clock_frequency = PseudoComponent->lookup_in_library("clock_frequency", true);
            PseudoComponent->queue.overwrite<Hertz>(0.0, MAX_TIME, KITFOX_DATA_CLOCK_FREQUENCY, clock_frequency);
        }
        
        // KITFOX_DATA_DIMENSION
        if(PseudoComponent->exists_in_library("dimension")) {
            dimension_t dimension;
            Setting &dimension_setting = PseudoComponent->lookup_in_library("dimension");
            dimension.left = dimension_setting["left"];
            dimension.bottom = dimension_setting["bottom"];
            dimension.width = dimension_setting["width"];
            dimension.height = dimension_setting["height"];
            if(dimension_setting.exists("die_index")) { dimension.die_index = dimension_setting["die_index"]; }
            else { dimension.die_index = 0; }
            if(dimension_setting.exists("die_name")) { strcpy(dimension.die_name, (const char*)dimension_setting["die_name"]); }
            PseudoComponent->queue.overwrite<dimension_t>(0.0, MAX_TIME, KITFOX_DATA_DIMENSION, dimension);
        }
        synchronize_data(PseudoComponent->id, 0.0, MAX_TIME, KITFOX_DATA_DIMENSION);
        
        // Initial calculation depending on library types
        if(PseudoComponent->model_library) {
#ifdef KITFOX_DEBUG
            if(temperature > 0.0) cout << "\t\ttemperature = " << temperature << " K" << endl;
            if(voltage > 0.0) cout << "\t\tvoltage = " << voltage << " V" << endl;
            if(threshold_voltage > 0.0) cout << "\t\tthreshold_voltage = " << " V" << endl;
            if(clock_frequency > 0.0) cout << "\t\tclock_frequency = " << clock_frequency << " Hz" << endl;
#endif
            switch(PseudoComponent->model_library->type) {
                case KITFOX_LIBRARY_ENERGY_MODEL: {
                    energy_library_t *energy_library = dynamic_cast<energy_library_t*>(PseudoComponent->model_library);
                    
                    // Additional queue for energy library
                    PseudoComponent->queue.create<counter_t>(PseudoComponent->queue_size, KITFOX_DATA_COUNTER, KITFOX_QUEUE_DISCRETE, counter_t());
                    
                    // Initialize energy library.
                    energy_library->initialize();
                    
                    // Area calculation
                    dimension_t dimension = PseudoComponent->queue.get<dimension_t>(0.0, UNSPECIFIED_TIME, KITFOX_DATA_DIMENSION);
                    dimension.area = energy_library->get_area();
                    
#ifdef KITFOX_DEBUG
                    unit_energy_t unit_energy = energy_library->get_unit_energy();
                    if(unit_energy.baseline > 0.0)
                        cout << "\t\tbaseline energy = " << unit_energy.baseline << " J" << endl;
                    if(unit_energy.switching > 0.0)
                        cout << "\t\tswitching energy = " << unit_energy.switching << " J" << endl;
                    if(unit_energy.read > 0.0)
                        cout << "\t\tread energy = " << unit_energy.read << " J" << endl;
                    if(unit_energy.write > 0.0)
                        cout << "\t\twrite energy = " << unit_energy.write << " J" << endl;
                    if(unit_energy.read_tag > 0.0)
                        cout << "\t\tread_tag energy = " << unit_energy.read_tag << " J" << endl;
                    if(unit_energy.write_tag > 0.0)
                        cout << "\t\twrite_tag energy = " << unit_energy.write_tag << " J" << endl;
                    if(unit_energy.search > 0.0)
                        cout << "\t\tsearch energy = " << unit_energy.search << " J" << endl;
                    if(unit_energy.leakage > 0.0)
                        cout << "\t\tleakage energy = " << unit_energy.leakage << " J (" << unit_energy.leakage*clock_frequency << " W)" << endl;
#endif
                    
                    PseudoComponent->queue.overwrite<dimension_t>(0.0, MAX_TIME, KITFOX_DATA_DIMENSION, dimension);
                    
                    // TDP calculation
                    power_t tdp = energy_library->get_tdp_power();
                    // TDP is cumulatively added up toward the root of component tree.
                    PseudoComponent->queue.overwrite<power_t>(0.0, MAX_TIME, KITFOX_DATA_TDP, tdp);
                    break;
                }
                case KITFOX_LIBRARY_THERMAL_MODEL: {
                    thermal_library_t *thermal_library = dynamic_cast<thermal_library_t*>(PseudoComponent->model_library);
                    
                    // Additional queue for thermal library
                    if(PseudoComponent->exists_in_library("generate_thermal_grid")) {
                        if(PseudoComponent->lookup_in_library("generate_thermal_grid")) {
                            PseudoComponent->queue.create<grid_t<Kelvin> >(PseudoComponent->queue_size, KITFOX_DATA_THERMAL_GRID, KITFOX_QUEUE_DISCRETE, grid_t<Kelvin>());
                            thermal_library->generate_thermal_grid = true;
                        }
                    }
                    
                    // floorplan information
                    Setting &setting = PseudoComponent->lookup_in_library("floorplan");
                    for(unsigned i = 0; i < setting.getLength(); i++) {
                        const char *floorplan_name = setting[i];
                        Comp_ID floorplan_id = get_component_id(floorplan_name);
                        thermal_library->add_floorplan(floorplan_name, floorplan_id);
#ifdef KITFOX_DEBUG
                        if(i == 0) cout << "\t\tfloorplan(s):" << endl;
                        cout << "\t\t\t" << floorplan_name << endl;
#endif
                    }
                    // Initialize thermal library.
                    thermal_library->initialize();
                    break;
                }
                case KITFOX_LIBRARY_RELIABILITY_MODEL: {
                    reliability_library_t *reliability_library = dynamic_cast<reliability_library_t*>(PseudoComponent->model_library);
                    
                    // Addition queue for reliability library
                    PseudoComponent->queue.create<Unitless>(PseudoComponent->queue_size, KITFOX_DATA_FAILURE_RATE, KITFOX_QUEUE_DISCRETE, 0.0);
                    
                    // Initialize reliability library
                    reliability_library->initialize();
                    break;
                }
                case KITFOX_LIBRARY_SENSOR_MODEL: {
                    sensor_library_t *sensor_library = dynamic_cast<sensor_library_t*>(PseudoComponent->model_library);
                    
                    // Initialize sensor library
                    sensor_library->initialize();
                    break;
                }
                default: {
                    error(NULL, "unknown model library type");
                }
            }
            
            // Register library update callback function
            PseudoComponent->queue.register_callback(PseudoComponent->model_library, &model_library_t::update_library_variable);
        }
        
#ifdef KITFOX_DEBUG
        assert(pull_data(PseudoComponent->id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_DIMENSION, &dimension) == KITFOX_QUEUE_ERROR_NONE);
        if(dimension.get_area() > 0.0) 
            cout << "\t\tarea = " << dimension.get_area()*1e6 << " mm^2";
        if((dimension.width*dimension.height) > 0.0)
            cout << " (" << dimension.width*dimension.height*1e6 << " mm^2)";
        cout << endl;
        
        
        power_t tdp;
        assert(pull_data(PseudoComponent->id, 0.0, UNSPECIFIED_TIME, KITFOX_DATA_TDP, &tdp) == KITFOX_QUEUE_ERROR_NONE);
        if(tdp.get_total() > 0.0) 
            cout << "\t\tTDP = " << tdp.get_total() << " W (dynamic = " << tdp.dynamic << " W, leakage = " << tdp.leakage << " W)" << endl;

        cout << "KITFOX DEBUG ";
        if(KITFOX_COMM) cout << "[LP=" << rank << "]";
        cout << ": pseudo component " << PseudoComponent->name << " [ID="
        << PseudoComponent->id << "] is initialized" << endl;
#endif
    }
    catch(SettingNotFoundException e) {
        error("libconfig", "%s is not defined in %s", e.getPath(), configuration->getFileName().c_str());
    }
    catch(SettingTypeException e) {
        error("libconfig", "%s has incorrect type in %s", e.getPath(), configuration->getFileName().c_str());
    }
}

const Comp_ID kitfox_t::get_new_comp_id(string ComponentName)
{
    if(++component_count == (component_id_range.second-component_id_range.first+1)) {
        error(NULL, "no more pseudo component can be created; this KitFox has reached the maximum size");
    }
    
    // Store name-ID pair.
    Comp_ID new_component_id = component_count+component_id_range.first-1;
    name_ID_map.insert(pair<string, Comp_ID>(ComponentName, new_component_id));
    return new_component_id;
}

const bool kitfox_t::is_local_component(Comp_ID ComponentID)
{
    return (bool)component.count(ComponentID);
}

const bool kitfox_t::is_local_component(string ComponentName)
{
    return (bool)name_ID_map.count(ComponentName);
}

pseudo_component_t* kitfox_t::get_pseudo_component(Comp_ID ComponentID)
{
    if(is_local_component(ComponentID)) { return component[ComponentID]; }
    else {
        for(unsigned i = 0; i < remote_component.size(); i++) {
            if(remote_component[i]->id == ComponentID) { return remote_component[i]; }
        }
        return NULL;
    }
}

pseudo_component_t* kitfox_t::get_pseudo_component(string ComponentName)
{
    if(is_local_component(ComponentName)) { return component[name_ID_map[ComponentName]]; }
    else {
        for(unsigned i = 0; i < remote_component.size(); i++) {
            if(remote_component[i]->name == ComponentName) { return remote_component[i]; }
        }
        return NULL;
    }
}


// It is called only by the KitFox server when the KitFox in the other MPI rank tries to connect pseudo components.
// A superset component with ComponentID is trying to find its remote subset component with ComponentName.
const Comp_ID kitfox_t::connect_remote_component(Comp_ID ComponentID, string ComponentName)
{
    Comp_ID component_id = INVALID_COMP_ID;
  
    if(name_ID_map.count(ComponentName)) {
        // There must be a subset component with ComponentName.
        component_id = name_ID_map[ComponentName];
        pseudo_component_t *pseudo_component = component[component_id];
        // A placeholder of superset component should have been created.
        pseudo_component_t *superset_component = pseudo_component->get_superset();
        if(!superset_component) {
            error(NULL, "cannot bind a pseudo component [name=%s] that is defined as a root", ComponentName.c_str(), component_id);
        }
        // Now the Comp_ID of placeholder component (superset) is known.
        superset_component->id = ComponentID;
            
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG [LP=" << rank << "]: pseudo component [ID=" 
        << component_id << "] is remotely connected" << endl;
#endif
    }
    else { error(NULL, "cannot find a pseudo component [name=%s] for remote connection", ComponentName.c_str()); }
  
    // Return the Comp_ID of the subset component.
    // If INVALID_COMP_ID is returned, the superset component is notified
    // that no subset component with ComponentName is found, and the KitFox initialization fails.
    return component_id;
}


// Find a pseudo component with ComponentName and return its Comp_ID.
const Comp_ID kitfox_t::get_component_id(string ComponentName, bool UseServer)
{
    // Find a pseudo component with ComponentName. If not found, use server if any other KitFox has a matching component.    
    Comp_ID component_id = INVALID_COMP_ID;
    if(name_ID_map.count(ComponentName)) { component_id = name_ID_map[ComponentName]; }
    else if(UseServer&&server) { component_id = server->get_component_id(ComponentName); }
    else { error(NULL, "cannot find a pseudo component [name=%s] for get_component_id()", ComponentName.c_str()); }
    return component_id;
}


// Find a pseudo component with ComponentID and return its string name.
string kitfox_t::get_component_name(Comp_ID ComponentID, bool UseServer)
{
    // Find a pseudo component with ComponentID. If not found, use server if any other KitFox has a matching component.
    string component_name;
    if(is_local_component(ComponentID)) { component_name = component[ComponentID]->name; }
    else if(UseServer&&server) { component_name = server->get_component_name(ComponentID); }
    else { error(NULL, "cannot find a pseudo component [id=%lu] for get_component_name()", ComponentID); }
    return component_name;
}


// Synchronize the data of a pseudo component with ComponentID with other pseudo components.
// This function distributes the data to other pseudo components, assuming the data is already available
// at the pseudo component with ComponentID.
const int kitfox_t::synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, bool UseServer)
{
    // TODO: This function can better be written with a template function, 
    // but server will anyway have to have this sort of switch-case statement to call a template function.
    // In addition, using switch statement gives flexibility of handling data differently depending DataType.
        
    int queue_error = KITFOX_QUEUE_ERROR_DATA_TYPE_INVALID;
    if(is_local_component(ComponentID)) {
        // A local pseudo component is found.
        pseudo_component_t *pseudo_component = component[ComponentID];
        
        if(pseudo_component->get_subset_count() == 0) { return KITFOX_QUEUE_ERROR_NONE; }
        
        switch(DataType) {
            case KITFOX_DATA_THERMAL_GRID: {
                // Nothing to synchronize
                queue_error = KITFOX_QUEUE_ERROR_NONE; 
                break; 
            }
            case KITFOX_DATA_COUNTER: {
                counter_t counter;
                // The pseudo component has valid counter data.
                if(pull_data(pseudo_component->id, Time, Period, DataType, &counter) == KITFOX_QUEUE_ERROR_NONE) {
                    queue_error = KITFOX_QUEUE_ERROR_NONE;
                }
                else {
                    for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                        counter_t subset_counter;
                        pseudo_component_t *subset_component = pseudo_component->get_subset(i);
                        // Recursively synchronize subset components down the component tree.
                        // Then, add up the counter from the leaf components.
                        int queue_error = synchronize_and_pull_data(subset_component->id, Time, Period, DataType, &subset_counter);
                        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                            error(NULL, "synchronizing %s data failed with pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                        }
                        counter = counter + subset_counter;
                    }
                    pseudo_component->queue.overwrite<counter_t>(Time, Period, DataType, counter);
                    queue_error = pseudo_component->queue.get_error(DataType);
                }
                break;
            }
            case KITFOX_DATA_TDP:
            case KITFOX_DATA_POWER: {
                power_t power;
                // The pseudo component has valid power data.
                if(pull_data(pseudo_component->id, Time, Period, DataType, &power) == KITFOX_QUEUE_ERROR_NONE) {
                    queue_error = KITFOX_QUEUE_ERROR_NONE;
                }
                else {
                    for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                        power_t subset_power;
                        pseudo_component_t *subset_component = pseudo_component->get_subset(i);
                        // Recursively synchronize subset components down the component tree.
                        // Then, add up the power from the leaf components.
                        int queue_error = synchronize_and_pull_data(subset_component->id, Time, Period, DataType, &subset_power);
                        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                            error(NULL, "synchronizing %s data failed with pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                        }
                        power = power + subset_power;
                    }
                    pseudo_component->queue.overwrite<power_t>(Time, Period, DataType, power);
                    queue_error = pseudo_component->queue.get_error(DataType);
                }
                break;
            }
            case KITFOX_DATA_TEMPERATURE: {
                // If this pseudo component has valid temperature, apply the same value to all subset components.
                Kelvin temperature = pseudo_component->queue.get<Kelvin>(Time, Period, DataType);
                if(pseudo_component->queue.get_error(DataType) == KITFOX_QUEUE_ERROR_NONE) {
                    for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                        Kelvin subset_temperature;
                        pseudo_component_t *subset_component = pseudo_component->get_subset(i);
                        // Apply the same temperature value to all subset components.
                        // If a subset component has a valid temperature value, then skip that component.
                        if(pull_data(subset_component->id, Time, Period, DataType, &subset_temperature) != KITFOX_QUEUE_ERROR_NONE) {
                            int queue_error = overwrite_data(subset_component->id, Time, Period, DataType, &temperature);
                            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                                error(NULL, "synchronizing %s data failed at pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                            }
                        }
                        // Synchronization continues down the pseudo component tree.
                        synchronize_data(subset_component->id, Time, Period, DataType);
                    }
                }
                // Do not put an error message for "else" condition.
                // The highest-level component (i.e., package) may not have temperature but thermal grid data.
                queue_error = KITFOX_QUEUE_ERROR_NONE;
                break;
            }
            case KITFOX_DATA_DIMENSION: {
                dimension_t dimension = pseudo_component->queue.get<dimension_t>(Time, Period, DataType);
                dimension.area = 0.0;
                for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                    dimension_t subset_dimension;
                    pseudo_component_t *subset_component = pseudo_component->get_subset(i); 
                    // Recursively synchronize subset components down the component ree.
                    // Then, add up the area from the leaf components.
                    int queue_error = synchronize_and_pull_data(subset_component->id, Time, Period, DataType, &subset_dimension);
                    if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                        error(NULL,"synchronizing %s data failed at pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                    }
                    dimension.area = dimension.area + subset_dimension.get_area();
                }
                if(dimension.area > 0.0) {
                    pseudo_component->queue.overwrite<dimension_t>(Time, Period, DataType, dimension);
                    queue_error = pseudo_component->queue.get_error(DataType);
                }
                else { queue_error = KITFOX_QUEUE_ERROR_NONE; }
                break;
            }
            case KITFOX_DATA_VOLTAGE: 
            case KITFOX_DATA_THRESHOLD_VOLTAGE: {
                // If this pseudo component has valid voltage, apply the same value to all subset components.
                Volt voltage = pseudo_component->queue.get<Volt>(Time, Period, DataType);
                if(pseudo_component->queue.get_error(DataType) == KITFOX_QUEUE_ERROR_NONE) {
                    for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                        Volt subset_voltage;
                        pseudo_component_t *subset_component = pseudo_component->get_subset(i);
                        // Apply the same voltage value to all subset components.
                        int queue_error = overwrite_data(subset_component->id, Time, Period, DataType, &voltage);
                        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                            error(NULL,"synchronizing %s data failed at pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                        }
                        // Synchronization continues down the pseudo component tree.
                        synchronize_data(subset_component->id, Time, Period, DataType);
                    }
                }
                // Do not put an error message for "else" condition.
                queue_error = KITFOX_QUEUE_ERROR_NONE;
                break;
            }
            case KITFOX_DATA_CLOCK_FREQUENCY: {
                // If this pseudo component has valid voltage, apply the same value to all subset components.
                Hertz clock_frequency = pseudo_component->queue.get<Hertz>(Time, Period, DataType);
                if(pseudo_component->queue.get_error(DataType) == KITFOX_QUEUE_ERROR_NONE) {
                    for(unsigned i = 0; i < pseudo_component->get_subset_count(); i++) {
                        Hertz subset_clock_frequency;
                        pseudo_component_t *subset_component = pseudo_component->get_subset(i);
                        // Apply the same clock frequency value to all subset components.
                        int queue_error = overwrite_data(subset_component->id, Time, Period, DataType, &clock_frequency);
                        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                                error(NULL,"synchronizing %s data failed at pseudo_component [id=%lu, name=%s] (%s) at time=%lf, period=%lf", KITFOX_DATA_STR[DataType], subset_component->id, subset_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
                        }
                        // Synchronization continues down the pseudo component tree.
                        synchronize_data(subset_component->id, Time, Period, DataType);
                    }
                }
                // Do not put an error message for "else" condition.
                queue_error = KITFOX_QUEUE_ERROR_NONE;
                break;
            }
            case KITFOX_DATA_FAILURE_RATE: {
                // TODO: Nothing to synchronize
                queue_error = KITFOX_QUEUE_ERROR_NONE;
                break;
            }
            default: { return queue_error; }
        }
    }
    else if(UseServer&&server){ queue_error = server->synchronize_data(ComponentID, Time, Period, DataType); }
    else { error(NULL, "can't find a pseudo component [id=%lu] for synchronize_data()", ComponentID); }
    return queue_error;
}


// It calculates power and update data queues.
void kitfox_t::calculate_power(Comp_ID ComponentID, Second Time, Second Period, counter_t Counter, bool IsTDP, bool UseServer)
{
    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        energy_library_t *energy_library = dynamic_cast<energy_library_t*>(pseudo_component->model_library);
        if(!energy_library) {
            error(NULL, "power can't be calculated with a pseudo component[id=%lu, name=%s] due to casting failure of energy library", pseudo_component->id, pseudo_component->name.c_str());
        }
        
        power_t power;
        if(IsTDP) { power = energy_library->get_tdp_power(); }
        else { power = energy_library->get_runtime_power(Time, Period, Counter); }
        int queue_error = push_data(ComponentID, Time, Period, KITFOX_DATA_POWER, &power);
        queue_error = pull_data(ComponentID, Time, Period, KITFOX_DATA_POWER, &power);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
            error(NULL,"a pseudo component[id=%lu, name=%s] failed at inserting calculated power data (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
        }
        
#ifdef KITFOX_DEBUG
            cout << "KITFOX DEBUG ";
            if(KITFOX_COMM) cout << "[LP=" << rank << "] at t=" << Time << ", p=" << Period;
            cout << ": pseudo component " << pseudo_component->name << " [ID=" << pseudo_component->id << "].power.";
            if(IsTDP) cout << "TDP.";
            else cout << "power.";
            cout << "total=" << power.get_total() << "W (dynamic=" << power.dynamic << "W, leakage=" << power.leakage << "W)" << endl;
#endif
    }
    else if(UseServer&&server) { 
        if(server->calculate_power(ComponentID, Time, Period, Counter, IsTDP) == false) {
            error(NULL, "can't find a pseudo component[id=%lu] for calculate_power()", ComponentID);
        }
    }
    else { error(NULL, "can't find a pseudo component[id=%lu] for calculate_power()", ComponentID); }
}


// It calculates temperature and update data queues.
void kitfox_t::calculate_temperature(Comp_ID ComponentID, Second Time, Second Period, bool DoPowerSync, bool UseServer)
{
    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        thermal_library_t *thermal_library = dynamic_cast<thermal_library_t*>(pseudo_component->model_library);
        
        if(!thermal_library) {
            error(NULL, "temperature can't be calculated with a pseudo component[id=%lu, name=%s] due to casting failure of thermal library", pseudo_component->id, pseudo_component->name.c_str());
        }
        
        if(DoPowerSync) {
            int queue_error = synchronize_data(ComponentID, Time, Period, KITFOX_DATA_POWER);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "temperature can't be calculated with a pseudo component[Comp=%lu, name=%s] due to %s synchronization failure at its sub-component (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_POWER], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
        }
        
        for(unsigned i = 0; i < thermal_library->get_floorplan_counts(); i++) {
            Comp_ID floorplan_id = thermal_library->get_floorplan_id(i);
            power_t floorplan_power;
            int queue_error = pull_data(floorplan_id, Time, Period, KITFOX_DATA_POWER, &floorplan_power);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "temperature can't be calculcated with a pseudo component[id=%lu, name=%s] due to power mapping failure at its floorplan component[id=%lu] (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), floorplan_id, KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
            // Map floorplan power to the thermal library model.
            thermal_library->map_floorplan_power(floorplan_id, floorplan_power);
        }
        // Temperature can be calculated only after power synchronization is completed.
        thermal_library->calculate_temperature(Time, Period);

        if(thermal_library->generate_thermal_grid) {
            grid_t<Kelvin> thermal_grid = thermal_library->get_thermal_grid();
            int queue_error = push_data(ComponentID, Time, Period, KITFOX_DATA_THERMAL_GRID, &thermal_grid);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL,"thermal grid can't be updated with a pseudo component[id=%d] (%s) at time=%lf, period=%lf", ComponentID, KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
            //thermal_grid.clear();
            //assert(pull_data(ComponentID, Time, Period, KITFOX_DATA_THERMAL_GRID, &thermal_grid) == KITFOX_QUEUE_ERROR_NONE);
            //cout << thermal_grid.rows() << " " << thermal_grid.cols() << " " << thermal_grid.dies() << endl;
            //cout << thermal_grid(0, 0, 0) << endl;
        }
        
        for(unsigned i = 0; i < thermal_library->get_floorplan_counts(); i++) {
            Kelvin floorplan_temperature;
            Comp_ID floorplan_id = thermal_library->get_floorplan_id(i);
            // Use grid-floorplan temperature mapping defined in thermal library setting or provided with user input
            floorplan_temperature = thermal_library->get_floorplan_temperature(floorplan_id, KITFOX_TEMPERATURE_MAPPING_UNKNOWN);
            int queue_error = push_and_synchronize_data(floorplan_id, Time, Period, KITFOX_DATA_TEMPERATURE, &floorplan_temperature);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "temperature can't be updated with a pseudo component[id=%lu] (%s) at time=%lf, period=%lf", floorplan_id, KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
        }
    }
    else if(UseServer&&server) {
        if(server->calculate_temperature(ComponentID, Time, Period, DoPowerSync) == false) {
            error(NULL, "can't find a pseudo component[id=%lu] for calculate_temperature()", ComponentID);
        }
    }
    else {
        // This function can't be called for a pseudo component in different MPI ranks.
        error(NULL, "can't find a local pseudo component[id=%lu] for calculate_temperature()", ComponentID);
    }
}


// It calculates failure rates and update data queues
void kitfox_t::calculate_failure_rate(Comp_ID ComponentID, Second Time, Second Period, bool DoDataSync, bool UseServer)
{
    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        reliability_library_t *reliability_library = dynamic_cast<reliability_library_t*>(pseudo_component->model_library);
        
        if(!reliability_library) {
            error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to casting failure of reliability library", pseudo_component->id, pseudo_component->name.c_str());
        }
        
        // Synchronize temperture, voltage, and clock frequency before calculating the failure rate.
        int queue_error;
        if(DoDataSync) {
            queue_error = synchronize_data(ComponentID, Time, Period, KITFOX_DATA_TEMPERATURE);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to %s synchronization failure (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_TEMPERATURE], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
            queue_error = synchronize_data(ComponentID, Time, Period, KITFOX_DATA_VOLTAGE);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to voltage synchronization failure (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_VOLTAGE], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
            queue_error = synchronize_data(ComponentID, Time, Period, KITFOX_DATA_CLOCK_FREQUENCY);
            if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
                error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to voltage synchronization failure (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_CLOCK_FREQUENCY], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
            }
        }
        
        Kelvin temperature; Volt voltage; Hertz clock_frequency; Unitless failure_rate;
        queue_error = pull_data(ComponentID, Time, Period, KITFOX_DATA_TEMPERATURE, &temperature);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
            error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to %s data error (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_TEMPERATURE], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
        }
        queue_error = pull_data(ComponentID, Time, Period, KITFOX_DATA_VOLTAGE, &voltage);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
            error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to %s data error (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_VOLTAGE], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
        }
        queue_error = pull_data(ComponentID, Time, Period, KITFOX_DATA_CLOCK_FREQUENCY, &clock_frequency);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
            error(NULL, "failure rate can't be calculated with a pseudo component[id=%lu, name=%s] due to %s data error (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_CLOCK_FREQUENCY], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
        }
        queue_error = pull_data(ComponentID, Time-Period, fabs(Time-Period)<MAX_TIME_PRECISION?UNSPECIFIED_TIME:Period, KITFOX_DATA_FAILURE_RATE, &failure_rate);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) {
            error(NULL, "failure rate can't be updated with a pseudo component[id=%lu, name=%s] due to invalid %s data at previous sampling point (%s) at time=%lf, period=%lf", pseudo_component->id, pseudo_component->name.c_str(), KITFOX_DATA_STR[KITFOX_DATA_FAILURE_RATE], KITFOX_QUEUE_ERROR_STR[queue_error], Time, Period);
        }
        
        // Failure rate = (Lambda * time);
        failure_rate += (Period * reliability_library->get_failure_rate(Time, Period, temperature, voltage, clock_frequency));
        assert(push_data(ComponentID, Time, Period, KITFOX_DATA_FAILURE_RATE, &failure_rate) == KITFOX_QUEUE_ERROR_NONE);
    }
    else if(UseServer&&server) {
        if(server->calculate_failure_rate(ComponentID, Time, Period, DoDataSync) == false) {
            error(NULL, "can't find a pseudo component[id=%lu] for calculate_failure_rate()", ComponentID);
        }
    }
    else {
        // This function can't be called for a pseudo component in different MPI ranks.
        error(NULL, "can't find a local pseudo component[id=%lu] for calculate_failure_rate()", ComponentID);
    }
}

bool kitfox_t::update_library_variable(Comp_ID ComponentID, int VariableType, void *Value, bool IsLibraryVariable, bool UseServer, int VariableSize, bool Wait)
{
    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        if(!pseudo_component->model_library) {
            error(NULL, "update_library_variable can't be done with a pseudo component[id=%lu, name=%s] because it has no library model", pseudo_component->id, pseudo_component->name.c_str());
        }
        
        return pseudo_component->model_library->update_library_variable(VariableType, Value, IsLibraryVariable);
    }
    else if(UseServer&&server) {
        if(IsLibraryVariable) {
            assert(VariableSize > 0);
            return server->update_library_variable(ComponentID, VariableType, VariableSize, Value, IsLibraryVariable, Wait);
        }
        else {
            return server->update_library_variable(ComponentID, VariableType, KITFOX_DATA_SIZE[VariableType], Value, IsLibraryVariable, Wait);
        }
        
    }
    return false;
}


energy_library_t* kitfox_t::get_energy_library(Comp_ID ComponentID) {
    energy_library_t* energy_library = NULL;

    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        if(!pseudo_component->model_library) {
            //error(NULL, "Casting library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
        energy_library = dynamic_cast<energy_library_t*>(pseudo_component->model_library);
        if(!energy_library) {
            //error(NULL, "Casting energy library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
    }
    else {
        //error(NULL, "Casting library is not allowed for a remote pseudo component[id=%lu]", ComponentID);
        return NULL;
    }

    return energy_library;
}


thermal_library_t* kitfox_t::get_thermal_library(Comp_ID ComponentID) {
    thermal_library_t* thermal_library = NULL;

    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        if(!pseudo_component->model_library) {
            //error(NULL, "Casting library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
        thermal_library = dynamic_cast<thermal_library_t*>(pseudo_component->model_library);
        if(!thermal_library) {
            //error(NULL, "Casting thermal library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
    }
    else {
        //error(NULL, "Casting library is not allowed for a remote pseudo component[id=%lu]", ComponentID);
        return NULL;
    }

    return thermal_library;
}


reliability_library_t* kitfox_t::get_reliability_library(Comp_ID ComponentID) {
    reliability_library_t* reliability_library = NULL;

    if(is_local_component(ComponentID)) {
        pseudo_component_t *pseudo_component = component[ComponentID];
        if(!pseudo_component->model_library) {
            //error(NULL, "Casting library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
        reliability_library = dynamic_cast<reliability_library_t*>(pseudo_component->model_library);
        if(!reliability_library) {
            //error(NULL, "Casting reliability library failed with a pseudo component[id=%lu, name=%s]", pseudo_component->id, pseudo_component->name.c_str());
            return NULL;
        }
    }
    else {
        error(NULL, "Casting library is not allowed for a remote pseudo component[id=%lu]", ComponentID);
    }

    return reliability_library;
}

