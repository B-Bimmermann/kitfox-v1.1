#include <iostream>
#include "queue.h"

using namespace std;
using namespace libKitFox;

// data time is comprised of [time] and [period]
// data start_time = [time-period];
// data end time = [time];
// data is assumed to be continuous to the left,
// which means data range is (start time,end time]
data_time_t::data_time_t(Second t, Second p) :
update_count(0)
{
    time = get_trunc_time(t);
    period = get_trunc_time(p);
}

data_time_t::data_time_t(Second t, Second p, Index c) :
update_count(c)
{
    time = get_trunc_time(t);
    period = get_trunc_time(p);
}

data_time_t::data_time_t(const data_time_t &d)
{
    time = d.time;
    period = d.period;
}

data_time_t::~data_time_t()
{
}

// truncate digits
Second data_time_t::get_trunc_time(const Second t)
{
    Second time_precision = MAX_TIME_PRECISION/2.0;
    return trunc(t/time_precision)*time_precision;
}

// simply compare times
bool data_time_t::operator < (const data_time_t &q) const
{
    return ((q.time-time)>=MAX_TIME_PRECISION);
    //return time < q.time;
}

// compare times and if time range is continuous
bool data_time_t::operator <= (const data_time_t &q) const
{
    return (fabs(time-(q.time-q.period))<=MAX_TIME_PRECISION);
    //return (time == (q.time-q.period));
}

// simply compare times
bool data_time_t::operator > (const data_time_t &q) const
{
    return ((time-q.time)>=MAX_TIME_PRECISION);
    //return (q.time < time);
}

// compare times and if time range is continuous
bool data_time_t::operator >= (const data_time_t &q) const
{
    return (fabs(q.time-(time-period))<=MAX_TIME_PRECISION);
    //return (q.time == (time-period));
}

// exact time range match
bool data_time_t::operator == (const data_time_t &q) const
{
   return ((fabs(time-q.time)<=MAX_TIME_PRECISION)&&(fabs(period-q.period)<=MAX_TIME_PRECISION));
   //return ((time == q.time)&&(period == q.period));
}

// time within range
bool data_time_t::operator [] (const Second &t) const
{
    return (((t-(time-period))>=MAX_TIME_PRECISION)&&(((time-t)>=MAX_TIME_PRECISION)||(fabs(time-t)<=MAX_TIME_PRECISION)));
    //return ((t > (time-period))&&(t <= time));
}





// base queue
base_queue_t::base_queue_t() :
size(0),
error(KITFOX_QUEUE_ERROR_NONE)
{
}

base_queue_t::base_queue_t(unsigned int s) :
size(s),
error(KITFOX_QUEUE_ERROR_NONE)
{
}

const int base_queue_t::get_error()
{
    return error;
}





// queue of data queues
queue_t::queue_t() :
error(KITFOX_QUEUE_ERROR_NONE),
library(NULL),
cb(NULL)
{
}

queue_t::~queue_t()
{
    while(queue.size())
    {
        delete queue.begin()->second;
        queue.erase(queue.begin());
    }
}

void queue_t::reset()
{
    for(std::map<int,base_queue_t*>::iterator it = queue.begin();
        it != queue.end(); it++) {
        it->second->reset();
    }
}

void queue_t::register_callback(model_library_t *lib,
                                bool (model_library_t::*update_library_variable)
                                (int,void*,bool))
{
    library = lib;
    cb = update_library_variable;
    error = KITFOX_QUEUE_ERROR_NONE;
}

void queue_t::callback(const int dt, void *data)
{
    if(library&&cb) {
        (library->*cb)(dt,data,false);
        error = KITFOX_QUEUE_ERROR_NONE;
    }
}

const int queue_t::get_error(const int dt)
{
    // it doesn't check if queue[dt] is sane
    if(error) return error;
    else return queue[dt]->get_error();
}

const char* queue_t::get_error_str(const int dt)
{
    // it doesn't check if queue[dt] is sane
    if(error) return KITFOX_QUEUE_ERROR_STR[error];
    else return KITFOX_QUEUE_ERROR_STR[queue[dt]->get_error()];
}
