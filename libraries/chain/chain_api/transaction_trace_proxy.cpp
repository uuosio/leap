#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <fc/io/json.hpp>
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

    string id() {
        return (*_transaction_trace)->id.str();
    }

    uint32_t block_num() {
        return (*_transaction_trace)->block_num;
    }

    bool is_onblock() {
        return eosio::chain::is_onblock(**_transaction_trace);
    }

    string block_time() {
        return fc::json::to_string((*_transaction_trace)->block_time, fc::time_point::maximum());
    }

    string producer_block_id() {
        if ((*_transaction_trace)->producer_block_id) {
            return (*_transaction_trace)->producer_block_id->str();
        } else {
            return "";
        }
    }

    string receipt() {
        if ((*_transaction_trace)->receipt) {
            return fc::json::to_string((*_transaction_trace)->receipt, fc::time_point::maximum());
        } else {
            return "";
        }
    }

    int64_t elapsed() {
        return (*_transaction_trace)->elapsed.count();
    }

    uint64_t net_usage() {
        return (*_transaction_trace)->net_usage;
    }

    bool scheduled() {
        return (*_transaction_trace)->scheduled;
    }
// vector<action_trace>                       action_traces;
// std::optional<account_delta>               account_ram_delta;
    string account_ram_delta() {
        if ((*_transaction_trace)->account_ram_delta) {
            return fc::json::to_string((*_transaction_trace)->account_ram_delta, fc::time_point::maximum());
        } else {
            return "";
        }
    }

    transaction_trace_ptr& failed_dtrx_trace() {
        return (*_transaction_trace)->failed_dtrx_trace;
    }
// std::optional<fc::exception>               except;
    string except_() {
        if (!(*_transaction_trace)->except) {
            return "";
        }
        return fc::json::to_string((*_transaction_trace)->except, fc::time_point::maximum());
    }
// std::optional<uint64_t>                    error_code;
    uint64_t error_code() {
        if (!(*_transaction_trace)->error_code) {
            return 0;
        }
        return *(*_transaction_trace)->error_code;
    }
// std::exception_ptr                         except_ptr;

    transaction_trace_ptr& get_transaction_trace_ptr() {
        return *_transaction_trace;
    }

    int get_action_traces_size() {
        return (*_transaction_trace)->action_traces.size();
    }

private:
    transaction_trace_ptr *_transaction_trace;
};

transaction_trace_proxy::transaction_trace_proxy(transaction_trace_ptr *ttp, bool attach): impl(std::make_shared<transaction_trace_impl>(ttp, attach)) {

}

transaction_trace_proxy::~transaction_trace_proxy() {

}

string transaction_trace_proxy::id() {
    return impl->id();
}

uint32_t transaction_trace_proxy::block_num() {
    return impl->block_num();
}

bool transaction_trace_proxy::is_onblock() {
    return impl->is_onblock();
}

string transaction_trace_proxy::block_time() {
    return impl->block_time();
}

string transaction_trace_proxy::producer_block_id() {
    return impl->producer_block_id();
}

string transaction_trace_proxy::receipt() {
    return impl->receipt();
}

int64_t transaction_trace_proxy::elapsed() {
    return impl->elapsed();
}

uint64_t transaction_trace_proxy::net_usage() {
    return impl->net_usage();
}

bool transaction_trace_proxy::scheduled() {
    return impl->scheduled();
}

string transaction_trace_proxy::account_ram_delta() {
    return impl->account_ram_delta();
}

transaction_trace_ptr& transaction_trace_proxy::failed_dtrx_trace() {
    return impl->failed_dtrx_trace();
}

string transaction_trace_proxy::except_() {
    return impl->except_();
}

uint64_t transaction_trace_proxy::error_code() {
    return impl->error_code();
}

int transaction_trace_proxy::get_action_traces_size() {
    return impl->get_action_traces_size();
}

action_trace_proxy *transaction_trace_proxy::get_action_trace(int index) {
    return new action_trace_proxy(impl->get_transaction_trace_ptr(), index);
}

bool transaction_trace_proxy::free_action_trace(action_trace_proxy *_action_trace_proxy) {
    if (_action_trace_proxy == nullptr) {
        return false;
    }
    delete _action_trace_proxy;
    return true;
}
