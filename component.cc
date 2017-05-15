#include <iostream>
#include <cstdlib>
#include "component.h"
#include "kitfox.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

pseudo_component_t::pseudo_component_t(Comp_ID ID, kitfox_t *KitFox, string Name, 
                                       libconfig::Setting *ConfigSetting, bool IsRemoteComponent) :
    rank(KitFox->rank),
    id(ID),
    name(Name),
    kitfox(KitFox),
    model_library(NULL),
    setting(ConfigSetting),
    queue_size(1),
    is_remote(IsRemoteComponent),
    is_parallelizable(false),
    superset(NULL)
{
    if(name.length() > MAX_KITFOX_COMP_NAME_LENGTH) {
        cout << "KITFOX ERROR: " << name << " is longer than maximum length " << MAX_KITFOX_COMP_NAME_LENGTH << endl;
        exit(EXIT_FAILURE);
    }
  
    if(exists("queue_size",true)) { queue_size = lookup("queue_size",true); }
    if(exists("pthread_init")) { is_parallelizable = lookup("pthread_init"); }
}

pseudo_component_t::~pseudo_component_t()
{
	queue.reset();
    delete model_library;
}

void pseudo_component_t::add_superset(pseudo_component_t *PseudoComponent)
{
    if(!get_superset()) {
        superset = PseudoComponent;
        PseudoComponent->subset.push_back(this);
    }
    else if(get_superset() != PseudoComponent) {
        cout << "KITFOX ERROR: cannot bind " << name << " to " << PseudoComponent->name << endl;
        exit(EXIT_FAILURE);
    }
}

void pseudo_component_t::add_subset(pseudo_component_t *PseudoComponent)
{
    if(!PseudoComponent->get_superset()) {
        PseudoComponent->superset = this;
        subset.push_back(PseudoComponent);
    }
    else if(PseudoComponent->get_superset() != this) {
        cout << "KITFOX ERROR: cannot bind " << PseudoComponent->name << " to " << name << endl;
        exit(EXIT_FAILURE);
    }
}

pseudo_component_t* pseudo_component_t::get_superset() const
{
    return superset;
}

pseudo_component_t* pseudo_component_t::get_subset(unsigned i)  const
{
    return subset[i];
}

const int pseudo_component_t::get_subset_count() const
{
    return subset.size();
}

Setting& pseudo_component_t::get_setting() const
{
    return *setting;
}

Setting& pseudo_component_t::get_library_setting() const
{
    if(!setting->exists("library")) {
        cout << "KITFOX ERROR: " << name << ".library is not defined" << endl;
        exit(EXIT_FAILURE);
    }
    return (*setting)["library"];
}

const bool pseudo_component_t::exists_library(void)
{
    if(setting->exists("library")) { return true; }
    return false;
}

const bool pseudo_component_t::exists(string &Var, bool InPathLookup)
{
    if(!InPathLookup) { return get_setting().exists(Var); }
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->get_setting().exists(Var)) { return true; }
            if(pc->get_setting().exists(Var)) { return true; }
            pc = pc->superset;
        }
    }
    return false;
}

const bool pseudo_component_t::exists(const char *Var, bool InPathLookup)
{
    if(!InPathLookup)
        return get_setting().exists(Var);
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->get_setting().exists(Var)) { return true; }
            if(pc->get_setting().exists(Var)) { return true; }
            pc = pc->superset;
        }
    }
    return false;
}

const bool pseudo_component_t::exists_in_library(string &Var, bool InPathLookup)
{
    if(!InPathLookup)
        return (exists_library()&&get_library_setting().exists(Var));
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->exists_library()&&pc->get_library_setting().exists(Var)) { return true; }
            if(pc->exists_library()&&pc->get_library_setting().exists(Var)) { return true; }
            pc = pc->get_superset();
        }
    }
    return false;
}

const bool pseudo_component_t::exists_in_library(const char *Var, bool InPathLookup)
{
    if(!InPathLookup)
        return (exists_library()&&get_library_setting().exists(Var));
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->exists_library()&&pc->get_library_setting().exists(Var)) { return true; }
            if(pc->exists_library()&&pc->get_library_setting().exists(Var)) { return true; }
            pc = pc->get_superset();
        }
    }
    return false;
}

Setting& pseudo_component_t::lookup(string &Var, bool InPathLookup)
{
    if(!InPathLookup) {
        if(get_setting().exists(Var)) { return get_setting()[Var.c_str()]; }
    
        cout << "KITFOX ERROR: " << name << "." << Var << " is not defined" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->get_setting().exists(Var)) {
            if(pc->get_setting().exists(Var)) {
                pc->get_setting()[Var.c_str()];
            }
            pc = pc->get_superset();
        }
    }
    cout << "KITFOX ERROR: " << name << "." << Var << " is not defined" << endl;
    exit(EXIT_FAILURE);
}

Setting& pseudo_component_t::lookup(const char *Var, bool InPathLookup)
{
    if(!InPathLookup) {
        if(get_setting().exists(Var)) { return get_setting()[Var]; }
    
        cout << "KITFOX ERROR: " << name << "." << Var << " is not defined" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->get_setting().exists(Var)) {
            if(pc->get_setting().exists(Var)) {
                pc->get_setting()[Var];
            }
            pc = pc->get_superset();
        }
    }
    cout << "KITFOX ERROR: " << name << "." << Var << " is not defined" << endl;
    exit(EXIT_FAILURE);
}

Setting& pseudo_component_t::lookup_in_library(string &Var, bool InPathLookup)
{
    if(!InPathLookup) {
        if(exists_library()&&get_library_setting().exists(Var)) { return get_library_setting()[Var.c_str()]; }

        cout << "KITFOX ERROR: " << name << ".library." << Var << " is not defined" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->exists_library()&&pc->get_library_setting().exists(Var)) {
            if(pc->exists_library()&&pc->get_library_setting().exists(Var)) {
                return pc->get_library_setting()[Var.c_str()];
            }
            pc = pc->get_superset();
        }
    }
    cout << "KITFOX ERROR: " << name << ".library." << Var << " is not defined" << endl;
    exit(EXIT_FAILURE);
}

Setting& pseudo_component_t::lookup_in_library(const char *Var, bool InPathLookup)
{
    if(!InPathLookup) {
        if(exists_library()&&get_library_setting().exists(Var)) { return get_library_setting()[Var]; }
    
        cout << "KITFOX ERROR: " << name << ".library." << Var << " is not defined" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        pseudo_component_t *pc = this;
        while(pc) {
            //if(!pc->is_remote&&pc->exists_library()&&pc->get_library_setting().exists(Var)) {
            if(pc->exists_library()&&pc->get_library_setting().exists(Var)) {
                return pc->get_library_setting()[Var]; 
            }
            pc = pc->get_superset();
        }
    }
    cout << "KITFOX ERROR: " << name << ".library." << Var << " is not defined" << endl;
    exit(EXIT_FAILURE);
}

