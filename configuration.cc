#include <iostream>
#include <cstdlib>
#include "configuration.h"

using namespace std;
using namespace libconfig;
using namespace libKitFox;

configuration_t::configuration_t()
{
}

configuration_t::~configuration_t()
{
}

void configuration_t::readFile(const char *filename)
{
    config_file = filename;
    Config::readFile(filename);
}

void configuration_t::writeFile(const char *filename)
{
    Config::writeFile(filename);
}

const string configuration_t::getFileName() const
{
    return config_file;
}

const string configuration_t::getCompPath(Setting &setting) const
{
    string path = setting.getPath(); // Path name in libconfig
  
    // Remove "component." in the path name
    while(path.find("component.") != string::npos) {
        path.erase(path.find("component."),10);
    }
    
    /*
    Setting *s = &setting;
    while(!s->isRoot()) {
        if(s->exists("superset")) {
            if(s->getPath() != (string("component.")+s->getName())) {
                cout << "KITFOX ERROR (libconfig): " << path << ".superset can only be defined "
                << "in the root components of a MPI rank" << endl;
                exit(EXIT_FAILURE);
            }
            string superset_path = (const char*)(*s)["superset"];
            return superset_path+"."+path;
        }
        s = &s->getParent();
    }
    */
    
    return path;
}

