#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/trace.hpp>

#include "transaction_trace_proxy.hpp"

using namespace std;
using namespace eosio::chain;

class transaction_trace_impl {
public:
    transaction_trace_impl(const transaction_trace_ptr& bsp) {
        _transaction_trace = bsp;
    }

    // transaction_trace_impl(const transaction_trace_impl& other) {
    //     _transaction_trace = other._transaction_trace;
    // }

    string get_id() {
        return _transaction_trace->id.str();
    }

    uint32_t get_block_num() {
        return _transaction_trace->block_num;
    }

    bool is_onblock() {
        return eosio::chain::is_onblock(*_transaction_trace);
    }

private:
    transaction_trace_ptr _transaction_trace;
};

transaction_trace_proxy::transaction_trace_proxy(const transaction_trace_ptr& bsp): impl(std::make_shared<transaction_trace_impl>(bsp)) {

}

// transaction_trace_proxy::transaction_trace_proxy(const transaction_trace_proxy& other) {
//     impl = other.impl;
// }

transaction_trace_proxy::~transaction_trace_proxy() {

}

string transaction_trace_proxy::get_id() {
    return impl->get_id();
}

uint32_t transaction_trace_proxy::get_block_num() {
    return impl->get_block_num();
}

bool transaction_trace_proxy::is_onblock() {
    return impl->is_onblock();
}
