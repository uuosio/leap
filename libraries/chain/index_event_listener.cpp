#include <boost/core/demangle.hpp>

#include <eosio/chain/types.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/index_event_listener.hpp>

#include <eosio/chain/account_object.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/resource_limits_private.hpp>
#include <eosio/chain/protocol_state_object.hpp>
#include <eosio/chain/code_object.hpp>
#include <eosio/chain/database_header_object.hpp>

#include <ipyeos.hpp>

namespace eosio { namespace chain {

index_event_listener::index_event_listener(transaction_context& tx_context, int64_t max_billed_cpu_time_us):
tx_context(tx_context),
start_billing_cpu_time_us(0),
max_billed_cpu_time_us(max_billed_cpu_time_us)
{
    FC_ASSERT(!get_undo_index_events(), "undo_index_events already set");
    set_undo_index_events(this);
}

index_event_listener::~index_event_listener() {
    set_undo_index_events(nullptr);
}

void index_event_listener::on_event_begin(const char* function, const std::type_info& value_type_info) {
    if (tx_context.explicit_billed_cpu_time) {
        return;
    }

    auto proxy = get_ipyeos_proxy_ex();
    if (!proxy) {
        return;
    }

    if (!proxy->is_adjust_cpu_billing_enabled()) {
        return;
    }

// database_header_object
// protocol_state_object
// resource_limits_config_object
// resource_limits_state_object
// dynamic_global_property_object
// global_property_object
    auto hash_code = value_type_info.hash_code();
    if (hash_code == typeid(resource_limits::resource_limits_config_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(resource_limits::resource_limits_state_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(dynamic_global_property_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(global_property_object).hash_code()) {
        return;
    }


    FC_ASSERT(start_billing_cpu_time_us == 0, "start_billing_cpu_time_us should be 0");
    start_billing_cpu_time_us = fc::time_point::now().time_since_epoch().count();
    tx_context.pause_billing_timer();
}

void index_event_listener::on_event_end(const char* function, const std::type_info& value_type_info) {
    if (tx_context.explicit_billed_cpu_time) {
        return;
    }

    auto proxy = get_ipyeos_proxy_ex();
    if (!proxy) {
        return;
    }

    if (!proxy->is_adjust_cpu_billing_enabled()) {
        return;
    }

    auto hash_code = value_type_info.hash_code();
    if (hash_code == typeid(resource_limits::resource_limits_config_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(resource_limits::resource_limits_state_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(dynamic_global_property_object).hash_code()) {
        return;
    }

    if (hash_code == typeid(global_property_object).hash_code()) {
        return;
    }

    int64_t billed_cpu_time_us = fc::time_point::now().time_since_epoch().count() - start_billing_cpu_time_us;
    start_billing_cpu_time_us = 0;

    if (billed_cpu_time_us > max_billed_cpu_time_us) {
        wlog("${func}: cost: ${billed_cpu_time_us} us, obj: ${obj})", ("billed_cpu_time_us", billed_cpu_time_us)("func", function)("obj", boost::core::demangle(value_type_info.name())));
        billed_cpu_time_us = max_billed_cpu_time_us;
    } else {
        dlog("${func}: cost: ${billed_cpu_time_us} us, obj: ${obj})", ("billed_cpu_time_us", billed_cpu_time_us)("func", function)("obj", boost::core::demangle(value_type_info.name())));
    }
    tx_context.resume_billing_timer(fc::microseconds(billed_cpu_time_us));
}

void index_event_listener::on_find_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_find_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    // elog("${key_type} ${value_type}", ("key_type", boost::core::demangle(key_type_info.name()))("value_type", value_type_info.name()));
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_lower_bound_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_lower_bound_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_upper_bound_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_upper_bound_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_equal_range_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_equal_range_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) {
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_create_begin(const std::type_info& value_type_info, const void *id) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_create_end(const std::type_info& value_type_info, const void *id, bool success) {
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_modify_begin(const std::type_info& value_type_info, const void *obj) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_modify_end(const std::type_info& value_type_info, const void *obj, bool success) {
    on_event_end(__FUNCTION__, value_type_info);
}

void index_event_listener::on_remove_begin(const std::type_info& value_type_info, const void *obj) {
    on_event_begin(__FUNCTION__, value_type_info);
}

void index_event_listener::on_remove_end(const std::type_info& value_type_info, const void *obj) {
    on_event_end(__FUNCTION__, value_type_info);
}

} }
