#include <eosio/chain/controller.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/webassembly/interface.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/resource_limits.hpp>

#include <boost/signals2/connection.hpp>

#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>

#include <dlfcn.h>

#include "chain_manager.hpp"
#include "chain_macro.hpp"
#include "ipyeos_proxy.hpp"

extern "C" ipyeos_proxy *get_ipyeos_proxy();

using namespace std;
using namespace boost;

using namespace eosio;
using namespace eosio::chain;

using boost::signals2::scoped_connection;

static std::map<std::filesystem::path, std::shared_ptr<native_contract>> s_native_libraries;

struct get_info_results {
    string                               server_version;
    chain::chain_id_type                 chain_id = chain_id_type::empty_chain_id();
    uint32_t                             head_block_num = 0;
    uint32_t                             last_irreversible_block_num = 0;
    chain::block_id_type                 last_irreversible_block_id;
    chain::block_id_type                 head_block_id;
    fc::time_point                       head_block_time;
    account_name                         head_block_producer;

    uint64_t                             virtual_block_cpu_limit = 0;
    uint64_t                             virtual_block_net_limit = 0;

    uint64_t                             block_cpu_limit = 0;
    uint64_t                             block_net_limit = 0;
    //string                               recent_slots;
    //double                               participation_rate = 0;
    std::optional<string>                server_version_string;
    std::optional<uint32_t>              fork_db_head_block_num;
    std::optional<chain::block_id_type>  fork_db_head_block_id;
    std::optional<string>                server_full_version_string;
    std::optional<uint64_t>              total_cpu_weight;
    std::optional<uint64_t>              total_net_weight;
    std::optional<uint32_t>              earliest_available_block_num;
    std::optional<fc::time_point>        last_irreversible_block_time;
};

FC_REFLECT(get_info_results,
           (server_version)(chain_id)(head_block_num)(last_irreversible_block_num)(last_irreversible_block_id)
           (head_block_id)(head_block_time)(head_block_producer)
           (virtual_block_cpu_limit)(virtual_block_net_limit)(block_cpu_limit)(block_net_limit)
           (server_version_string)(fork_db_head_block_num)(fork_db_head_block_id)(server_full_version_string)
           (total_cpu_weight)(total_net_weight)(earliest_available_block_num)(last_irreversible_block_time))

class chain_proxy_impl {
private:
    string debug_public_key;
    std::unique_ptr<eosio::chain::chain_manager> cm;
    eosio::chain::controller *c;
    std::shared_ptr<chain_rpc_api_proxy> _api_proxy;
    std::map<std::string, std::shared_ptr<eosio::chain::abi_serializer>> abi_cache;
    string last_error;
    bool attached = false;

    std::map<uint64_t, std::shared_ptr<native_contract>> native_contracts;

    std::optional<scoped_connection> accepted_block_connection;
    std::optional<scoped_connection> irreversible_block_connection;

    fn_block_event_listener accepted_block_event_listener;
    void *accepted_block_event_listener_data = nullptr;

    fn_block_event_listener irreversible_block_event_listener;
    void *irreversible_block_event_listener_data = nullptr;

    get_info_results info;
    string info_json;

public:
    chain_proxy_impl() {
    }

    ~chain_proxy_impl() {
    }

    int init(string& config, string& _genesis, string& chain_id, string& protocol_features_dir, string& snapshot_file, string& debug_producer_key) {
        this->set_debug_producer_key(debug_producer_key);
        cm = std::make_unique<chain_manager>(config, _genesis, chain_id, protocol_features_dir, snapshot_file);
        this->cm->init();
        this->c = this->cm->get_controller();

        chain_rpc_api_proxy *api = get_ipyeos_proxy()->cb->new_chain_api(this->c);
        this->_api_proxy = std::shared_ptr<chain_rpc_api_proxy>(api);
        return 1;
    }

    int attach(eosio::chain::controller* c) {
        this->c = c;
        chain_rpc_api_proxy *api = get_ipyeos_proxy()->cb->new_chain_api(this->c);
        this->_api_proxy = std::shared_ptr<chain_rpc_api_proxy>(api);
        this->attached = true;
        return 1;
    }

    controller* get_controller() {
        return this->c;
    }

    const chainbase::database* get_database() {
        return &this->c->db();
    }

    chain_rpc_api_proxy* api_proxy() {
        return this->_api_proxy.get();
    }

    void say_hello() {
        printf("hello,world from chain_proxy\n");
    }

    void chain_id(string& result) {
        try {
            result = c->get_chain_id().str();
        } CATCH_AND_LOG_EXCEPTION();
    }

    bool startup(bool initdb) {
        try {
            cm->startup(initdb);
            update_chain_info();
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        cm.reset();
        return false;
    }

    bool set_accepted_block_event_listener(fn_block_event_listener _listener, void *user_data) {
        accepted_block_event_listener = _listener;
        accepted_block_event_listener_data = user_data;
        accepted_block_connection = c->accepted_block.connect([this](const block_state_ptr& blk) {
            if (accepted_block_event_listener) {
                auto bsp = new block_state_proxy(blk);
                accepted_block_event_listener(bsp, accepted_block_event_listener_data);
                delete bsp;
            }
            update_chain_info();
        });
        return true;
    }

    bool set_irreversible_block_event_listener(fn_block_event_listener _listener, void *user_data) {
        irreversible_block_event_listener = _listener;
        irreversible_block_event_listener_data = user_data;
        irreversible_block_connection = c->accepted_block.connect([this](const block_state_ptr& blk) {
            if (accepted_block_event_listener) {
                auto bsp = new block_state_proxy(blk);
                irreversible_block_event_listener(bsp, irreversible_block_event_listener_data);
                delete bsp;
            }
        });
        return true;
    }

    void update_chain_info() {
        const auto& rm = c->get_resource_limits_manager();
        info = {
            "",
            c->get_chain_id(),
            c->head_block_num(),
            c->last_irreversible_block_num(),
            c->last_irreversible_block_id(),
            c->head_block_id(),
            c->head_block_time(),
            c->head_block_producer(),
            rm.get_virtual_block_cpu_limit(),
            rm.get_virtual_block_net_limit(),
            rm.get_block_cpu_limit(),
            rm.get_block_net_limit(),
            "", //app().version_string(),
            c->fork_db_head_block_num(),
            c->fork_db_head_block_id(),
            "", //app().full_version_string(),
            rm.get_total_cpu_weight(),
            rm.get_total_net_weight(),
            c->earliest_available_block_num(),
            c->last_irreversible_block_time()
        };
        info_json = fc::json::to_string(info, fc::time_point::maximum());
    }

    string get_info() {
        return info_json;
    }

    block_timestamp_type calculate_pending_block_time(const eosio::chain::controller& chain) {
        const fc::time_point     now   = fc::time_point::now();
        const fc::time_point     base  = std::max<fc::time_point>(now, chain.head_block_time());
        return block_timestamp_type(base).next();
    }

    int start_block(int64_t block_time_since_epoch_ms, uint16_t confirm_block_count, string& _new_features) {
        try {
            fc::time_point time;
            if (0 == block_time_since_epoch_ms) {
                time = calculate_pending_block_time(*c);
            } else {
                time = fc::time_point(fc::microseconds(block_time_since_epoch_ms * 1000));
            }

            if (_new_features.size()) {
                auto new_features = fc::json::from_string(_new_features).as<vector<digest_type>>();
                c->start_block(block_timestamp_type(time), confirm_block_count, new_features, controller::block_status::incomplete);
            } else {
                c->start_block(block_timestamp_type(time), confirm_block_count, {}, controller::block_status::incomplete);
            }
            return 1;
        } CATCH_AND_LOG_EXCEPTION();
        return 0;
    }

    int abort_block() {
    try {
            c->abort_block();
            return 1;
    } CATCH_AND_LOG_EXCEPTION();
    return 0;
    }

    bool finalize_block(string& _priv_keys) {
        try {
            auto priv_keys = fc::json::from_string(_priv_keys).as<vector<string>>();
            controller::block_report br;
            c->finalize_block(br, [&]( const digest_type d ) {
                vector<signature_type> sigs;
                for (auto& key: priv_keys) {
                    auto priv_key = private_key_type(key);
                    sigs.emplace_back(priv_key.sign(d));
                }
                return sigs;
            } );
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    bool commit_block() {
        try {
            c->commit_block();
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    string get_block_id_for_num(uint32_t block_num ) {
        try {
            auto id = c->get_block_id_for_num(block_num);
            return id.str();
        } CATCH_AND_LOG_EXCEPTION();
        return "";
    }

    string get_global_properties() {
        auto& obj = c->get_global_properties();
        return fc::json::to_string(obj, fc::time_point::maximum());
    }

    string get_dynamic_global_properties() {
        try {
            auto& obj = c->get_dynamic_global_properties();
            return fc::json::to_string(obj, fc::time_point::maximum());
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string get_actor_whitelist() {
        return fc::json::to_string(c->get_actor_whitelist(), fc::time_point::maximum());
    }

    string get_actor_blacklist() {
        return fc::json::to_string(c->get_actor_blacklist(), fc::time_point::maximum());
    }

    string get_contract_whitelist() {
        return fc::json::to_string(c->get_contract_whitelist(), fc::time_point::maximum());
    }

    string get_contract_blacklist() {
        return fc::json::to_string(c->get_contract_blacklist(), fc::time_point::maximum());
    }

    string get_action_blacklist() {
        return fc::json::to_string(c->get_action_blacklist(), fc::time_point::maximum());
    }

    string get_key_blacklist() {
        return fc::json::to_string(c->get_key_blacklist(), fc::time_point::maximum());
    }

    void set_actor_whitelist(string& params) {
        auto whitelist = fc::json::from_string(params).as<flat_set<account_name>>();
        c->set_actor_whitelist(whitelist);
    }

    void set_actor_blacklist(string& params) {
        auto whitelist = fc::json::from_string(params).as<flat_set<account_name>>();
        c->set_actor_blacklist(whitelist);
    }

    void set_contract_whitelist(string& params) {
        auto whitelist = fc::json::from_string(params).as<flat_set<account_name>>();
        c->set_contract_whitelist(whitelist);
    }

    void set_action_blacklist(string& params) {
        auto whitelist = fc::json::from_string(params).as<flat_set<pair<account_name, action_name>>>();
        c->set_action_blacklist(whitelist);
    }

    void set_key_blacklist(string& params) {
        auto whitelist = fc::json::from_string(params).as<flat_set<public_key_type>>();
        c->set_key_blacklist(whitelist);
    }

    uint32_t head_block_num() {
        return c->head_block_num();
    }

    int64_t head_block_time() {
        return c->head_block_time().time_since_epoch().count();
    }

    string head_block_id() {
        return c->head_block_id().str();
    }

    string head_block_producer() {
        return c->head_block_producer().to_string();
    }

    string head_block_header() {
        return fc::json::to_string(c->head_block_header(), fc::time_point::maximum());
    }

    string head_block_state() {
        return fc::json::to_string(c->head_block_state(), fc::time_point::maximum());
    }

    uint32_t earliest_available_block_num() {
        return c->earliest_available_block_num();
    }

    int64_t last_irreversible_block_time() {
        return c->last_irreversible_block_time().time_since_epoch().count();
    }

    uint32_t fork_db_head_block_num() {
        return c->fork_db_head_block_num();
    }

    string fork_db_head_block_id() {
        return c->fork_db_head_block_id().str();
    }

    uint32_t fork_db_pending_head_block_num() {
        // try {
        //     return c->fork_db_pending_head_block_num();
        // } CATCH_AND_LOG_EXCEPTION()
        return 0;
    }

    string fork_db_pending_head_block_id() {
        return "";
        // return c->fork_db_pending_head_block_id().str();
    }

    string fork_db_pending_head_block_time() {
        return "";
        // return c->fork_db_pending_head_block_time();
    }

    string fork_db_pending_head_block_producer() {
        return "";
        // return c->fork_db_pending_head_block_producer().to_string();
    }

    int64_t pending_block_time() {
        return c->pending_block_time().time_since_epoch().count();
    }

    string pending_block_producer() {
        return c->pending_block_producer().to_string();
    }

    string pending_producer_block_id() {
        auto id = c->pending_producer_block_id();
        if (id) {
            return id->str();
        }
        return "";
    }

    // string get_pending_trx_receipts() {
    //     return fc::json::to_string(c->get_pending_trx_receipts(), fc::time_point::maximum());
    // }

    string active_producers() {
        return fc::json::to_string(c->active_producers(), fc::time_point::maximum());
    }

    string pending_producers() {
        return fc::json::to_string(c->pending_producers(), fc::time_point::maximum());
    }

    string proposed_producers() {
        return fc::json::to_string(c->proposed_producers(), fc::time_point::maximum());
    }

    uint32_t last_irreversible_block_num() {
        return c->last_irreversible_block_num();
    }

    string last_irreversible_block_id() {
        try {
            return c->last_irreversible_block_id().str();
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string fetch_block_by_number(uint32_t block_num) {
        try {
            auto block_ptr = c->fetch_block_by_number(block_num);
            if (!block_ptr) {
                return "";
            }
            auto _raw_block = fc::raw::pack<eosio::chain::signed_block>(*block_ptr);
            return string(_raw_block.data(), _raw_block.size());
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string fetch_block_by_id(string& params) {
        try {
            auto block_id = fc::variant(params).as<block_id_type>();
            auto block_ptr = c->fetch_block_by_id(block_id);
            if (!block_ptr) {
                return "";
            }
            auto _raw_block = fc::raw::pack<eosio::chain::signed_block>(*block_ptr);
            return string(_raw_block.data(), _raw_block.size());
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string fetch_block_state_by_number(uint32_t block_num) {
        try {
            auto block_ptr = c->fetch_block_state_by_number(block_num);
            if (!block_ptr) {
                return "";
            }
            auto _raw_block_state = fc::raw::pack<eosio::chain::block_state>(*block_ptr);
            return string(_raw_block_state.data(), _raw_block_state.size());
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string fetch_block_state_by_id(string& params) {
        try {
            auto block_id = fc::json::from_string(params).as<block_id_type>();
            auto block_ptr = c->fetch_block_state_by_id(block_id);
            if (!block_ptr) {
                return "";
            }
            auto _raw_block_state = fc::raw::pack<eosio::chain::block_state>(*block_ptr);
            return string(_raw_block_state.data(), _raw_block_state.size());
        } CATCH_AND_LOG_EXCEPTION()
        return "";
    }

    string calculate_integrity_hash() {
        auto hash = c->calculate_integrity_hash();
        return hash.str();
    }

    /*
            void write_snapshot( const snapshot_writer_ptr& snapshot )const;
    */

    bool sender_avoids_whitelist_blacklist_enforcement(string& sender) {
        return c->sender_avoids_whitelist_blacklist_enforcement(account_name(sender));
    }

    bool check_actor_list(string& param) {
        try {
            auto _param = fc::json::from_string(param).as<flat_set<account_name>>();
            c->check_actor_list(_param);
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool check_contract_list(string& param) {
        try {
            auto _param = fc::variant(param).as<account_name>();
            c->check_contract_list(_param);
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool check_action_list(string& code, string& action) {
        try {
            c->check_action_list(name(code), name(action));
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool check_key_list(string& param) {
        try {
            auto _param = fc::json::from_string(param).as<public_key_type>();
            c->check_key_list(_param);
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool is_building_block() {
        return c->is_building_block();
    }

    bool is_speculative_block() {
        return c->is_speculative_block();
    }

    bool is_ram_billing_in_notify_allowed() {
        return c->is_ram_billing_in_notify_allowed();
    }

    void add_resource_greylist(string& param) {
        c->add_resource_greylist(name(param));
    }

    void remove_resource_greylist(string& param) {
        c->remove_resource_greylist(name(param));
    }

    bool is_resource_greylisted(string& param) {
        return c->is_resource_greylisted(name(param));
    }

    string get_resource_greylist() {
        return fc::json::to_string(c->get_resource_greylist(), fc::time_point::maximum());
    }

    string get_config() {
        return "";
        // return fc::json::to_string(c->get_config(), fc::time_point::maximum());
    }

    bool validate_expiration(string& param) {
        try {
            auto trx = fc::json::from_string(param).as<transaction>();
            c->validate_expiration(trx);
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool validate_tapos(string& param) {
        try {
            auto trx = fc::json::from_string(param).as<transaction>();
            c->validate_tapos(trx);
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    bool validate_db_available_size() {
        try {
            c->validate_db_available_size();
            return true;
        } CATCH_AND_LOG_EXCEPTION()
        return false;
    }

    // bool validate_reversible_available_size() {
    //     try {
    //         c->validate_reversible_available_size();
    //         return true;
    //     } CATCH_AND_LOG_EXCEPTION()
    //     return false;
    // }

    bool is_protocol_feature_activated(string& param) {
        auto digest = fc::variant(param).as<digest_type>();
        return c->is_protocol_feature_activated(digest);
    }

    bool is_builtin_activated(int feature) {
        return c->is_builtin_activated((builtin_protocol_feature_t)feature);
    }

    bool is_known_unexpired_transaction(string& param) {
        auto id = fc::json::from_string(param).as<transaction_id_type>();
        return c->is_known_unexpired_transaction(id);
    }

    int64_t set_proposed_producers(string& param) {
        auto id = fc::json::from_string(param).as<vector<producer_authority>>();
        return c->set_proposed_producers(id);
    }

    bool light_validation_allowed() {
        return c->light_validation_allowed();
    }

    bool skip_auth_check() {
        return c->skip_auth_check();
    }

    bool skip_db_sessions() {
        return c->skip_db_sessions();
    }

    bool skip_trx_checks() {
        return c->skip_trx_checks();
    }

    bool contracts_console() {
        return c->contracts_console();
    }

    int get_read_mode() {
        return (int)c->get_read_mode();
    }

    int get_validation_mode() {
        return (int)c->get_validation_mode();
    }

    void set_subjective_cpu_leeway(uint64_t leeway) {
        c->set_subjective_cpu_leeway(fc::microseconds(leeway));
    }

    void set_greylist_limit(uint32_t limit) {
        c->set_greylist_limit(limit);
    }

    uint32_t get_greylist_limit() {
        return c->get_greylist_limit();
    }

    void add_to_ram_correction(string& account, uint64_t ram_bytes) {
        c->add_to_ram_correction(account_name(account), ram_bytes);
    }

    bool all_subjective_mitigations_disabled() {
        return c->all_subjective_mitigations_disabled();
    }

    string get_scheduled_producer(string& _block_time) {
        auto block_time = fc::time_point::from_iso_string(_block_time);
        auto producer = c->head_block_state()->get_scheduled_producer(block_time);
        return fc::json::to_string(producer, fc::time_point::maximum());
    }

    void gen_transaction(bool json, string& _actions, int64_t expiration_sec, string& reference_block_id, string& _chain_id, bool compress, std::string& _private_keys, vector<char>& result) {
        try {
            signed_transaction trx;
            auto actions = fc::json::from_string(_actions).as<vector<eosio::chain::action>>();
            trx.actions = std::move(actions);
            trx.expiration = time_point_sec(expiration_sec);
            eosio::chain::block_id_type id(reference_block_id);
            trx.set_reference_block(id);
            trx.max_net_usage_words = 0;

            eosio::chain::chain_id_type chain_id(_chain_id);

            auto priv_keys = fc::json::from_string(_private_keys).as<vector<private_key_type>>();
            for (auto& key: priv_keys) {
                trx.sign(key, chain_id);
            }
            packed_transaction::compression_type type;
            if (compress) {
                type = packed_transaction::compression_type::zlib;
            } else {
                type = packed_transaction::compression_type::none;
            }
            if (json) {
                string s = fc::json::to_string(trx, fc::time_point::maximum());
                result = vector<char>(s.begin(), s.end());
            } else {
                auto packed_trx = packed_transaction(std::move(trx), type);
                auto v = fc::raw::pack(packed_trx);
                result = std::move(v);
            }
        } CATCH_AND_LOG_EXCEPTION();
    }

    bool push_transaction(const char *_packed_tx, size_t _packed_tx_size, int64_t block_deadline_ms, uint32_t billed_cpu_time_us, bool explicit_cpu_bill, uint32_t subjective_cpu_bill_us, bool read_only, string& result) {
        try {
            auto ptrx = std::make_shared<packed_transaction>();
            transaction_metadata_ptr ptrx_meta;
            fc::raw::unpack(_packed_tx, _packed_tx_size, *ptrx);
            if (read_only) {
                ptrx_meta = transaction_metadata::create_no_recover_keys(ptrx, transaction_metadata::trx_type::read_only);
            } else {
                ptrx_meta = transaction_metadata::recover_keys(ptrx, c->get_chain_id());
            }
        //    auto ptrx_meta = transaction_metadata::create_no_recover_keys( trx, transaction_metadata::trx_type::input );
            time_point _block_deadline;
            if (0 == block_deadline_ms) {
                _block_deadline = fc::time_point::maximum();
            } else {
                _block_deadline = fc::time_point(fc::microseconds(block_deadline_ms*1000));
            }
            auto max_trx_time = fc::microseconds::maximum();
            auto ret = c->push_transaction(ptrx_meta, _block_deadline, max_trx_time, billed_cpu_time_us, explicit_cpu_bill, subjective_cpu_bill_us);
            result = fc::json::to_string(ret, fc::time_point::maximum());
            if (ret->except) {
                return false;
            }
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    string get_scheduled_transactions() {
        vector<transaction_id_type> result;
        const auto& idx = c->db().get_index<generated_transaction_multi_index,by_delay>();
        auto itr = idx.begin();
        while( itr != idx.end() && itr->delay_until <= c->pending_block_time() ) {
            result.emplace_back(itr->trx_id);
            ++itr;
        }
        return fc::json::to_string(result, fc::time_point::maximum());
    }

    string get_scheduled_transaction(const char *sender_id, size_t sender_id_size, string& sender) {
        try {
            FC_ASSERT(sender_id_size == 16, "sender_id should be 16 bytes");
            unsigned __int128 _sender_id;
            memcpy(&_sender_id, sender_id, 16);
            // auto& generated_transaction_idx = c->db().get_index<generated_transaction_multi_index>();
            const auto* gto = c->db().find<generated_transaction_object,by_sender_id>(boost::make_tuple(account_name(sender), _sender_id));
            if ( gto ) {
                return fc::json::to_string(*gto, fc::time_point::maximum());
            }
        } CATCH_AND_LOG_EXCEPTION();
        return "";
    }

    string push_scheduled_transaction(string& scheduled_tx_id, string& deadline, uint32_t billed_cpu_time_us) {
        try {
            auto id = transaction_id_type(scheduled_tx_id);
            auto _deadline = fc::time_point::from_iso_string(deadline);
            
            auto max_trx_time = fc::microseconds::maximum();

            auto ret = c->push_scheduled_transaction(id, _deadline, max_trx_time, billed_cpu_time_us, false);
            return fc::json::to_string(ret, fc::time_point::maximum());
        } CATCH_AND_LOG_EXCEPTION();
        return "";
    }

    bool push_block_from_block_log(void *block_log_ptr, uint32_t block_num) {
        try {
            auto block_log = static_cast<eosio::chain::block_log*>(block_log_ptr);
            auto b = block_log->read_block_by_num(block_num);
            if (!b) {
                return false;
            }
            auto bsf = c->create_block_state_future(b->calculate_id(), b);
            controller::block_report br;
            c->push_block( br, bsf.get(), []( const branch_type& forked_branch ) {
                elog("forked_branch_callback not implemented");
            }, []( const transaction_id_type& id ) {
                // elog("trx_meta_cache_lookup ${id}", ("id", id));
                return nullptr;
            } );
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    bool push_block(const char *raw_block, size_t raw_block_size, string *block_statistics) {
        try {
            auto block = std::make_shared<signed_block>();
            fc::raw::unpack(raw_block, raw_block_size, *block);

            const auto& id = block->calculate_id();
            auto bsf = c->create_block_state_future(id, block);
            controller::block_report br;

            auto now = fc::time_point::now();

            c->push_block( br, bsf.get(), []( const branch_type& forked_branch ) {
                elog("forked_branch_callback not implemented");
            }, []( const transaction_id_type& id ) {
                // elog("trx_meta_cache_lookup ${id}", ("id", id));
                return nullptr;
            } );

            if (block_statistics) {
                auto blk_num = block->block_num();
                *block_statistics = fc::format_string("Received block ${id}... #${n} signed by ${p} "
                    "[trxs: ${count}, lib: ${lib}, confirmed: ${confs}, net: ${net}, cpu: ${cpu} us, elapsed: ${elapsed} us, time: ${time} us, latency: "
                    "${latency} ms]",
                    fc::mutable_variant_object()
                    ("p", block->producer)
                    ("id", id.str().substr(8, 16))
                    ("n", blk_num)
                    ("count", block->transactions.size())
                    ("lib", c->last_irreversible_block_num())
                    ("confs", block->confirmed)
                    ("net", br.total_net_usage)
                    ("cpu", br.total_cpu_usage_us)
                    ("elapsed", br.total_elapsed_time)
                    ("time", br.total_time)
                    ("latency", (now - block->timestamp).count() / 1000));
            }

            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    void load_abi(string& account) {
        auto itr = abi_cache.find(account);
        if (itr == abi_cache.end()) {
            const auto& accnt = c->db().get<account_object, by_name>(account_name(account));
            abi_def abi;
            if(!abi_serializer::to_abi(accnt.abi, abi)) {
                return;
            }
            abi_cache[account] = std::make_shared<abi_serializer>(abi, abi_serializer::create_yield_function(fc::microseconds(150000)));
        }
    }

    eosio::chain::abi_serializer* get_abi_cache(string& account) {
        auto itr = abi_cache.find(account);
    if (itr != abi_cache.end()) {
        return itr->second.get();
    }
    return nullptr;
    }

    void clear_abi_cache(string& account) {
    auto itr = abi_cache.find(account);
    if (itr != abi_cache.end()) {
        abi_cache.erase(itr);
    }
    }

    bool pack_action_args(string& account, string& action, string& _args, vector<char>& result) {
        try {
            load_abi(account);
            auto& serializer = abi_cache[account];
            if (!serializer) {
                EOS_ASSERT(false, action_validate_exception, "ABI for {account} not found!", ("account", account));
                return false;
            }
            auto action_type = serializer->get_action_type(action_name(action));
            EOS_ASSERT(!action_type.empty(), action_validate_exception, "Unknown action ${action}", ("action", action));

            fc::variant args = fc::json::from_string(_args);
            result = serializer->variant_to_binary(action_type, args, abi_serializer::create_yield_function(fc::microseconds(150000)));
            return true;
        } CATCH_AND_LOG_EXCEPTION();
        return false;
    }

    string unpack_action_args(string& account, string& action, string& _binargs) {
        try {
            load_abi(account);
            auto& serializer = abi_cache[account];
            if (!serializer) {
                EOS_ASSERT(false, action_validate_exception, "ABI for {account} not found!", ("account", account));
                return "";
            }
            auto action_type = serializer->get_action_type(action_name(action));
            EOS_ASSERT(!action_type.empty(), action_validate_exception, "Unknown action ${action}", ("action", action));

            bytes binargs = bytes(_binargs.data(), _binargs.data() + _binargs.size());
            auto v = serializer->binary_to_variant(action_type, binargs, abi_serializer::create_yield_function(fc::microseconds(150000)));
            return fc::json::to_string(v, fc::time_point::maximum());
        } CATCH_AND_LOG_EXCEPTION();
        return "";
    }

    string get_producer_public_keys() {
        try {
            const auto& auth = c->pending_block_signing_authority();
            std::vector<public_key_type> pub_keys;

            producer_authority::for_each_key(auth, [&](const public_key_type& key){
                pub_keys.emplace_back(key);
            });

            return fc::json::to_string(pub_keys, fc::time_point::maximum());
        } CATCH_AND_LOG_EXCEPTION();
        return "";
    }

    bool call_native_contract(uint64_t receiver, uint64_t first_receiver, uint64_t action) {
        auto itr = native_contracts.find(receiver);
        if (itr == native_contracts.end()) {
            return false;
        }
        return itr->second->apply(receiver, first_receiver, action);
    }

    static std::optional<std::pair<void *, fn_native_apply>> load_native_contract(const string& native_contract_lib) {
        void* handle = dlopen(native_contract_lib.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            elog("++++++++Failed to load ${s} error: ${err}!", ("s", native_contract_lib)("err", dlerror()));
            return std::nullopt;
        }
        // typedef int (*fn_native_init)(struct IntrinsicsFuncs* funcs);
        fn_native_init native_init = (fn_native_init)dlsym(handle, "native_init");
        if (native_init == nullptr) {
            elog("++++++++native_init entry not found!");
        } else {
            native_init(get_intrinsics());
        }

        fn_native_apply native_apply = (fn_native_apply)dlsym(handle, "native_apply");
        if (native_apply == nullptr) {
            elog("++++++++native_apply entry not found!");
            return std::nullopt;
        }
        return std::make_pair(handle, native_apply);
    }

    bool set_native_contract(const string& contract, const string& native_contract_lib) {
        std::filesystem::path _native_contract_lib = native_contract_lib;
        auto _contract = eosio::chain::name(contract).to_uint64_t();

        if (native_contract_lib.size() == 0) {
            auto itr = native_contracts.find(_contract);
            if (itr != native_contracts.end()) {
                auto itr2 = s_native_libraries.find(itr->second->path);
                EOS_ASSERT(itr2 != s_native_libraries.end(), eosio::chain::chain_exception, "itr != s_native_libraries.end()");
                if (itr2->second.use_count() == 2) {
                    dlclose(itr2->second->handle);
                    s_native_libraries.erase(itr2);
                }
                native_contracts.erase(itr);
                return true;
            }
            return false;
        } else {
            if (!std::filesystem::exists(_native_contract_lib)) {
                elog("++++++++${s} does not exists!", ("s", native_contract_lib));
                return false;
            }

            _native_contract_lib = std::filesystem::absolute(_native_contract_lib);
            auto last_write_time = std::filesystem::last_write_time(_native_contract_lib);
            auto itr = s_native_libraries.find(_native_contract_lib);

            std::shared_ptr<native_contract> _native_contract;
            if (itr != s_native_libraries.end()) {
                _native_contract = itr->second;
                // reload native lib if changed
                if (itr->second->last_write_time != last_write_time) {
                    dlclose(itr->second->handle);
                    auto ret = load_native_contract(native_contract_lib);
                    if (!ret) {
                        // invalid native lib
                        // remove all native contracts which have the same handle
                        auto itr2 = native_contracts.begin();
                        while (itr2 != native_contracts.end()) {
                            if (itr2->second->handle == itr->second->handle) {
                                itr2 = native_contracts.erase(itr2);
                            } else {
                                ++itr2;
                            }
                        }
                        s_native_libraries.erase(itr);
                        return false;
                    }
                    itr->second->handle = ret->first;
                    itr->second->apply = ret->second;
                    itr->second->last_write_time = last_write_time;
                    elog("++++++++reloading ${s}", ("s", _native_contract_lib.string()));
                }
            } else {
                auto ret = load_native_contract(native_contract_lib);
                if (!ret) {
                    return false;
                }
                _native_contract = std::make_shared<native_contract>(
                    native_contract{
                        _native_contract_lib.string(),
                        ret->first,
                        last_write_time,
                        ret->second
                    }
                );
                s_native_libraries[_native_contract_lib] = _native_contract;
            }
            native_contracts[_contract] = _native_contract;
            return true;
        }
    }

    string get_native_contract(const string& contract) {
        auto _contract = eosio::chain::name(contract).to_uint64_t();
        auto itr = native_contracts.find(_contract);
        if (itr == native_contracts.end()) {
            return "";
        }
        return itr->second->path;
    }

    void set_debug_producer_key(string &pub_key) {
        debug_public_key = pub_key;
    }

    string get_debug_producer_key() {
        return debug_public_key;
    }
};


chain_proxy::chain_proxy(): impl(new chain_proxy_impl()) {

}

chain_proxy::~chain_proxy() {

}

int chain_proxy::init(string& config, string& _genesis, string& chain_id, string& protocol_features_dir, string& snapshot_file, string& debug_producer_key) {
    return impl->init(config, _genesis, chain_id, protocol_features_dir, snapshot_file, debug_producer_key);
}

int chain_proxy::attach(eosio::chain::controller* c) {
    return impl->attach(c);
}

controller* chain_proxy::get_controller() {
    return impl->get_controller();
}

const chainbase::database* chain_proxy::get_database() {
    return impl->get_database();
}

chain_rpc_api_proxy* chain_proxy::api_proxy() {
    return impl->api_proxy();
}

void chain_proxy::say_hello() {
    impl->say_hello();
}

void chain_proxy::chain_id(string& result) {
    impl->chain_id(result);
}

bool chain_proxy::startup(bool initdb) {
    return impl->startup(initdb);
}

bool chain_proxy::set_accepted_block_event_listener(fn_block_event_listener _listener, void *user_data) {
    return impl->set_accepted_block_event_listener(_listener, user_data);
}

bool chain_proxy::set_irreversible_block_event_listener(fn_block_event_listener _listener, void *user_data) {
    return impl->set_irreversible_block_event_listener(_listener, user_data);
}

int chain_proxy::start_block(int64_t block_time_since_epoch_ms, uint16_t confirm_block_count, string& _new_features) {
    return impl->start_block(block_time_since_epoch_ms, confirm_block_count, _new_features);
}

int chain_proxy::abort_block() {
    return impl->abort_block();
}

bool chain_proxy::finalize_block(string& _priv_keys) {
    return impl->finalize_block(_priv_keys);
}

bool chain_proxy::commit_block() {
    return impl->commit_block();
}

string chain_proxy::get_info() {
    return impl->get_info();
}

string chain_proxy::get_block_id_for_num(uint32_t block_num ) {
    return impl->get_block_id_for_num(block_num);
}

string chain_proxy::get_global_properties() {
    return impl->get_global_properties();
}

string chain_proxy::get_dynamic_global_properties() {
    return impl->get_dynamic_global_properties();
}

string chain_proxy::get_actor_whitelist() {
    return impl->get_actor_whitelist();
}

string chain_proxy::get_actor_blacklist() {
    return impl->get_actor_blacklist();
}

string chain_proxy::get_contract_whitelist() {
    return impl->get_contract_whitelist();
}

string chain_proxy::get_contract_blacklist() {
    return impl->get_contract_blacklist();
}

string chain_proxy::get_action_blacklist() {
    return impl->get_action_blacklist();
}

string chain_proxy::get_key_blacklist() {
    return impl->get_key_blacklist();
}

void chain_proxy::set_actor_whitelist(string& params) {
    impl->set_actor_whitelist(params);
}

void chain_proxy::set_actor_blacklist(string& params) {
    impl->set_actor_blacklist(params);
}

void chain_proxy::set_contract_whitelist(string& params) {
    impl->set_contract_whitelist(params);
}

void chain_proxy::set_action_blacklist(string& params) {
    impl->set_action_blacklist(params);
}

void chain_proxy::set_key_blacklist(string& params) {
    impl->set_key_blacklist(params);
}

uint32_t chain_proxy::head_block_num() {
    return impl->head_block_num();
}

int64_t chain_proxy::head_block_time() {
    return impl->head_block_time();
}

string chain_proxy::head_block_id() {
    return impl->head_block_id();
}

string chain_proxy::head_block_producer() {
    return impl->head_block_producer();
}

string chain_proxy::head_block_header() {
    return impl->head_block_header();
}

string chain_proxy::head_block_state() {
    return impl->head_block_state();
}

uint32_t chain_proxy::earliest_available_block_num() {
    return impl->earliest_available_block_num();
}

int64_t chain_proxy::last_irreversible_block_time() {
    return impl->last_irreversible_block_time();
}

uint32_t chain_proxy::fork_db_head_block_num() {
    return impl->fork_db_head_block_num();
}

string chain_proxy::fork_db_head_block_id() {
    return impl->fork_db_head_block_id();
}

uint32_t chain_proxy::fork_db_pending_head_block_num() {
    // try {
    //     return c->fork_db_pending_head_block_num();
    // } CATCH_AND_LOG_EXCEPTION()
    return 0;
}

string chain_proxy::fork_db_pending_head_block_id() {
    return "";
    // return c->fork_db_pending_head_block_id().str();
}

string chain_proxy::fork_db_pending_head_block_time() {
    return "";
    // return c->fork_db_pending_head_block_time();
}

string chain_proxy::fork_db_pending_head_block_producer() {
    return "";
    // return c->fork_db_pending_head_block_producer().to_string();
}

int64_t chain_proxy::pending_block_time() {
    return impl->pending_block_time();
}

string chain_proxy::pending_block_producer() {
    return impl->pending_block_producer();
}

string chain_proxy::pending_producer_block_id() {
    return impl->pending_producer_block_id();
}

// string chain_proxy::get_pending_trx_receipts() {
//     return fc::json::to_string(c->get_pending_trx_receipts(), fc::time_point::maximum());
// }

string chain_proxy::active_producers() {
    return impl->active_producers();
}

string chain_proxy::pending_producers() {
    return impl->pending_producers();
}

string chain_proxy::proposed_producers() {
    return impl->proposed_producers();
}

uint32_t chain_proxy::last_irreversible_block_num() {
    return impl->last_irreversible_block_num();
}

string chain_proxy::last_irreversible_block_id() {
    return impl->last_irreversible_block_id();
}

string chain_proxy::fetch_block_by_number(uint32_t block_num) {
    return impl->fetch_block_by_number(block_num);
}

string chain_proxy::fetch_block_by_id(string& params) {
    return impl->fetch_block_by_id(params);
}

string chain_proxy::fetch_block_state_by_number(uint32_t block_num) {
    return impl->fetch_block_state_by_number(block_num);
}

string chain_proxy::fetch_block_state_by_id(string& params) {
    return impl->fetch_block_state_by_id(params);
}

string chain_proxy::calculate_integrity_hash() {
    return impl->calculate_integrity_hash();
}

/*
         void write_snapshot( const snapshot_writer_ptr& snapshot )const;
*/

bool chain_proxy::sender_avoids_whitelist_blacklist_enforcement(string& sender) {
    return impl->sender_avoids_whitelist_blacklist_enforcement(sender);
}

bool chain_proxy::check_actor_list(string& param) {
    return impl->check_actor_list(param);
}

bool chain_proxy::check_contract_list(string& param) {
    return impl->check_contract_list(param);
}

bool chain_proxy::check_action_list(string& code, string& action) {
    return impl->check_action_list(code, action);
}

bool chain_proxy::check_key_list(string& param) {
    return impl->check_key_list(param);
}

bool chain_proxy::is_building_block() {
    return impl->is_building_block();
}

bool chain_proxy::is_speculative_block() {
    return impl->is_speculative_block();
}

bool chain_proxy::is_ram_billing_in_notify_allowed() {
    return impl->is_ram_billing_in_notify_allowed();
}

void chain_proxy::add_resource_greylist(string& param) {
    impl->add_resource_greylist(param);
}

void chain_proxy::remove_resource_greylist(string& param) {
    impl->remove_resource_greylist(param);
}

bool chain_proxy::is_resource_greylisted(string& param) {
    return impl->is_resource_greylisted(param);
}

string chain_proxy::get_resource_greylist() {
    return impl->get_resource_greylist();
}

string chain_proxy::get_config() {
    return "";
    // return fc::json::to_string(c->get_config(), fc::time_point::maximum());
}

bool chain_proxy::validate_expiration(string& param) {
    return impl->validate_expiration(param);
}

bool chain_proxy::validate_tapos(string& param) {
    return impl->validate_tapos(param);
}

bool chain_proxy::validate_db_available_size() {
    return impl->validate_db_available_size();
}

// bool chain_proxy::validate_reversible_available_size() {
//     try {
//         c->validate_reversible_available_size();
//         return true;
//     } CATCH_AND_LOG_EXCEPTION()
//     return false;
// }

bool chain_proxy::is_protocol_feature_activated(string& param) {
    return impl->is_protocol_feature_activated(param);
}

bool chain_proxy::is_builtin_activated(int feature) {
    return impl->is_builtin_activated(feature);
}

bool chain_proxy::is_known_unexpired_transaction(string& param) {
    return impl->is_known_unexpired_transaction(param);
}

int64_t chain_proxy::set_proposed_producers(string& param) {
    return impl->set_proposed_producers(param);
}

bool chain_proxy::light_validation_allowed() {
    return impl->light_validation_allowed();
}

bool chain_proxy::skip_auth_check() {
    return impl->skip_auth_check();
}

bool chain_proxy::skip_db_sessions() {
    return impl->skip_db_sessions();
}

bool chain_proxy::skip_trx_checks() {
    return impl->skip_trx_checks();
}

bool chain_proxy::contracts_console() {
    return impl->contracts_console();
}

int chain_proxy::get_read_mode() {
    return impl->get_read_mode();
}

int chain_proxy::get_validation_mode() {
    return impl->get_validation_mode();
}

void chain_proxy::set_subjective_cpu_leeway(uint64_t leeway) {
    impl->set_subjective_cpu_leeway(leeway);
}

void chain_proxy::set_greylist_limit(uint32_t limit) {
    impl->set_greylist_limit(limit);
}

uint32_t chain_proxy::get_greylist_limit() {
    return impl->get_greylist_limit();
}

void chain_proxy::add_to_ram_correction(string& account, uint64_t ram_bytes) {
    impl->add_to_ram_correction(account, ram_bytes);
}

bool chain_proxy::all_subjective_mitigations_disabled() {
    return impl->all_subjective_mitigations_disabled();
}

string chain_proxy::get_scheduled_producer(string& _block_time) {
    return impl->get_scheduled_producer(_block_time);
}

void chain_proxy::gen_transaction(bool json, string& _actions, int64_t expiration_sec, string& reference_block_id, string& _chain_id, bool compress, std::string& _private_keys, vector<char>& result) {
    impl->gen_transaction(json, _actions, expiration_sec, reference_block_id, _chain_id, compress, _private_keys, result);
}

bool chain_proxy::push_transaction(const char *_packed_tx, size_t _packed_tx_size, int64_t block_deadline_ms, uint32_t billed_cpu_time_us, bool explicit_cpu_bill, uint32_t subjective_cpu_bill_us, bool read_only, string& result) {
    return impl->push_transaction(_packed_tx, _packed_tx_size, block_deadline_ms, billed_cpu_time_us, explicit_cpu_bill, subjective_cpu_bill_us, read_only, result);
}

string chain_proxy::get_scheduled_transactions() {
    return impl->get_scheduled_transactions();
}

string chain_proxy::get_scheduled_transaction(const char *sender_id, size_t sender_id_size, string& sender) {
    return impl->get_scheduled_transaction(sender_id, sender_id_size, sender);
}

string chain_proxy::push_scheduled_transaction(string& scheduled_tx_id, string& deadline, uint32_t billed_cpu_time_us) {
    return impl->push_scheduled_transaction(scheduled_tx_id, deadline, billed_cpu_time_us);
}

bool chain_proxy::push_block_from_block_log(void *block_log_ptr, uint32_t block_num) {
    return impl->push_block_from_block_log(block_log_ptr, block_num);
}

bool chain_proxy::push_block(const char *raw_block, size_t raw_block_size, string *block_statistics) {
    return impl->push_block(raw_block, raw_block_size, block_statistics);
}

void chain_proxy::load_abi(string& account) {
    impl->load_abi(account);
}

eosio::chain::abi_serializer* chain_proxy::get_abi_cache(string& account) {
   return impl->get_abi_cache(account);
}

void chain_proxy::clear_abi_cache(string& account) {
   impl->clear_abi_cache(account);
}

bool chain_proxy::pack_action_args(string& account, string& action, string& _args, vector<char>& result) {
    return impl->pack_action_args(account, action, _args, result);
}

string chain_proxy::unpack_action_args(string& account, string& action, string& _binargs) {
    return impl->unpack_action_args(account, action, _binargs);
}

string chain_proxy::get_producer_public_keys() {
    return impl->get_producer_public_keys();
}

bool chain_proxy::call_native_contract(uint64_t receiver, uint64_t first_receiver, uint64_t action) {
    return impl->call_native_contract(receiver, first_receiver, action);
}

bool chain_proxy::set_native_contract(const string& contract, const string& native_contract_lib) {
    return impl->set_native_contract(contract, native_contract_lib);
}

string chain_proxy::get_native_contract(const string& contract) {
    return impl->get_native_contract(contract);
}

void chain_proxy::set_debug_producer_key(string &pub_key) {
    return impl->set_debug_producer_key(pub_key);
}

string chain_proxy::get_debug_producer_key() {
    return impl->get_debug_producer_key();
}
