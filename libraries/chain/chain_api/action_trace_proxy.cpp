#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/trace.hpp>
#include <fc/io/raw.hpp>
#include <fc/io/json.hpp>
#include "action_trace_proxy.hpp"

using namespace std;
using namespace eosio::chain;

class action_trace_impl {
public:
    action_trace_impl(const transaction_trace_ptr& trace, int index):
    _transaction_trace_ptr(trace),
    _action_trace(trace->action_traces[index])
    {
    }

    ~action_trace_impl() {
    }

    uint32_t action_ordinal() {
        return _action_trace.action_ordinal.value;
    }

    uint32_t creator_action_ordinal() {
        return _action_trace.creator_action_ordinal.value;
    }

    uint32_t closest_unnotified_ancestor_action_ordinal() {
        return _action_trace.closest_unnotified_ancestor_action_ordinal.value;
    }

    string receipt() {
        return fc::json::to_string(_action_trace.receipt, fc::time_point::maximum());
    }

    string receiver() {
        return _action_trace.receiver.to_string();
    }

    string act() {
        return fc::json::to_string(_action_trace.act, fc::time_point::maximum());
    }
    // account_name             account;
    // action_name              name;
    // vector<permission_level> authorization;
    string get_action_account() {
        return _action_trace.act.account.to_string();
    }

    string get_action_name() {
        return _action_trace.act.name.to_string();
    }

    string get_action_authorization() {
        return fc::json::to_string(_action_trace.act.authorization, fc::time_point::maximum());
    }

    string get_action_data() {
        return fc::json::to_string(_action_trace.act.data, fc::time_point::maximum());
    }

    bool context_free() {
        return _action_trace.context_free;
    }

    uint64_t elapsed() {
        return _action_trace.elapsed.count();
    }

    string console() {
        return _action_trace.console;
    }

    string trx_id() {
        return _action_trace.trx_id.str();
    }

    uint32_t block_num() {
        return _action_trace.block_num;
    }

    string block_time() {
        return fc::json::to_string(_action_trace.block_time, fc::time_point::maximum());
    }
//     std::optional<block_id_type>    producer_block_id;
    string producer_block_id() {
        return fc::json::to_string(_action_trace.producer_block_id, fc::time_point::maximum());
    }
//     flat_set<account_delta>         account_ram_deltas;
    string account_ram_deltas() {
        return fc::json::to_string(_action_trace.account_ram_deltas, fc::time_point::maximum());
    }
//     std::optional<fc::exception>    except;
    string except_() {
        return fc::json::to_string(_action_trace.except, fc::time_point::maximum());
    }
//     std::optional<uint64_t>         error_code;
    uint64_t error_code() {
        return _action_trace.error_code.value_or(0);
    }
//     std::vector<char>               return_value;
    string return_value() {
        return fc::json::to_string(_action_trace.return_value, fc::time_point::maximum());
    }

private:
    transaction_trace_ptr _transaction_trace_ptr;
    action_trace& _action_trace;
};

action_trace_proxy::action_trace_proxy(const transaction_trace_ptr& trace, int index): impl(std::make_shared<action_trace_impl>(trace, index)) {

}

action_trace_proxy::~action_trace_proxy() {

}

uint32_t action_trace_proxy::action_ordinal() {
    return impl->action_ordinal();
}

uint32_t action_trace_proxy::creator_action_ordinal() {
    return impl->creator_action_ordinal();
}

uint32_t action_trace_proxy::closest_unnotified_ancestor_action_ordinal() {
    return impl->closest_unnotified_ancestor_action_ordinal();
}

string action_trace_proxy::receipt() {
    return impl->receipt();
}

string action_trace_proxy::receiver() {
    return impl->receiver();
}
//     action                          act;
string action_trace_proxy::act() {
    return impl->act();
}

string action_trace_proxy::get_action_account() {
    return impl->get_action_account();
}

string action_trace_proxy::get_action_name() {
    return impl->get_action_name();
}

string action_trace_proxy::get_action_authorization() {
    return impl->get_action_authorization();
}

string action_trace_proxy::get_action_data() {
    return impl->get_action_data();
}

bool action_trace_proxy::context_free() {
    return impl->context_free();
}

uint64_t action_trace_proxy::elapsed() {
    return impl->elapsed();
}

string action_trace_proxy::console() {
    return impl->console();
}

string action_trace_proxy::trx_id() {
    return impl->trx_id();
}

uint32_t action_trace_proxy::block_num() {
    return impl->block_num();
}

string action_trace_proxy::block_time() {
    return impl->block_time();
}

string action_trace_proxy::producer_block_id() {
    return impl->producer_block_id();
}

string action_trace_proxy::account_ram_deltas() {
    return impl->account_ram_deltas();
}

string action_trace_proxy::except_() {
    return impl->except_();
}

uint64_t action_trace_proxy::error_code() {
    return impl->error_code();
}

string action_trace_proxy::return_value() {
    return impl->return_value();
}
