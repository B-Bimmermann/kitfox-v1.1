#ifndef __KITFOX_THREADER_H__
#define __KITFOX_THREADER_H__

#include <pthread.h>
#include <vector>

namespace libKitFox {

class kitfox_t;
    
class threader_t
{
public:
    threader_t(kitfox_t *KitFox);
    ~threader_t();

    void create(void* (*f)(void*), void *v);
    void join();
    
private:
    kitfox_t *kitfox;
    pthread_attr_t attr;
    std::vector<pthread_t> threads;
};

} // namespace libKitFox

#endif
