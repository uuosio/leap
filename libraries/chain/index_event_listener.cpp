#include <eosio/chain/types.hpp>

#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/index_event_listener.hpp>

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

void index_event_listener::on_event_begin(const char *value_type_name) {
    if (!tx_context.control.is_speculative_block()) {
        return;
    }

    FC_ASSERT(start_billing_cpu_time_us == 0, "start_billing_cpu_time_us should be 0");
    start_billing_cpu_time_us = fc::time_point::now().time_since_epoch().count();
    tx_context.pause_billing_timer();
}

void index_event_listener::on_event_end(const char *value_type_name) {
    if (!tx_context.control.is_speculative_block()) {
        return;
    }

    int64_t billed_cpu_time_us = fc::time_point::now().time_since_epoch().count() - start_billing_cpu_time_us;
    start_billing_cpu_time_us = 0;

    if (billed_cpu_time_us > max_billed_cpu_time_us) {
        billed_cpu_time_us = max_billed_cpu_time_us;
    }
    tx_context.resume_billing_timer(fc::microseconds(billed_cpu_time_us));
}

void index_event_listener::on_find_begin(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_find_end(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_end(value_type_name);
}

void index_event_listener::on_lower_bound_begin(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_lower_bound_end(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_end(value_type_name);
}

void index_event_listener::on_upper_bound_begin(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_upper_bound_end(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_end(value_type_name);
}

void index_event_listener::on_equal_range_begin(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_equal_range_end(const char *key_type_name, const char *value_type_name, const void *key) {
    on_event_end(value_type_name);
}

void index_event_listener::on_create_begin(const char *value_type_name, const void *id) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_create_end(const char *value_type_name, const void *id, bool success) {
    on_event_end(value_type_name);
}

void index_event_listener::on_modify_begin(const char *value_type_name, const void *obj) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_modify_end(const char *value_type_name, const void *obj, bool success) {
    on_event_end(value_type_name);
}

void index_event_listener::on_remove_begin(const char *value_type_name, const void *obj) {
    on_event_begin(value_type_name);
}

void index_event_listener::on_remove_end(const char *value_type_name, const void *obj) {
    on_event_end(value_type_name);
}

} }
