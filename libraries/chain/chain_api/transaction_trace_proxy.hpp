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

using namespace eosio::chain;

class transaction_trace_proxy {
public:
    transaction_trace_proxy(transaction_trace_ptr *trace, bool attach);
    // transaction_trace_proxy(const transaction_trace_proxy& other);

    virtual ~transaction_trace_proxy();

    virtual string get_id();
    virtual uint32_t block_num();
    virtual bool is_onblock();

private:
    std::shared_ptr<transaction_trace_impl> impl;
};

