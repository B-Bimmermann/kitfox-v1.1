#ifndef __KITFOX_QUEUE_H__
#define __KITFOX_QUEUE_H__

#include <iostream>
#include <map>
#include <cmath>
#include <functional>
#include "kitfox-defs.h"

namespace libKitFox
{
    class model_library_t;
    
    
    


    // Data time is comprised of [time] and [period].
    // Data start_time = [time-period];
    // Data end time = [time];
    // Data is assumed to be continuous to the left,
    // which means data range is (start time,end time].
    class data_time_t
    {
    public:
        data_time_t(Second t, Second p);
        data_time_t(Second t, Second p, Index c);
        data_time_t(const data_time_t &d);
        ~data_time_t();
        
        bool operator < (const data_time_t &q) const;
        bool operator <= (const data_time_t &q) const;
        bool operator > (const data_time_t &q) const;
        bool operator >= (const data_time_t &q) const;
        bool operator == (const data_time_t &q) const;
        bool operator [] (const Second &t) const;
        
        Second time, period;
        Index update_count;
        
    private:
        Second get_trunc_time(const Second t);
    };
  
    inline bool operator<(const data_time_t &dt, const Second &t)
    {
        return (t-dt.time)>=MAX_TIME_PRECISION;
    }
    
    inline bool operator>(const data_time_t &dt, const Second &t)
    {
        return (dt.time-t)>=MAX_TIME_PRECISION;
    }
    
    inline bool operator<=(const data_time_t &dt, const Second &t)
    {
        return (((t-dt.time)>=MAX_TIME_PRECISION)||(fabs(t-dt.time)<=MAX_TIME_PRECISION));
    }
    
    inline bool operator>=(const data_time_t &dt, const Second &t)
    {
        return (((dt.time-t)>=MAX_TIME_PRECISION)||(fabs(dt.time-t)<=MAX_TIME_PRECISION));
    }
    
    inline bool operator==(const data_time_t &dt, const Second &t)
    {
        return (fabs(dt.time-t)<=MAX_TIME_PRECISION);
    }
  
  
  
    // Base queue
    class base_queue_t
    {
    public:
        base_queue_t();
        base_queue_t(unsigned int s);
        virtual ~base_queue_t() {}
        
        virtual void reset() = 0;
        const int get_error();
        
    protected:
        unsigned int size;
        int error;
    };
  
  
  
  
  
    // Data queue of a single type of data
    template <typename T>
    class data_queue_t : public base_queue_t
    {
    public:
        data_queue_t();
        data_queue_t(unsigned int s, int dt, int qt);
        ~data_queue_t();
        
        void reset();
        void push_back(Second t, Second p, T d);
        void overwrite(Second t, Second p, T d);
        T get(Second t, Second p);
        
    private:
        int queue_type;
        int data_type;
        std::map<data_time_t,T> queue;
    };
    
    // Data queue of a single type of data
    template <typename T>
    data_queue_t<T>::data_queue_t() :
    base_queue_t()
    {
    }
    
    template <typename T>
    data_queue_t<T>::data_queue_t(unsigned int s, int dt, int qt) :
    base_queue_t(s), data_type(dt), queue_type(qt)
    {
    }
    
    template <typename T>
    data_queue_t<T>::~data_queue_t()
    {
        queue.clear();
    }
    
    template <typename T>
    void data_queue_t<T>::reset()
    {
        queue.clear();
        error = KITFOX_QUEUE_ERROR_NONE;
    }
    
    // Data push_back into the queue requires both time and period.
    // Time information represents that the data is valid in the duration of (t-p,t].
    // Since most of the data are sampled and discrete, this representation means
    // that the data is calculated at time = t (seconds), based on the sampled information
    // during p (seconds) or between (t-p) and t seconds.
    template <typename T>
    void data_queue_t<T>::push_back(const Second t, const Second p, T d)
    {
        error = KITFOX_QUEUE_ERROR_NONE;
        
        if(queue_type == KITFOX_QUEUE_DISCRETE) {
            if(p == UNSPECIFIED_TIME) {
                // If period is not specified, then assume continuous data range
                // and calculate the period by substracting two time points.
                data_time_t unspecified_data_time(t,p);
                if(queue.size() == 0) {
                    // It's an empty queue. Simply insert the data.
                    queue.insert(std::pair<data_time_t,T>(unspecified_data_time,d));
                }
                else if(unspecified_data_time > queue.rbegin()->first.time) {
                //else if(queue.rbegin()->first.time < unspecified_data_time.time) {
                    // Precedent data exists. Substract the time points to calculate period.
                    data_time_t data_time(unspecified_data_time.time,
                                          unspecified_data_time.time-queue.rbegin()->first.time);
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                    
                    while(queue.size() > size) { // Fit to queue size.
                        queue.erase(queue.begin());
                    }
                }
                else {
                    // Set the error code.
                    error = KITFOX_QUEUE_ERROR_TIME_OVERLAP;
                    
                    // Flush the queue and insert a new data.
                    queue.clear();
                    queue.insert(std::pair<data_time_t,T>(unspecified_data_time,d));
                }
            }
            else {
                data_time_t data_time(t,p);
                
                // Check if data time is continuous; no overlap or gap
                if((queue.size() == 0)||(queue.rbegin()->first <= data_time)) {
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                    while(queue.size() > size) { // Fit to queue size.
                        queue.erase(queue.begin());
                    }
                }
                else {
                    // Set the error code.
                    if(queue.rbegin()->first[data_time.time-data_time.period]||
                       queue.rbegin()->first[data_time.time]) {
                        error = KITFOX_QUEUE_ERROR_TIME_OVERLAP;
                    }
                    else if(queue.rbegin()->first < data_time) {
                        error = KITFOX_QUEUE_ERROR_TIME_DISCONTINUOUS;
                    }
                    else {
                        error = KITFOX_QUEUE_ERROR_TIME_OUTORDER;
                    }
                    
                    // Flush the queue and insert a new data.
                    queue.clear();
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                }
            }
        }
        else { // KITFOX_QUEUE_OPEN
            data_time_t data_time(t,p);
            
            // Time violation. Flush the queue. Set the error code.
            if((queue.size() > 0)&&(data_time <= queue.rbegin()->first.time)) {
            //if((queue.size() > 0)&&(queue.rbegin()->first.time >= data_time.time)) {
                error = KITFOX_QUEUE_ERROR_TIME_OUTORDER;
                queue.clear();
            }
            
            // Insert the new data.
            queue.insert(std::pair<data_time_t,T>(data_time,d));
            
            while(queue.size() > size) { // Fit to queue size.
                queue.erase(queue.begin());
            }
        }
    }
    
    
    // Overwrite the entry of the queue that matches with time range (t-p,t].
    template <typename T>
    void data_queue_t<T>::overwrite(Second t, Second p, T d)
    {
        error = KITFOX_QUEUE_ERROR_NONE;
        
        if(queue_type == KITFOX_QUEUE_DISCRETE) {
            if(p == UNSPECIFIED_TIME) {
                data_time_t unspecified_data_time(t,p);
                typename std::map<data_time_t,T>::iterator it = queue.end();
                
                // Replace the data that falls into the given time range.
                while(it != queue.begin()) {
                    if((--it)->first[unspecified_data_time.time]) {
                        it->second = d;
                        return;
                    }
                }
                
                // Replacement failed. Check other possible conditions.
                if(queue.size() == 0) {
                    // This is an empty queue. Simply insert the data.
                    queue.insert(std::pair<data_time_t,T>(unspecified_data_time,d));
                }
                else if(unspecified_data_time > queue.rbegin()->first.time) {
                //else if(queue.rbegin()->first.time < unspecified_data_time.time) {
                    // The new data satisfies the push_back() condition.
                    // Calculate period by subtracting two time points, and insert the data.
                    data_time_t data_time(unspecified_data_time.time,
                                          unspecified_data_time.time-queue.rbegin()->first.time);
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                    
                    while(queue.size() > size) { // Fit to queue size.
                        queue.erase(queue.begin());
                    }
                }
                else {
                    // Set the error code.
                    error = KITFOX_QUEUE_ERROR_TIME_OUTORDER;
                    
                    // Flush the queue and insert a new data.
                    queue.clear();
                    queue.insert(std::pair<data_time_t,T>(unspecified_data_time,d));
                }
            }
            else {
                data_time_t data_time(t,p);
                typename std::map<data_time_t,T>::iterator it = queue.end();
                
                // Replace the data that exactly matches with given time range.
                while(it != queue.begin()) {
                    if((--it)->first == data_time) {
                        it->second = d;
                        return;
                    }
                }
                
                // Replacement failed. Check other possible conditions.
                if((queue.size() == 0)||(queue.rbegin()->first <= data_time)) {
                    // There isn't a data that matches the given time range,
                    // but it satisfies the push_back of new data.
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                    
                    while(queue.size() > size) { // Fit to queue size.
                        queue.erase(queue.begin());
                    }
                }
                else {
                    // Set the error code.
                    error = KITFOX_QUEUE_ERROR_TIME_OUTORDER;
                    
                    // Flush the queue and insert the new data.
                    queue.clear();
                    queue.insert(std::pair<data_time_t,T>(data_time,d));
                }
            }
        }
        else { // KITFOX_QUEUE_OPEN
            data_time_t data_time(t,p);
            typename std::map<data_time_t,T>::iterator it = queue.end();
            
            // Replace the data that exactly matches with given time.
            while(it != queue.begin()) {
                if(data_time == (--it)->first.time) {
                //if((--it)->first.time == data_time.time) {
                    it->second = d;
                    return;
                }
            }
            
            // Replacement failed. Check other possible conditions.
            if((queue.size() == 0)||(data_time > queue.rbegin()->first.time)) {
            //if((queue.size() == 0)||(queue.rbegin()->first.time < data_time.time)) {
                queue.insert(std::pair<data_time_t,T>(data_time,d));
                
                while(queue.size() > size) { // Fit to queue size.
                    queue.erase(queue.begin());
                }
            }
            else {
                // Set the error code.
                error = KITFOX_QUEUE_ERROR_TIME_OUTORDER;
                
                // Flush the queue and insert the new data.
                queue.clear();
                queue.insert(std::pair<data_time_t,T>(data_time,d));
            }
        }
    }
    
    
    // Return data that matches with the given time range.
    template <typename T>
    T data_queue_t<T>::get(const Second t, const Second p)
    {
        error = KITFOX_QUEUE_ERROR_NONE;
        
        T data;
        data_time_t data_time(t,p);
        typename std::map<data_time_t,T>::iterator it = queue.end();
        
        if(queue_type == KITFOX_QUEUE_DISCRETE) {
            if(p == UNSPECIFIED_TIME) {
                while(it != queue.begin()) {
                    // Return the data that falls into the given time range.
                    if((--it)->first[data_time.time]) {
                        data = it->second;
                        return data;
                    }
                }
            }
            else {
                while(it != queue.begin()) {
                    // Return the data that exactly matches with the given time range.
                    if((--it)->first == data_time) {
                        data = it->second;
                        return data;
                    }
                }
            }
            
            // Set the error code.
            error = KITFOX_QUEUE_ERROR_TIME_INVALID;
            return data;
        
        }
        else { // KITFOX_QUEUE_OPEN
            while(it != queue.begin()) {
                if(data_time >= (--it)->first.time) {
                //if((--it)->first.time <= data_time.time) {
                    data = it->second;
                    return data;
                }
            }
            
            // Set the error code.
            error = KITFOX_QUEUE_ERROR_TIME_INVALID;
            return data;
        }
    }


    
    
    
    // Queue of data queues
    class queue_t
    {
    public:
        queue_t();
        ~queue_t();
        
        template <typename T>
        void create(const unsigned int s, const int dt, const int qt, const T d);
        template <typename T>
        void push_back(const Second t, const Second p, const int dt, T d);
        template <typename T>
        T get(const Second t, const Second p, const int dt);
        template <typename T>
        void overwrite(const Second t, const Second p, const int dt, T d);
        template <typename T>
        bool is_sync(const Second t, const Second p, const int dt);
        void reset();
        void register_callback(model_library_t *lib,
                               bool (model_library_t::*update_library_variable)(int,void*,bool));
        void callback(const int dt, void *data);
        const int get_error(const int dt);
        const char* get_error_str(const int dt);
        
    private:
        template <typename T>
        data_queue_t<T>* get_data_queue(const int dt);
        
        std::map<int,base_queue_t*> queue;
        int error;
        
        model_library_t *library;
        bool (model_library_t::*cb)(int,void*,bool);
    };
    
    template <typename T>
    void queue_t::create(const unsigned int s, const int dt, const int qt, const T d)
    {
        if(queue.find(dt) != queue.end())
        {
            error = KITFOX_QUEUE_ERROR_DATA_DUPLICATED;
            return;
        }
        
        data_queue_t<T> *data_queue = new data_queue_t<T>(s,dt,qt);
        data_queue->push_back(0.0,MAX_TIME,d);
        queue.insert(std::pair<int,base_queue_t*>(dt,data_queue));
        error = KITFOX_QUEUE_ERROR_NONE;
    }
    
    template <typename T>
    void queue_t::push_back(const Second t, const Second p, const int dt, T d)
    {
        data_queue_t<T> *data_queue = get_data_queue<T>(dt);
        if(data_queue)
        {
            data_queue->push_back(t,p,d);
            callback(dt,&d);
            error = KITFOX_QUEUE_ERROR_NONE;
        }
    }
    
    template <typename T>
    T queue_t::get(const Second t, const Second p, const int dt)
    {
        T data;
        data_queue_t<T> *data_queue = get_data_queue<T>(dt);
        if(data_queue)
        {
            data = data_queue->get(t,p);
            error = KITFOX_QUEUE_ERROR_NONE;
        }
        return data;
    }
    
    template <typename T>
    void queue_t::overwrite(const Second t, const Second p, const int dt, T d)
    {
        data_queue_t<T> *data_queue = get_data_queue<T>(dt);
        if(data_queue)
        {
            data_queue->overwrite(t,p,d);
            callback(dt,&d);
            error = KITFOX_QUEUE_ERROR_NONE;
        }
    }
        
    template <typename T>
    data_queue_t<T>* queue_t::get_data_queue(const int dt)
    {
        std::map<int,base_queue_t*>::iterator it = queue.find(dt);
        if(it == queue.end())
        {
            error = KITFOX_QUEUE_ERROR_DATA_INVALID;
            return NULL;
        }
        
        data_queue_t<T> *data_queue = dynamic_cast<data_queue_t<T>*>(it->second);
        if(!data_queue)
        {
            error = KITFOX_QUEUE_ERROR_DATA_TYPE_INVALID;
            return NULL;
        }
        
        return data_queue;
    }

} // namespace libKitFox

#endif
