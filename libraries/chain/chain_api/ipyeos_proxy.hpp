#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

#include "transaction_proxy.hpp"
#include "chain_proxy.hpp"
#include "database_proxy.hpp"
#include "trace_api_proxy.hpp"
#include "block_log_proxy.hpp"
#include "snapshot_proxy.hpp"
#include "state_history_proxy.hpp"
#include "apply_context_proxy.hpp"
#include "key_value_data_index.hpp"
#include "multi_index_proxy.hpp"
#include "read_write_lock_proxy.hpp"
#include "block_state_proxy.hpp"
#include "transaction_trace_proxy.hpp"
#include "signed_block_proxy.hpp"
#include "../vm_api/vm_api_proxy.hpp"
#include "intrinsics.h"

using namespace std;

namespace eosio {
    namespace chain {
        class controller;
    }
}

class chain_rpc_api_proxy;
class apply_context_proxy;
class trace_api_proxy;

typedef bool (*fn_on_produce_block)(const char *raw_block, size_t raw_block_size);

class eos_cb {
public:
    eos_cb();
    virtual ~eos_cb();

    virtual int init(int argc, char** argv);
    virtual int init2(int argc, char** argv);
    virtual int exec();
    virtual int exec_once();
    virtual void initialize_logging(string& config_path);
    virtual void print_log(int level, string& logger_name, string& message);

    virtual void quit();
    virtual bool is_quiting();
    
    virtual void exit();
    virtual bool should_exit();

    virtual void *post(void *(*fn)(void *), void *args);
    virtual bool post_signed_block(const char *raw_block, size_t raw_block_size, bool _async);

    virtual bool set_on_produce_block_cb(fn_on_produce_block cb);
    virtual bool on_produce_block(const char *raw_block, size_t raw_block_size);

    virtual void *get_database();

    virtual void *get_controller();
    virtual void set_controller(void *controller);

    virtual string unpack_native_object(int type, const char *packed_native_object, size_t packed_native_object_size);

    virtual string data_dir();
    virtual string config_dir();

    virtual chain_rpc_api_proxy *new_chain_api(eosio::chain::controller *c);
    virtual trace_api_proxy *new_trace_api_proxy(void *chain, string& trace_dir, uint32_t slice_stride, int32_t minimum_irreversible_history_blocks, int32_t minimum_uncompressed_irreversible_history_blocks, uint32_t compression_seek_point_stride);
    virtual state_history_proxy *new_state_history_proxy();

    virtual void set_log_level(string& logger_name, int level);
    virtual int get_log_level(string& logger_name);

    virtual void enable_deep_mind(void *controller);
private:
    string _chain_config;
    void *controller = nullptr;
    bool _should_exit = false;
    fn_on_produce_block _on_produce_block = nullptr;
};

class ipyeos_proxy {
    public:
        ipyeos_proxy(eos_cb *cb);
        virtual ~ipyeos_proxy();

        virtual apply_context_proxy *get_apply_context_proxy();

        virtual transaction_proxy *transaction_proxy_new(
            uint32_t expiration,
            const char* ref_block_id,
            size_t ref_block_id_size,
            uint32_t max_net_usage_words, //fc::unsigned_int
            uint8_t  max_cpu_usage_ms,    //
            uint32_t delay_sec            //fc::unsigned_int
        );
        virtual transaction_proxy *transaction_proxy_new_ex(signed_transaction_ptr *transaction_ptr);
        virtual bool transaction_proxy_free(void *transaction_proxy_ptr);

        virtual vm_api_proxy *get_vm_api_proxy();

        virtual snapshot_proxy *new_snapshot_proxy(void *chain);
        virtual database_proxy *new_database_proxy(void *db_ptr, bool attached);
        virtual chainbase::database *new_database(const string& dir, bool read_only, uint64_t shared_file_size, bool allow_dirty);
        virtual block_log_proxy *new_block_log_proxy(string& block_log_dir);

        virtual chain_proxy* chain_new(string& config, string& _genesis, string& _chain_id, string& protocol_features_dir, string& snapshot_dir, string& debug_producer_key);
        virtual chain_proxy* chain_attach(void *controller);
        virtual void chain_free(chain_proxy* api);

        virtual block_state_proxy *block_state_proxy_new(void *block_state_proxy_ptr);
        virtual bool block_state_proxy_free(void *block_state_proxy_ptr);

        virtual transaction_trace_proxy *transaction_trace_proxy_new(transaction_trace_ptr *_transaction_trace_ptr, bool attach);
        virtual bool transaction_trace_proxy_free(transaction_trace_proxy *transaction_trace_proxy_ptr);

        signed_block_proxy *signed_block_proxy_new(signed_block_ptr *_signed_block_ptr);
        signed_block_proxy *signed_block_proxy_attach(signed_block_ptr *_signed_block_ptr);

        bool signed_block_proxy_free(void *signed_block_proxy_ptr);

        virtual key_value_index_proxy* new_key_value_index_proxy();
        virtual bool free_key_value_index_proxy(void *ptr);

        virtual read_write_lock_proxy* read_write_lock_proxy_new(const string& mutex_name);
        virtual bool read_write_lock_proxy_free(void *ptr);

        virtual u64_double_index_proxy* new_u64_double_index_proxy(int sort_type);
        virtual bool free_u64_double_index_proxy(void *ptr);


        virtual string extract_chain_id_from_snapshot(string& snapshot_dir);

        virtual void pack_abi(string& abi, vector<char>& packed_message);

        virtual void pack_native_object(int type, string& msg, vector<char>& packed_message);
        virtual void unpack_native_object(int type, string& packed_message, string& msg);

        virtual string get_last_error();
        virtual void set_last_error(string& error);

        virtual string get_chain_config(void *controller);
        virtual bool set_chain_config(void *controller, const string& config);

        virtual uint64_t s2n(string& name);
        virtual string n2s(uint64_t n);

        virtual bool call_native_contract(uint64_t receiver, uint64_t first_receiver, uint64_t action);

        virtual void enable_debug(bool debug);
        virtual bool is_debug_enabled();

        virtual void enable_adjust_cpu_billing(bool enabled);
        virtual bool is_adjust_cpu_billing_enabled();

        virtual void set_max_database_cpu_billing_time_us(int64_t us);
        virtual int64_t get_max_database_cpu_billing_time_us();

        virtual void set_worker_process(bool worker_process);
        virtual bool is_worker_process();

        virtual string create_key(string &key_type);
        virtual string get_public_key(string &priv_key);

        virtual string get_debug_producer_key(void *id);

        virtual string sign_digest(string &digest, string &priv_key);

        virtual bool base58_to_bytes(const string& s, vector<char>& out);
        virtual bool bytes_to_base58(const char* data, size_t data_size, string& out);
        virtual void ripemd160(const char *data, size_t data_size, vector<char>& hash);

        eos_cb *cb;
    private:
        map<eosio::chain::controller*, chain_proxy*> chain_proxy_map;
        map<void *, string> chain_config_map;
        string last_error;
        std::shared_ptr<apply_context_proxy> _apply_context_proxy;
        bool debug_enabled = false;
        bool adjust_cpu_billing_enabled = false;
        int64_t max_database_cpu_billing_time_us = 100;
};

typedef void (*fn_init_ipyeos_proxy)(ipyeos_proxy *proxy);
extern "C" ipyeos_proxy *get_ipyeos_proxy();
