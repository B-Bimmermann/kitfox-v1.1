#ifndef __KITFOX_CONFIG_H__
#define __KITFOX_CONFIG_H__

#include <libconfig.h++>

namespace libKitFox
{
    class configuration_t : public libconfig::Config
    {
    public:
        configuration_t();
        virtual ~configuration_t();

        void readFile(const char *filename);
        void writeFile(const char *filename);
        const std::string getFileName() const;
        const std::string getCompPath(libconfig::Setting &setting) const;
    
    private:
        std::string config_file; // config file name
    };
} // namespace libKitFox

#endif
