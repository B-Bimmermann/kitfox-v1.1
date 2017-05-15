#ifndef __KITFOX_NET_H__
#define __KITFOX_NET_H__

#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <stdint.h>
#include <mpi.h>
#include "kitfox-defs.h"

namespace libKitFox
{
    static inline void pack_message(void *msg, void *buf,
                                    unsigned int length, int *pos)
    {
        memcpy((uint8_t*)buf+*pos,msg,length);
        *pos = *pos+length;
        
        if(*pos > KITFOX_MPI_BUFFER_SIZE) {
            std::cout << "pack_message() failed. "
            << "MPI message is too large (max=" << KITFOX_MPI_BUFFER_SIZE << " bytes)" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    
    static inline void unpack_message(void *msg, void *buf,
                                      unsigned int length, int *pos)
    {
        memcpy(msg,(uint8_t*)buf+*pos,length);
        *pos = *pos+length;
        
        if(*pos > KITFOX_MPI_BUFFER_SIZE) {
            std::cout << "unpack_message() failed. "
            << "MPI message is too large (max=" << KITFOX_MPI_BUFFER_SIZE << " bytes)" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
} // namespace libKitFox

#endif
