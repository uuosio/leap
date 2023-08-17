#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include "action_trace_proxy.hpp"

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

    virtual string id();
    virtual uint32_t block_num();
    virtual bool is_onblock();

    virtual string block_time();
    virtual string producer_block_id();
    virtual string receipt();
    virtual int64_t elapsed();
    virtual uint64_t net_usage();
    virtual bool scheduled();
    virtual string account_ram_delta();
    virtual transaction_trace_ptr& failed_dtrx_trace();
    virtual string except_();
    virtual uint64_t error_code();

    virtual int get_action_traces_size();
    virtual action_trace_proxy *get_action_trace(int index);
    virtual bool free_action_trace(action_trace_proxy *_action_trace_proxy);
    virtual vector<char> pack();
    virtual string to_json();

private:
    std::shared_ptr<transaction_trace_impl> impl;
};

