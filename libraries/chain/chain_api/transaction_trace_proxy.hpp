#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class transaction_trace_impl;
namespace eosio { 
    namespace chain {
        struct transaction_trace;
        using transaction_trace_ptr = std::shared_ptr<transaction_trace>;
    }
}

class transaction_trace_proxy {
public:
    transaction_trace_proxy(const eosio::chain::transaction_trace_ptr& trace);
    // transaction_trace_proxy(const transaction_trace_proxy& other);

    virtual ~transaction_trace_proxy();

    string get_id();
    uint32_t get_block_num();
    bool is_onblock();

private:
    std::shared_ptr<transaction_trace_impl> impl;
};

