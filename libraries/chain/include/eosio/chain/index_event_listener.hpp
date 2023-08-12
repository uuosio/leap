#pragma once

#include <chainbase/undo_index_events.hpp>

static const int64_t default_max_database_cpu_billing_time_us = 100;
static const int64_t default_max_total_database_cpu_usage_us = 10000;
static const int64_t default_max_total_time_diff_us = 1000;

namespace eosio { namespace chain {
    using namespace chainbase;

    class transaction_context;

    class index_event_listener: public chainbase::undo_index_events {
    public:
        index_event_listener(transaction_context& tx_context);
        index_event_listener(const index_event_listener& other) = delete;
        index_event_listener(index_event_listener&& other) = delete;
        virtual ~index_event_listener();

        index_event_listener& operator=(const index_event_listener& other) = delete;
        index_event_listener& operator=(index_event_listener&& other) = delete;

        void on_find_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_find_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_lower_bound_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_lower_bound_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_upper_bound_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_upper_bound_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_equal_range_begin(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_equal_range_end(const std::type_info& key_type_info, const std::type_info& value_type_info, const void *key) override;
        void on_create_begin(const std::type_info& value_type_info, const void *id) override;
        void on_create_end(const std::type_info& value_type_info, const void *id, bool success) override;
        void on_modify_begin(const std::type_info& value_type_info, const void *obj) override;
        void on_modify_end(const std::type_info& value_type_info, const void *obj, bool success) override;
        void on_remove_begin(const std::type_info& value_type_info, const void *obj) override;
        void on_remove_end(const std::type_info& value_type_info, const void *obj) override;

        void on_event_begin(const char* function, const std::type_info& value_type_info);
        void on_event_end(const char* function, const std::type_info& value_type_info);
    private:
        transaction_context& tx_context;
        int64_t start_cpu_billing_time_us;
        int64_t max_database_cpu_billing_time_us;
        int64_t start_time_diff_us = 0;
        int64_t total_time_diff_us = 0;
        int64_t total_database_cpu_usage_us = 0;
    };
}}
