#include <assert.h>

#include "threader.h"
#include "kitfox.h"

using namespace std;
using namespace libKitFox;

threader_t::threader_t(kitfox_t *KitFox) :
    kitfox(KitFox)
{
   pthread_attr_init(&attr);
   assert(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0);
}

threader_t::~threader_t()
{
}

void threader_t::create(void* (*f)(void*), void *v)
{
    pthread_t thread;
    pthread_create(&thread,&attr,f,v);
    threads.push_back(thread);
}

void threader_t::join()
{
    //void *status;
    for(unsigned i = 0; i < threads.size(); i++) {
        int rc = pthread_join(threads[i],NULL);//&status);
        if(rc) {
            fprintf(stdout,"KITFOX ERROR (threader): pthread_join failed with rc=%d\n",rc);
            exit(EXIT_FAILURE);
        }
    }
    
    pthread_attr_destroy(&attr);
    threads.clear();
}
