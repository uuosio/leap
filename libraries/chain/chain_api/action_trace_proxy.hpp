#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class action_trace_impl;
namespace eosio { 
    namespace chain {
        struct action_trace;
        using action_trace_ptr = std::shared_ptr<action_trace>;
        struct transaction_trace;
        using transaction_trace_ptr = std::shared_ptr<transaction_trace>;
    }
}

using namespace eosio::chain;

class action_trace_proxy {
public:
    action_trace_proxy(const transaction_trace_ptr& trace, int index);
    // action_trace_proxy(const action_trace_proxy& other);

    virtual ~action_trace_proxy();

    virtual uint32_t action_ordinal();
    virtual uint32_t creator_action_ordinal();
    virtual uint32_t closest_unnotified_ancestor_action_ordinal();
    virtual string receipt();
    virtual string receiver();

    virtual string act();
    virtual string get_action_account();
    virtual string get_action_name();
    virtual string get_action_authorization();
    virtual string get_action_data();

    virtual bool context_free();
    virtual uint64_t elapsed();
    virtual string console();
    virtual string trx_id();
    virtual uint32_t block_num();
    virtual string block_time();
    virtual string producer_block_id();
    virtual string account_ram_deltas();
    virtual string except_();
    virtual uint64_t error_code();
    virtual string return_value();

private:
    std::shared_ptr<action_trace_impl> impl;
};

