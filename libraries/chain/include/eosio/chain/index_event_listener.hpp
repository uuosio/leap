#include <chainbase/undo_index_events.hpp>

const int64_t default_max_database_billed_cpu_time_us = 100;

namespace eosio { namespace chain {
    using namespace chainbase;

    class transaction_context;

    class index_event_listener: public chainbase::undo_index_events {
    public:
        index_event_listener(transaction_context& tx_context, int64_t max_billed_cpu_time_us);
        virtual ~index_event_listener();

        void on_find_begin(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_find_end(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_lower_bound_begin(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_lower_bound_end(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_upper_bound_begin(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_upper_bound_end(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_equal_range_begin(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_equal_range_end(const char *key_type_name, const char *value_type_name, const void *key) override;
        void on_create_begin(const char *value_type_name, const void *id) override;
        void on_create_end(const char *value_type_name, const void *id, bool success) override;
        void on_modify_begin(const char *value_type_name, const void *obj) override;
        void on_modify_end(const char *value_type_name, const void *obj, bool success) override;
        void on_remove_begin(const char *value_type_name, const void *obj) override;
        void on_remove_end(const char *value_type_name, const void *obj) override;

        void on_event_begin(const char* function, const char *value_type_name);
        void on_event_end(const char* function, const char *value_type_name);
    private:
        transaction_context& tx_context;
        int64_t start_billing_cpu_time_us;
        int64_t max_billed_cpu_time_us;
    };
}}
