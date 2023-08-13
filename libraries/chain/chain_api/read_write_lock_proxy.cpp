#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>

#include "read_write_lock_proxy.hpp"

using namespace std;

class read_write_lock_impl {
public:
    read_write_lock_impl(const std::string& mutex_name) {
        segment = std::make_unique<boost::interprocess::managed_shared_memory>(boost::interprocess::open_or_create, "read_write_lock_memory", 65536);
        mutex = segment->find_or_construct<boost::interprocess::interprocess_sharable_mutex>(mutex_name.c_str())();
    }

    ~read_write_lock_impl(){
        
    }

    void acquire_read_lock() {
        mutex->lock_sharable();
    }

    void release_read_lock() {
        mutex->unlock_sharable();
    }

    void acquire_write_lock() {
        mutex->lock();
    }

    void release_write_lock() {
        mutex->unlock();
    }

private:
    std::unique_ptr<boost::interprocess::managed_shared_memory> segment;
    boost::interprocess::interprocess_sharable_mutex *mutex;
};

read_write_lock_proxy::read_write_lock_proxy(const string& mutex_name): impl(std::make_unique<read_write_lock_impl>(mutex_name)) {

}

read_write_lock_proxy::~read_write_lock_proxy() {
}

void read_write_lock_proxy::acquire_read_lock() {
    impl->acquire_read_lock();
}

void read_write_lock_proxy::release_read_lock() {
    impl->release_read_lock();
}

void read_write_lock_proxy::acquire_write_lock() {
    impl->acquire_write_lock();
}

void read_write_lock_proxy::release_write_lock() {
    impl->release_write_lock();
}
