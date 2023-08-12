#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class u64_double_index_impl_base;

class u64_double_index_proxy {
public:
    u64_double_index_proxy(int sort_type);
    virtual ~u64_double_index_proxy();

    virtual bool create(uint64_t key, double value);
    virtual bool modify(uint64_t key, double old_value, double new_value);
    virtual bool remove(uint64_t key, double old_value);
    virtual bool find(double secondary, uint64_t& primary);
    virtual bool lower_bound(double& secondary, uint64_t& primary);
    virtual bool upper_bound(double& secondary, uint64_t& primary);

    virtual bool first(double& secondary, uint64_t& primary);
    virtual bool last(double& secondary, uint64_t& primary);

    virtual bool pop_first(double& secondary, uint64_t& primary);
    virtual bool pop_last(double& secondary, uint64_t& primary);

    virtual uint64_t row_count();
private:
    std::unique_ptr<u64_double_index_impl_base> impl;
};
