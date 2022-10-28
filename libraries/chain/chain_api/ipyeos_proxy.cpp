#include <optional>
#include <fc/log/logger.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/abi_def.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/apply_context.hpp>

#include <fc/io/json.hpp>
#include <dlfcn.h>

#include "ipyeos_proxy.hpp"
#include "chain_macro.hpp"
#include "native_object.hpp"
#include "apply_context_proxy.hpp"

using namespace eosio::chain;


static string last_error;

string& get_last_error() {
    return last_error;
}

void set_last_error(string& error) {
    last_error = error;
}

string& ipyeos_proxy::get_last_error() {
    return ::get_last_error();
}

void ipyeos_proxy::set_last_error(string& error) {
    ::set_last_error(error);
}

ipyeos_proxy::ipyeos_proxy() {
    this->_apply_context_proxy = std::make_shared<apply_context_proxy>();
}

ipyeos_proxy::~ipyeos_proxy() {
}

apply_context_proxy *ipyeos_proxy::get_apply_context_proxy() {
    return _apply_context_proxy.get();
}

vm_api_proxy *ipyeos_proxy::get_vm_api_proxy() {
    return _apply_context_proxy->get_vm_api_proxy();
}

chain_proxy* ipyeos_proxy::chain_new(string& config, string& _genesis, string& protocol_features_dir, string& snapshot_dir) {
    chain_proxy *proxy = new chain_proxy();
    proxy->init(config, _genesis, protocol_features_dir, snapshot_dir);
    chain_proxy_map[proxy->chain()] = proxy;
    return proxy;
}

void ipyeos_proxy::chain_free(chain_proxy* c) {
    try {
        if (!c) {
            return;
        }
        auto itr = chain_proxy_map.find(c->chain());
        if (itr != chain_proxy_map.end()) {
            chain_proxy_map.erase(itr);
        }
        delete c;
    } CATCH_AND_LOG_EXCEPTION();
}

void ipyeos_proxy::set_log_level(string& logger_name, int level) {
    fc::logger::get(logger_name).set_log_level(fc::log_level(level));
}

void ipyeos_proxy::pack_abi(string& abi, vector<char>& packed_obj)
{
    try {
        auto _abi = fc::json::from_string(abi).as<abi_def>();
        packed_obj = fc::raw::pack<abi_def>(_abi);
    } CATCH_AND_LOG_EXCEPTION();
}

void ipyeos_proxy::pack_native_object(int type, string& msg, vector<char>& packed_obj) {
    pack_native_object_(type, msg, packed_obj);
}

void ipyeos_proxy::unpack_native_object(int type, string& packed_obj, string& result) {
    unpack_native_object_(type, packed_obj, result);
}


uint64_t ipyeos_proxy::s2n(string& s) {
    return eosio::chain::name(s).to_uint64_t();
}

string ipyeos_proxy::n2s(uint64_t n) {
    return eosio::chain::name(n).to_string();
}

bool ipyeos_proxy::call_native_contract(uint64_t receiver, uint64_t first_receiver, uint64_t action) {
    auto ptr = &this->get_apply_context_proxy()->get_context()->control;
    auto itr = chain_proxy_map.find(ptr);
    chain_proxy* proxy = itr->second;
    return proxy->call_native_contract(receiver, first_receiver, action);
}

void ipyeos_proxy::enable_debug(bool debug) {
    this->debug_enabled = debug;
}

bool ipyeos_proxy::is_debug_enabled() {
    return this->debug_enabled;
}

string ipyeos_proxy::create_key(string &key_type) {
    if(key_type.empty()) {
        key_type = string("K1");
    }

    private_key_type priv_key;
    if(key_type == "K1") {
        priv_key = fc::crypto::private_key::generate<fc::ecc::private_key_shim>();
    } else if(key_type == "R1") {
        priv_key = fc::crypto::private_key::generate<fc::crypto::r1::private_key_shim>();
    } else {
        EOS_THROW(eosio::chain::unsupported_key_type_exception, "Key type \"${kt}\" not supported by software wallet", ("kt", key_type));
    }
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "{\"public\":\"%s\", \"private\": \"%s\"}", priv_key.get_public_key().to_string().c_str(), priv_key.to_string().c_str());

    return string(buf);
}

string ipyeos_proxy::get_public_key(string &priv_key) {
    try {
        auto priv = fc::variant(priv_key).as<fc::crypto::private_key>();
        return priv.get_public_key().to_string();
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string ipyeos_proxy::sign_digest(string &priv_key, string &digest) {
    try {
        auto _priv_key = fc::variant(priv_key).as<fc::crypto::private_key>();
        digest_type _digest = fc::variant(digest).as<eosio::chain::digest_type>();
        return fc::json::to_string(_priv_key.sign(_digest), fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}
