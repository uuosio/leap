#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class read_write_lock_impl;

class read_write_lock_proxy {
public:
    read_write_lock_proxy(const string& mutex_name);
    virtual ~read_write_lock_proxy();

    virtual void acquire_read_lock();
    virtual void release_read_lock();
    virtual void acquire_write_lock();
    virtual void release_write_lock();

private:
    std::unique_ptr<read_write_lock_impl> impl;
};
