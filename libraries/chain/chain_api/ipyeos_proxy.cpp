#include <optional>
#include <fc/log/logger.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/abi_def.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/apply_context.hpp>

#include <eosio/chain/account_object.hpp>
#include <eosio/chain/code_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/eosio_contract.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/protocol_state_object.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/database_header_object.hpp>

#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/protocol_state_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/resource_limits_private.hpp>

#include <eosio/chain/index_event_listener.hpp>

#include <fc/io/json.hpp>
#include <dlfcn.h>

#include "ipyeos_proxy.hpp"
#include "chain_macro.hpp"
#include "native_object.hpp"
#include "apply_context_proxy.hpp"

using namespace eosio::chain::resource_limits;

using resource_index_set = index_set<
   resource_limits_index,
   resource_usage_index,
   resource_limits_state_index,
   resource_limits_config_index
>;

using authorization_index_set = index_set<
    permission_index,
    permission_usage_index,
    permission_link_index
>;

using controller_index_set = index_set<
   account_index,
   account_metadata_index,
   account_ram_correction_index,
   global_property_multi_index,
   protocol_state_multi_index,
   dynamic_global_property_multi_index,
   block_summary_multi_index,
   transaction_multi_index,
   generated_transaction_multi_index,
   table_id_multi_index,
   code_index,
   database_header_multi_index
>;

using contract_database_index_set = index_set<
   key_value_index,
   index64_index,
   index128_index,
   index256_index,
   index_double_index,
   index_long_double_index
>;

using namespace eosio::chain;

static bool s_worker_process = false;
static vector<string> s_errors;

string get_last_error() {
    if (s_errors.size() > 0) {
        return s_errors.back();
    }
    return "";
}

void set_last_error(string& error) {
    s_errors.push_back(error);
}

string ipyeos_proxy::get_last_error() {
    return ::get_last_error();
}

void ipyeos_proxy::set_last_error(string& error) {
    ::set_last_error(error);
}

string ipyeos_proxy::get_chain_config(void *controller) {
    FC_ASSERT(controller != nullptr, "chain is null");
    return chain_config_map[controller];
}

bool ipyeos_proxy::set_chain_config(void *controller, const string& config) {
    FC_ASSERT(controller != nullptr, "chain is null");
    chain_config_map[controller] = config;
    return true;
}

ipyeos_proxy::ipyeos_proxy(eos_cb *cb) {
    this->cb = cb;
    max_database_cpu_billing_time_us = default_max_database_cpu_billing_time_us;
    this->_apply_context_proxy = std::make_shared<apply_context_proxy>();
}

ipyeos_proxy::~ipyeos_proxy() {
}

signed_transaction_proxy *ipyeos_proxy::transaction_proxy_new(
    uint32_t expiration,
    const char* ref_block_id,
    size_t ref_block_id_size,
    uint32_t max_net_usage_words, //fc::unsigned_int
    uint8_t  max_cpu_usage_ms,    //
    uint32_t delay_sec            //fc::unsigned_int
) {
    return new signed_transaction_proxy(expiration, ref_block_id, ref_block_id_size, max_net_usage_words, max_cpu_usage_ms, delay_sec);
}

signed_transaction_proxy *ipyeos_proxy::transaction_proxy_new_ex(signed_transaction_ptr *transaction_ptr) {
    return new signed_transaction_proxy(*transaction_ptr);
}

bool ipyeos_proxy::transaction_proxy_free(void *transaction_proxy_ptr) {
    if (transaction_proxy_ptr) {
        delete static_cast<signed_transaction_proxy *>(transaction_proxy_ptr);
        return true;
    }
    return false;
}

packed_transaction_proxy *ipyeos_proxy::packed_transaction_proxy_new(packed_transaction_ptr *_packed_transaction_ptr, bool attach) {
    return new packed_transaction_proxy(_packed_transaction_ptr, attach);
}

packed_transaction_proxy *ipyeos_proxy::packed_transaction_proxy_new_ex(const char *raw_packed_tx, size_t raw_packed_tx_size) {
    try {
        auto trx = fc::raw::unpack<packed_transaction>(raw_packed_tx, raw_packed_tx_size);
        auto ptr = new packed_transaction_ptr(std::make_shared<packed_transaction>(std::move(trx)));
        return new packed_transaction_proxy(ptr, true);
    } CATCH_AND_LOG_EXCEPTION()
    return nullptr;
}

packed_transaction_proxy *ipyeos_proxy::packed_transaction_proxy_new_ex_ex(signed_transaction_proxy *signed_transaction_proxy_ptr, bool compressed) {
    return new packed_transaction_proxy(signed_transaction_proxy_ptr, compressed);
}

bool ipyeos_proxy::packed_transaction_proxy_free(packed_transaction_proxy *packed_transaction_proxy_ptr) {
    if (packed_transaction_proxy_ptr) {
        delete packed_transaction_proxy_ptr;
        return true;
    }
    return false;
}

apply_context_proxy *ipyeos_proxy::get_apply_context_proxy() {
    return _apply_context_proxy.get();
}

vm_api_proxy *ipyeos_proxy::get_vm_api_proxy() {
    return _apply_context_proxy->get_vm_api_proxy();
}

snapshot_proxy *ipyeos_proxy::new_snapshot_proxy(void *chain) {
    try {
        return new snapshot_proxy(chain);
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

database_proxy *ipyeos_proxy::new_database_proxy(void *db_ptr, bool attach) {
    return new database_proxy(static_cast<chainbase::database *>(db_ptr), attach);
}

chainbase::database *ipyeos_proxy::new_database(const string& dir, bool read_only, uint64_t shared_file_size, bool allow_dirty) {
    try {
        auto _read_only = read_only ? chainbase::database::read_only : chainbase::database::read_write;
        auto *db = new chainbase::database(dir, _read_only, shared_file_size, allow_dirty);

        controller_index_set::add_indices(*db);
        contract_database_index_set::add_indices(*db);

        authorization_index_set::add_indices(*db);
        resource_index_set::add_indices(*db);
        return db;
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

block_log_proxy *ipyeos_proxy::new_block_log_proxy(string& block_log_dir) {
    try {
        return new block_log_proxy(block_log_dir);
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

chain_proxy* ipyeos_proxy::chain_new(string& config, string& _genesis, string& chain_id, string& protocol_features_dir, string& snapshot_dir, string& debug_producer_key) {
    try {
        chain_proxy *proxy = new chain_proxy();
        proxy->init(config, _genesis, chain_id, protocol_features_dir, snapshot_dir, debug_producer_key);
        chain_proxy_map[proxy->get_controller()] = proxy;
        return proxy;
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

chain_proxy* ipyeos_proxy::chain_attach(void *controller) {
    chain_proxy *proxy = new chain_proxy();
    proxy->attach(static_cast<eosio::chain::controller*>(controller));
    return proxy;
}

void ipyeos_proxy::chain_free(chain_proxy* c) {
    try {
        if (!c) {
            return;
        }
        auto itr = chain_proxy_map.find(c->get_controller());
        if (itr != chain_proxy_map.end()) {
            chain_proxy_map.erase(itr);
        }
        delete c;
    } CATCH_AND_LOG_EXCEPTION();
}

key_value_index_proxy* ipyeos_proxy::new_key_value_index_proxy() {
    return new key_value_index_proxy();
}

bool ipyeos_proxy::free_key_value_index_proxy(void *ptr) {
    if (ptr) {
        delete static_cast<key_value_index_proxy*>(ptr);
        return true;
    }
    return false;
}

u64_double_index_proxy* ipyeos_proxy::new_u64_double_index_proxy(int sort_type) {
    return new u64_double_index_proxy(sort_type);
}

bool ipyeos_proxy::free_u64_double_index_proxy(void *ptr) {
    if (ptr) {
        delete static_cast<u64_double_index_proxy*>(ptr);
        return true;
    }
    return false;
}

read_write_lock_proxy* ipyeos_proxy::read_write_lock_proxy_new(const string& mutex_name) {
    return new read_write_lock_proxy(mutex_name);
}

bool ipyeos_proxy::read_write_lock_proxy_free(void *ptr) {
    if (ptr) {
        delete static_cast<read_write_lock_proxy*>(ptr);
        return true;
    }
    return false;
}

block_state_proxy *ipyeos_proxy::block_state_proxy_new(void *ptr) {
    return new block_state_proxy(*static_cast<block_state_ptr*>(ptr));
}

bool ipyeos_proxy::block_state_proxy_free(void *block_state_proxy_ptr) {
    if (block_state_proxy_ptr) {
        delete static_cast<block_state_proxy*>(block_state_proxy_ptr);
        return true;
    }
    return false;
}

transaction_trace_proxy *ipyeos_proxy::transaction_trace_proxy_new(transaction_trace_ptr *_transaction_trace_ptr, bool attach) {
    return new transaction_trace_proxy(_transaction_trace_ptr, attach);
}

transaction_trace_proxy *ipyeos_proxy::transaction_trace_proxy_new_ex(const char *raw_packed_transaction_trace, size_t raw_packed_transaction_trace_size) {
    try {
        auto ptt = fc::raw::unpack<transaction_trace>(raw_packed_transaction_trace, raw_packed_transaction_trace_size);
        auto ptr = new transaction_trace_ptr(std::make_shared<transaction_trace>(std::move(ptt)));
        return new transaction_trace_proxy(ptr, true);
    } CATCH_AND_LOG_EXCEPTION()
    return nullptr;
}

bool ipyeos_proxy::transaction_trace_proxy_free(transaction_trace_proxy *transaction_trace_proxy_ptr) {
    if (transaction_trace_proxy_ptr) {
        delete transaction_trace_proxy_ptr;
        return true;
    }
    return false;
}

signed_block_proxy *ipyeos_proxy::signed_block_proxy_new(signed_block_ptr *_signed_block_ptr) {
    return new signed_block_proxy(*_signed_block_ptr);
}

signed_block_proxy *ipyeos_proxy::signed_block_proxy_new_ex(const char *raw_signed_block, size_t raw_signed_block_size) {
    try {
        auto psb = fc::raw::unpack<signed_block>(raw_signed_block, raw_signed_block_size);
        auto ptr = new signed_block_ptr(std::make_shared<signed_block>(std::move(psb)));
        return new signed_block_proxy(ptr);
    } CATCH_AND_LOG_EXCEPTION()
    return nullptr;
}

signed_block_proxy *ipyeos_proxy::signed_block_proxy_attach(signed_block_ptr *_signed_block_ptr) {
    return new signed_block_proxy(_signed_block_ptr);
}

bool ipyeos_proxy::signed_block_proxy_free(void *signed_block_proxy_ptr) {
    if (signed_block_proxy_ptr) {
        delete static_cast<signed_block_proxy*>(signed_block_proxy_ptr);
        return true;
    }
    return false;
}

string ipyeos_proxy::extract_chain_id_from_snapshot(string& snapshot_dir) {
    try {
        auto infile = std::ifstream(snapshot_dir, (std::ios::in | std::ios::binary));
        istream_snapshot_reader reader(infile);
        reader.validate();
        auto chain_id = controller::extract_chain_id(reader);
        infile.close();
        return chain_id.str();        
    } CATCH_AND_LOG_EXCEPTION();
    return "";
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
    try {
        return eosio::chain::name(s).to_uint64_t();
    } CATCH_AND_LOG_EXCEPTION();
    return 0;
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

void ipyeos_proxy::enable_adjust_cpu_billing(bool enabled) {
    adjust_cpu_billing_enabled = enabled;
}

bool ipyeos_proxy::is_adjust_cpu_billing_enabled() {
    return adjust_cpu_billing_enabled;
}

void ipyeos_proxy::set_max_database_cpu_billing_time_us(int64_t us) {
    max_database_cpu_billing_time_us = us;
}

int64_t ipyeos_proxy::get_max_database_cpu_billing_time_us() {
    return max_database_cpu_billing_time_us;
}

void ipyeos_proxy::set_worker_process(bool worker_process) {
    s_worker_process = worker_process;
}

bool ipyeos_proxy::is_worker_process() {
    return s_worker_process;
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

string ipyeos_proxy::sign_digest(string &digest, string &priv_key) {
    try {
        auto _priv_key = fc::variant(priv_key).as<fc::crypto::private_key>();
        digest_type _digest = fc::variant(digest).as<eosio::chain::digest_type>();
        return _priv_key.sign(_digest).to_string();
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string ipyeos_proxy::get_debug_producer_key(void *id) {
    for (auto& chain_proxy : chain_proxy_map) {
        if ((void *)&chain_proxy.second->get_controller()->db() == id) {
            return chain_proxy.second->get_debug_producer_key();
        }
    }
    return "";
}


bool ipyeos_proxy::base58_to_bytes(const string& s, vector<char>& out) {
    try {
        out = fc::from_base58(s);
        return true;
    } CATCH_AND_LOG_EXCEPTION();
    return false;
}

bool ipyeos_proxy::bytes_to_base58( const char* data, size_t data_size, string& out) {
    try {
        out = fc::to_base58(data, data_size, fc::yield_function_t());
        return true;
    } CATCH_AND_LOG_EXCEPTION();
    return false;
}

void ipyeos_proxy::ripemd160(const char *data, size_t data_size, vector<char>& hash) {
    auto h = fc::ripemd160::hash(data, data_size);
    hash = vector<char>(h.data(), h.data() + h.data_size());
}
