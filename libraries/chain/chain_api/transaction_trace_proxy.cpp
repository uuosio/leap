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
    transaction_trace_impl(transaction_trace_ptr *bsp, bool attach) {
        if (attach) {
            _transaction_trace = bsp;
        } else {
            _transaction_trace = new transaction_trace_ptr(*bsp);
        }
    }

    ~transaction_trace_impl() {
        if (_transaction_trace) {
            delete _transaction_trace;
        }
    }

    // transaction_trace_impl(const transaction_trace_impl& other) {
    //     _transaction_trace = other._transaction_trace;
    // }

    string get_id() {
        return (*_transaction_trace)->id.str();
    }

    uint32_t block_num() {
        return (*_transaction_trace)->block_num;
    }

    bool is_onblock() {
        return eosio::chain::is_onblock(**_transaction_trace);
    }

private:
    transaction_trace_ptr *_transaction_trace;
};

transaction_trace_proxy::transaction_trace_proxy(transaction_trace_ptr *bsp, bool attach): impl(std::make_shared<transaction_trace_impl>(bsp, attach)) {

}

// transaction_trace_proxy::transaction_trace_proxy(const transaction_trace_proxy& other) {
//     impl = other.impl;
// }

transaction_trace_proxy::~transaction_trace_proxy() {

}

string transaction_trace_proxy::get_id() {
    return impl->get_id();
}

uint32_t transaction_trace_proxy::block_num() {
    return impl->block_num();
}

bool transaction_trace_proxy::is_onblock() {
    return impl->is_onblock();
}
