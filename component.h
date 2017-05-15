#ifndef __KITFOX_COMPONENT_H__
#define __KITFOX_COMPONENT_H__

#include <string>
#include <vector>
#include <mpi.h>
#include <libconfig.h++>
#include "kitfox-defs.h"
#include "queue.h"
#include "library.h"

namespace libKitFox
{
    class kitfox_t;

    class pseudo_component_t
    {
    public:
        pseudo_component_t(Comp_ID ID, kitfox_t *KitFox, std::string Name, 
                           libconfig::Setting *ConfigSetting, bool IsRemoteComponent = false);
        virtual ~pseudo_component_t();
    
        // pseudo component hierarchy
        void add_superset(pseudo_component_t *PseudoComponent);
        void add_subset(pseudo_component_t *PseudoComponent);
        pseudo_component_t* get_superset() const;
        pseudo_component_t* get_subset(unsigned i) const;
        const int get_subset_count() const;
        
        // config setting
        libconfig::Setting& get_setting() const;
        libconfig::Setting& get_library_setting() const;
        const bool exists_library(void);
        const bool exists(std::string &Var, bool InPathLookup = false);
        const bool exists(const char *Var, bool InPathLookup = false);
        const bool exists_in_library(std::string &Var, bool InPathLookup = false);
        const bool exists_in_library(const char *Var, bool InPathLookup = false);
        libconfig::Setting& lookup(std::string &Var, bool InPathLookup = false);
        libconfig::Setting& lookup(const char *Var, bool InPathLookup = false);
        libconfig::Setting& lookup_in_library(std::string &Var, bool InPathLookup = false);
        libconfig::Setting& lookup_in_library(const char *Var, bool InPathLookup = false);
        
        // Pseudo component information
        int rank;
        Comp_ID id;
        std::string name;
        bool is_remote;
        bool is_parallelizable;
        int queue_size;
        
        friend class kitfox_t;
        kitfox_t *kitfox;
        model_library_t *model_library;
     
        // data queue
        queue_t queue;
        
    private:
        pseudo_component_t *superset;
        std::vector<pseudo_component_t*> subset;
        libconfig::Setting *setting;
    };
} // namespace libKitFox

#endif
