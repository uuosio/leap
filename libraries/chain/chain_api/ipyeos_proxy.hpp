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
#include "apply_context_proxy.hpp"
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

class eos_cb {
public:
    eos_cb();
    virtual ~eos_cb();

    virtual int init(int argc, char** argv);
    virtual int exec();
    virtual int exec_once();
    virtual void initialize_logging(string& config_path);
    virtual void print_log(int level, string& logger_name, string& message);
    virtual void quit();
    virtual void *post(void *(*fn)(void *), void *args);
    virtual void *get_database();
    virtual void *get_controller();

    virtual string data_dir();
    virtual string config_dir();

    virtual chain_rpc_api_proxy *new_chain_api(eosio::chain::controller *c);
    virtual trace_api_proxy *new_trace_api_proxy(void *chain, string& trace_dir, uint32_t slice_stride, int32_t minimum_irreversible_history_blocks, int32_t minimum_uncompressed_irreversible_history_blocks, uint32_t compression_seek_point_stride);

    virtual void set_log_level(string& logger_name, int level);
    virtual int get_log_level(string& logger_name);

    virtual void enable_deep_mind(void *controller);
private:
    string _chain_config;
};

class ipyeos_proxy {
    public:
        ipyeos_proxy(eos_cb *cb);
        virtual ~ipyeos_proxy();

        virtual apply_context_proxy *get_apply_context_proxy();
        virtual transaction_proxy *get_transaction_proxy();
        virtual vm_api_proxy *get_vm_api_proxy();
        virtual snapshot_proxy *new_snapshot_proxy(void *chain);
        virtual database_proxy *new_database_proxy(void *db_ptr, bool attached);
        virtual chainbase::database *new_database(const string& dir, bool read_only, uint64_t shared_file_size, bool allow_dirty);
        virtual block_log_proxy *new_block_log_proxy(string& block_log_dir);
        virtual chain_proxy* chain_new(string& config, string& _genesis, string& _chain_id, string& protocol_features_dir, string& snapshot_dir, string& debug_producer_key);
        virtual void chain_free(chain_proxy* api);

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

        virtual void set_worker_process(bool worker_process);
        virtual bool is_worker_process();

        virtual string create_key(string &key_type);
        virtual string get_public_key(string &priv_key);

        virtual string get_debug_producer_key(void *id);

        virtual string sign_digest(string &digest, string &priv_key);

        virtual bool base58_to_bytes(const string& s, vector<char>& out);
        virtual bool bytes_to_base58(const char* data, size_t data_size, string& out);

        eos_cb *cb;
    private:
        map<eosio::chain::controller*, chain_proxy*> chain_proxy_map;
        map<void *, string> chain_config_map;
        string last_error;
        std::shared_ptr<apply_context_proxy> _apply_context_proxy;
        bool debug_enabled = false;
};

typedef void (*fn_init_ipyeos_proxy)(ipyeos_proxy *proxy);
extern "C" ipyeos_proxy *get_ipyeos_proxy();
