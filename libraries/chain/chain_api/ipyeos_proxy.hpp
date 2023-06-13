#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

#include "chain_proxy.hpp"
#include "database_proxy.hpp"
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

typedef chain_rpc_api_proxy *(*fn_new_chain_api)(eosio::chain::controller *c);

class eos_cb {
public:
    eos_cb();
    virtual ~eos_cb();

    virtual int init(int argc, char** argv);
    virtual int exec();
    virtual int exec_once();
    virtual void quit();
    virtual void* post(void* (*fn)(void *), void *args);
    virtual void *get_database();
};

class ipyeos_proxy {
    public:
        ipyeos_proxy(eos_cb *cb);
        virtual ~ipyeos_proxy();

        virtual apply_context_proxy *get_apply_context_proxy();
        virtual vm_api_proxy *get_vm_api_proxy();
        virtual database_proxy *new_database_proxy(void *db);
        virtual void set_log_level(string& logger_name, int level);
        
        virtual chain_proxy* chain_new(string& config, string& _genesis, string& protocol_features_dir, string& snapshot_dir);
        virtual void chain_free(chain_proxy* api);

        virtual void pack_abi(string& abi, vector<char>& packed_message);

        virtual void pack_native_object(int type, string& msg, vector<char>& packed_message);
        virtual void unpack_native_object(int type, string& packed_message, string& msg);

        virtual string& get_last_error();
        virtual void set_last_error(string& error);

        virtual uint64_t s2n(string& name);
        virtual string n2s(uint64_t n);

        virtual bool call_native_contract(uint64_t receiver, uint64_t first_receiver, uint64_t action);

        virtual void enable_debug(bool debug);
        virtual bool is_debug_enabled();

        virtual string create_key(string &key_type);
        virtual string get_public_key(string &priv_key);

        virtual string sign_digest(string &digest, string &priv_key);

        fn_new_chain_api new_chain_api = nullptr;

        eos_cb *cb;
    private:
        map<eosio::chain::controller*, chain_proxy*> chain_proxy_map;
        string last_error;
        std::shared_ptr<apply_context_proxy> _apply_context_proxy;
        bool debug_enabled = false;
};

typedef void (*fn_init_ipyeos_proxy)(ipyeos_proxy *proxy);
extern "C" ipyeos_proxy *get_ipyeos_proxy();
