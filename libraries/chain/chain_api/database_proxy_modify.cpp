#include <chainbase/chainbase.hpp>
#include <eosio/chain/name.hpp>
#include <eosio/chain/types.hpp>

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

#include <fc/io/raw.hpp>
#include <fc/io/json.hpp>

#include <database_objects.hpp>

#include "database_proxy.hpp"
#include "chain_macro.hpp"

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::chain::resource_limits;

#define HANDLE_DATABASE_OBJECT_FIND_BY_ID(OBJECT_NAME) \
    if (index_position == 0) { \
        int64_t _id; \
        fc::raw::unpack(stream, _id); \
        auto key = OBJECT_NAME##_object::id_type(_id); \
        obj = db.find<OBJECT_NAME##_object, by_id>(key); \
        if (!obj) { \
            return 0; \
        } \
    }

#define HANDLE_DATABASE_OBJECT_FIND_BY_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (index_position == INDEX_POSITION) { \
        KEY_TYPE key; \
        fc::raw::unpack(stream, key); \
        obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
    }

#define HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (index_position == INDEX_POSITION) { \
        std::tuple<__VA_ARGS__> key; \
        unpack_tuple(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
    }

#define MODIFY_BEGIN(OBJECT_NAME) \
            auto __obj = unpack_database_object<OBJECT_NAME##_object_>(raw_data, raw_data_size); \
            db.modify<OBJECT_NAME##_object>( *obj, [&]( auto& _obj ) {

#define MODIFY_END() \
            } ); \
            return 1;

#define HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(OBJECT_NAME, SECONDARY_KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp) { \
        const OBJECT_NAME##_object *obj = nullptr; \
        if (index_position == 0) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(OBJECT_NAME); \
        } \
        if (index_position == 1) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, 1, by_primary, table_id, uint64_t) \
        } \
        if (index_position == 2) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, 2, by_secondary, table_id, SECONDARY_KEY_TYPE, uint64_t); \
        } \
        auto __obj = unpack_database_object<OBJECT_NAME##_object_>(raw_data, raw_data_size); \
        db.modify<OBJECT_NAME##_object>( *obj, [&]( auto& _obj ) { \
            _obj.primary_key = __obj.primary_key; \
            _obj.payer = __obj.payer; \
            memcpy(&_obj.secondary_key, &__obj.secondary_key, sizeof(__obj.secondary_key)); \
        } ); \
        return 1; \
    }

int32_t database_proxy::modify(int32_t tp, int32_t index_position, const char *raw_key, size_t raw_key_size, const char *raw_data, size_t raw_data_size) {
    fc::datastream<const char*> stream(raw_key, raw_key_size);

    try {
    //     // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
        // ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
        if (account_object_type == tp) {
            const account_object* obj = nullptr;
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(account)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(account, 1, by_name, account_name)
            MODIFY_BEGIN(account)
                // account_name         name; //< name should not be changed within a chainbase modifier lambda
                // block_timestamp_type creation_date;
                // vector<char>          abi;
                FC_ASSERT(_obj.name == __obj.name, "name should not be changed within a chainbase modifier lambda");
                _obj.creation_date = __obj.creation_date;
                _obj.abi.assign(__obj.abi.data(), __obj.abi.size());
            MODIFY_END()
        }
        if (account_metadata_object_type == tp) {
            const account_metadata_object* obj = nullptr;
            // by_name>, account_name
            // ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
            // ordered_unique<tag<by_name>, member<account_metadata_object, account_name, &account_metadata_object::name>>
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(account_metadata)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(account_metadata, 1, by_name, account_name)
            MODIFY_BEGIN(account_metadata)
                // account_name          name; //< name should not be changed within a chainbase modifier lambda
                // uint64_t              recv_sequence = 0;
                // uint64_t              auth_sequence = 0;
                // uint64_t              code_sequence = 0;
                // uint64_t              abi_sequence  = 0;
                // digest_type           code_hash;
                // time_point            last_code_update;
                // uint32_t              flags = 0;
                // uint8_t               vm_type = 0;
                // uint8_t               vm_version = 0;
                FC_ASSERT(_obj.name == __obj.name, "name should not be changed within a chainbase modifier lambda");
                _obj.recv_sequence = __obj.recv_sequence;
                _obj.auth_sequence = __obj.auth_sequence;
                _obj.code_sequence = __obj.code_sequence;
                _obj.abi_sequence  = __obj.abi_sequence;
                _obj.code_hash = __obj.code_hash;
                _obj.last_code_update = __obj.last_code_update;
                _obj.flags = __obj.flags;
                _obj.vm_type = __obj.vm_type;
                _obj.vm_version = __obj.vm_version;
            MODIFY_END()
        }
        if (permission_object_type == tp) {
        //    struct by_parent;
        //    struct by_owner;
        //    struct by_name;
            // by_parent
            //     permission_object::id_type, &permission_object::parent
            //     permission_object::id_type, &permission_object::id
            // by_owner
            //     account_name, &permission_object::owner
            //     permission_name, &permission_object::name
            // by_name
            //     permission_name, &permission_object::name
            //     permission_object::id_type, &permission_object::id
            const permission_object* obj = nullptr;
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 1, by_parent, permission_object::id_type, permission_object::id_type)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 2, eosio::chain::by_owner, account_name, permission_name)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 3, by_name, permission_name, permission_object::id_type)
            MODIFY_BEGIN(permission)
                // permission_usage_object::id_type  usage_id;
                // int64_t                           parent; ///< parent permission
                // account_name                      owner; ///< the account this permission belongs to (should not be changed within a chainbase modifier lambda)
                // permission_name                   name; ///< human-readable name for the permission (should not be changed within a chainbase modifier lambda)
                // time_point                        last_updated; ///< the last time this authority was updated
                // authority_                        auth; ///< authority required to execute this permission
                FC_ASSERT(_obj.owner == __obj.owner, "owner should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.name == __obj.name, "name should not be changed within a chainbase modifier lambda");

                _obj.usage_id = __obj.usage_id;
                _obj.parent = __obj.parent;
                _obj.last_updated = __obj.last_updated;

                _obj.auth.threshold = __obj.auth.threshold;
                _obj.auth = __obj.auth;
            MODIFY_END()
        }

        if (permission_usage_object_type == tp) {
            const permission_usage_object* obj = nullptr;
            // ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission_usage)
            MODIFY_BEGIN(permission_usage)
                // time_point        last_used;   ///< when this permission was last used
                _obj.last_used = __obj.last_used;
            MODIFY_END()
        }

        if (permission_link_object_type == tp) {
            const permission_link_object *obj = nullptr;
        //    struct by_action_name;
        //    struct by_permission_name;
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission_link)
        // ordered_unique<tag<by_action_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission_link, 1, by_action_name, account_name, account_name, account_name)
        // ordered_unique<tag<by_permission_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_name, required_permission),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_link_object::id_type, id)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission_link, 2, by_permission_name, account_name, permission_name, permission_link_object::id_type)
            MODIFY_BEGIN(permission_link)
                // account_name    account;
                // account_name    code; /// TODO: rename to scope
                // action_name       message_type;
                // permission_name required_permission;
                _obj.account = __obj.account;
                _obj.code = __obj.code;
                _obj.message_type = __obj.message_type;
                _obj.required_permission = __obj.required_permission;
            MODIFY_END()
        }

        if (key_value_object_type == tp) {
            const key_value_object *obj = nullptr;
        //     by_id
        //     by_scope_primary
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(key_value)
        //  ordered_unique<tag<by_scope_primary>,
        //        member<key_value_object, table_id, &key_value_object::t_id>,
        //        member<key_value_object, uint64_t, &key_value_object::primary_key>
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(key_value, 1, by_scope_primary, table_id, uint64_t)
            MODIFY_BEGIN(key_value)
                // int64_t              t_id; //< t_id should not be changed within a chainbase modifier lambda
                // uint64_t             primary_key; //< primary_key should not be changed within a chainbase modifier lambda
                // account_name         payer;
                // vector<char>         value;
                FC_ASSERT(_obj.t_id == __obj.t_id, "t_id should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.primary_key == __obj.primary_key, "primary_key should not be changed within a chainbase modifier lambda");
                _obj.payer = __obj.payer;
                _obj.value.assign(__obj.value.data(), __obj.value.size());
            MODIFY_END()
        }

    //   by_id
    //   by_primary
    //   by_secondary

    // by_primary>
    //         member<index_object, table_id, &index_object::t_id>
    //         member<index_object, uint64_t, &index_object::primary_key>
    // by_secondary
    //         table_id, &index_object::t_id>,
    //         SecondaryKey, &index_object::secondary_key>,
    //         uint64_t, &index_object::primary_key>
        HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(index64, uint64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(index128, uint128_t)
        HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(index256, key256_t)
        HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(index_double, float64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(index_long_double, float128_t)

        if (global_property_object_type == tp) {
            const global_property_object *obj = nullptr;
            // ordered_unique<tag<by_id>,
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(global_property)
            MODIFY_BEGIN(global_property)
                // id_type                             id;
                // std::optional<block_num_type>       proposed_schedule_block_num;
                // shared_producer_authority_schedule  proposed_schedule;
                // chain_config                        configuration;
                // chain_id_type                       chain_id;
                // kv_database_config                  kv_configuration;
                // wasm_config                         wasm_configuration;
                _obj.proposed_schedule_block_num = __obj.proposed_schedule_block_num;
                _obj.proposed_schedule.version = __obj.proposed_schedule.version;
                _obj.proposed_schedule = __obj.proposed_schedule.to_shared(_obj.proposed_schedule.producers.get_allocator());
                _obj.configuration = __obj.configuration;
                _obj.chain_id = __obj.chain_id;
                _obj.kv_configuration = __obj.kv_configuration;
                _obj.wasm_configuration = __obj.wasm_configuration;
            MODIFY_END()
        }

        if (dynamic_global_property_object_type == tp) {
            const dynamic_global_property_object *obj = nullptr;
            // ordered_unique<tag<by_id>,
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(dynamic_global_property)
            MODIFY_BEGIN(dynamic_global_property)
                _obj.global_action_sequence = __obj.global_action_sequence;
            MODIFY_END()
        }

        if (block_summary_object_type == tp) {
            const block_summary_object *obj = nullptr;
            // ordered_unique<tag<by_id>
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(block_summary)
            MODIFY_BEGIN(block_summary)
                _obj.block_id = __obj.block_id;
            MODIFY_END()
        }

        if (transaction_object_type == tp) {
            const transaction_object *obj = nullptr;
            // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_object::id_type, id)>,
            // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_id_type, trx_id)>,
            // ordered_unique< tag<by_expiration>,
            //      BOOST_MULTI_INDEX_MEMBER( transaction_object, time_point_sec, expiration ),
            //      BOOST_MULTI_INDEX_MEMBER( transaction_object, transaction_object::id_type, id)
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(transaction)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(transaction, 1, by_trx_id, transaction_id_type);
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(transaction, 2, by_expiration, time_point_sec, transaction_object::id_type);
            MODIFY_BEGIN(transaction)
                // id_type             id;
                // time_point_sec      expiration;
                // transaction_id_type trx_id; //< trx_id should not be changed within a chainbase modifier lambda
                FC_ASSERT(_obj.trx_id == __obj.trx_id, "trx_id should not be changed within a chainbase modifier lambda");
                _obj.expiration = __obj.expiration;
            MODIFY_END()
        }

        if (table_id_object_type == tp) {
            const table_id_object *obj = nullptr;
        //    by_id
        //    struct by_code_scope_table;
            // ordered_unique<tag<by_id>,
            // ordered_unique<tag<by_code_scope_table>,
            //     member<table_id_object, account_name, &table_id_object::code>,
            //     member<table_id_object, scope_name,   &table_id_object::scope>,
            //     member<table_id_object, table_name,   &table_id_object::table>
            // >
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(table_id)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(table_id, 1, by_code_scope_table, account_name, scope_name, table_name);
            MODIFY_BEGIN(table_id)
                // id_type        id;
                // account_name   code;  //< code should not be changed within a chainbase modifier lambda
                // scope_name     scope; //< scope should not be changed within a chainbase modifier lambda
                // table_name     table; //< table should not be changed within a chainbase modifier lambda
                // account_name   payer;
                // uint32_t       count = 0; /// the number of elements in the table
                FC_ASSERT(_obj.code == __obj.code, "code should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.scope == __obj.scope, "scope should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.table == __obj.table, "table should not be changed within a chainbase modifier lambda");
                _obj.payer = __obj.payer;
                _obj.count = __obj.count;
            MODIFY_END()
        }

        //    struct by_trx_id;
        //    struct by_expiration;
        //    struct by_delay;
        //    struct by_sender_id;

            // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(generated_transaction_object, generated_transaction_object::id_type, id)>,
            // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, transaction_id_type, trx_id)>,
            // ordered_unique< tag<by_expiration>,
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, expiration),
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            // ordered_unique< tag<by_delay>,
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, delay_until),
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            // ordered_unique< tag<by_sender_id>,
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, account_name, sender),
            //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, uint128_t, sender_id)
        if (generated_transaction_object_type == tp) {
            const generated_transaction_object* obj = nullptr;
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(generated_transaction)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(generated_transaction, 1, by_trx_id, transaction_id_type);
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 2, by_expiration, time_point, generated_transaction_object::id_type);
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 3, by_delay, time_point, generated_transaction_object::id_type);
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 4, by_sender_id, account_name, uint128_t);
            MODIFY_BEGIN(generated_transaction)
                // id_type                       id;
                // transaction_id_type           trx_id; //< trx_id should not be changed within a chainbase modifier lambda
                // account_name                  sender; //< sender should not be changed within a chainbase modifier lambda
                // uint128_t                     sender_id = 0; /// ID given this transaction by the sender (should not be changed within a chainbase modifier lambda)
                // account_name                  payer;
                // time_point                    delay_until; /// this generated transaction will not be applied until the specified time
                // time_point                    expiration; /// this generated transaction will not be applied after this time
                // time_point                    published;
                // shared_blob                   packed_trx;
                FC_ASSERT(_obj.trx_id == __obj.trx_id, "trx_id should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.sender == __obj.sender, "sender should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.sender_id == __obj.sender_id, "sender_id should not be changed within a chainbase modifier lambda");
                _obj.payer = __obj.payer;
                _obj.delay_until = __obj.delay_until;
                _obj.expiration = __obj.expiration;
                _obj.published = __obj.published;
                _obj.packed_trx.assign(__obj.packed_trx.data(), __obj.packed_trx.size());
            MODIFY_END();
        }

        if (resource_limits_object_type == tp) {
            const resource_limits_object *obj = nullptr;
        // by_id
        // by_owner
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits)
        //  ordered_unique<tag<by_owner>,
        //        BOOST_MULTI_INDEX_MEMBER(resource_limits_object, bool, pending),
        //        BOOST_MULTI_INDEX_MEMBER(resource_limits_object, account_name, owner)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(resource_limits, 1, resource_limits::by_owner, bool, account_name);
            MODIFY_BEGIN(resource_limits)
            // account_name owner; //< owner should not be changed within a chainbase modifier lambda
            // bool pending = false; //< pending should not be changed within a chainbase modifier lambda
            // int64_t net_weight = -1;
            // int64_t cpu_weight = -1;
            // int64_t ram_bytes = -1;
                FC_ASSERT(_obj.owner == __obj.owner, "owner should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.pending == __obj.pending, "pending should not be changed within a chainbase modifier lambda");
                _obj.net_weight = __obj.net_weight;
                _obj.cpu_weight = __obj.cpu_weight;
                _obj.ram_bytes = __obj.ram_bytes; 
            MODIFY_END()
        }

        if (resource_usage_object_type == tp) {
            const resource_usage_object *obj = nullptr;
        // by_id
        // by_owner
            // ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
            // ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_usage)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(resource_usage, 1, resource_limits::by_owner, account_name);
            MODIFY_BEGIN(resource_usage)
            // account_name owner; //< owner should not be changed within a chainbase modifier lambda
            // usage_accumulator        net_usage;
            // usage_accumulator        cpu_usage;
            // uint64_t                 ram_usage = 0;
                FC_ASSERT(_obj.owner == __obj.owner, "owner should not be changed within a chainbase modifier lambda");
                _obj.net_usage = __obj.net_usage;
                _obj.cpu_usage = __obj.cpu_usage;
                _obj.ram_usage = __obj.ram_usage;
            MODIFY_END()
        }

        if (resource_limits_state_object_type == tp) {
            const resource_limits_state_object *obj = nullptr;
        // by_id
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits_state)
            MODIFY_BEGIN(resource_limits_state)
                // resource_limits::usage_accumulator average_block_net_usage;
                // resource_limits::usage_accumulator average_block_cpu_usage;
                // uint64_t pending_net_usage = 0ULL;
                // uint64_t pending_cpu_usage = 0ULL;
                // uint64_t total_net_weight = 0ULL;
                // uint64_t total_cpu_weight = 0ULL;
                // uint64_t total_ram_bytes = 0ULL;
                // uint64_t virtual_net_limit = 0ULL;
                // uint64_t virtual_cpu_limit = 0ULL;
                _obj.average_block_net_usage = __obj.average_block_net_usage;
                _obj.average_block_cpu_usage = __obj.average_block_cpu_usage;
                _obj.pending_net_usage = __obj.pending_net_usage;
                _obj.pending_cpu_usage = __obj.pending_cpu_usage;
                _obj.total_net_weight = __obj.total_net_weight;
                _obj.total_cpu_weight = __obj.total_cpu_weight;
                _obj.total_ram_bytes = __obj.total_ram_bytes;
                _obj.virtual_net_limit = __obj.virtual_net_limit;
                _obj.virtual_cpu_limit = __obj.virtual_cpu_limit;
            MODIFY_END()
        }

        if (resource_limits_config_object_type == tp) {
            const resource_limits_config_object *obj = nullptr;
        // by_id
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits_config)
            MODIFY_BEGIN(resource_limits_config)
                _obj.cpu_limit_parameters = __obj.cpu_limit_parameters;
                _obj.net_limit_parameters = __obj.net_limit_parameters;
                _obj.account_cpu_usage_average_window = __obj.account_cpu_usage_average_window;
                _obj.account_net_usage_average_window = __obj.account_net_usage_average_window;
            MODIFY_END()
        }

        if (protocol_state_object_type == tp) {
            const protocol_state_object *obj = nullptr;
        // by_id
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(protocol_state)
            MODIFY_BEGIN(protocol_state)
                // vector<activated_protocol_feature_>        activated_protocol_features;
                // vector<digest_type>                        preactivated_protocol_features;
                // flat_multimap<uint64_t, string>            whitelisted_intrinsics;
                // uint32_t                                   num_supported_key_types = 0;

                // _obj.activated_protocol_features = __obj.activated_protocol_features;
                // _obj.preactivated_protocol_features = __obj.preactivated_protocol_features;
                _obj.activated_protocol_features.clear();
                for( const auto& feature : __obj.activated_protocol_features ) {
                    auto _feature = protocol_state_object::activated_protocol_feature(
                        feature.feature_digest,
                        feature.activation_block_num
                    );
                    _obj.activated_protocol_features.emplace_back(_feature);
                }

                _obj.preactivated_protocol_features.clear();
                for( const auto& feature : __obj.preactivated_protocol_features ) {
                    _obj.preactivated_protocol_features.emplace_back(feature);
                }

                _obj.whitelisted_intrinsics.clear();
                for( const auto& i : __obj.whitelisted_intrinsics ) {
                    add_intrinsic_to_whitelist( _obj.whitelisted_intrinsics, i );
                }
                _obj.num_supported_key_types = __obj.num_supported_key_types;
            MODIFY_END()
        }

        if (account_ram_correction_object_type == tp) {
            const account_ram_correction_object *obj = nullptr;
        // by_id
        // by_name
            // ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
            // ordered_unique<tag<by_name>, member<account_ram_correction_object, account_name, &account_ram_correction_object::name>>
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(account_ram_correction)
            HANDLE_DATABASE_OBJECT_FIND_BY_KEY(account_ram_correction, 1, by_name, account_name)
            MODIFY_BEGIN(account_ram_correction)
                FC_ASSERT(_obj.name == __obj.name, "name should not be changed within a chainbase modifier lambda");
                // _obj.name = __obj.name; //< name should not be changed within a chainbase modifier lambda
                _obj.ram_correction = __obj.ram_correction;
            MODIFY_END()
        }

        // by_id
        // by_code_hash
            // ordered_unique<tag<by_id>, member<code_object, code_object::id_type, &code_object::id>>,
            // ordered_unique<tag<by_code_hash>,
            //     member<code_object, digest_type, &code_object::code_hash>,
            //     member<code_object, uint8_t,     &code_object::vm_type>,
            //     member<code_object, uint8_t,     &code_object::vm_version>
        if (code_object_type == tp) {
            const code_object *obj = nullptr;
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(code)
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(code, 1, by_code_hash, digest_type, uint8_t, uint8_t)
            MODIFY_BEGIN(code)
            // digest_type  code_hash; //< code_hash should not be changed within a chainbase modifier lambda
            // vector<char>  code;
            // uint64_t     code_ref_count;
            // uint32_t     first_block_used;
            // uint8_t      vm_type = 0; //< vm_type should not be changed within a chainbase modifier lambda
            // uint8_t      vm_version = 0; //< vm_version should not be changed within a chainbase modifier lambda
                FC_ASSERT(_obj.code_hash == __obj.code_hash, "code_hash should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.vm_type == __obj.vm_type, "vm_type should not be changed within a chainbase modifier lambda");
                FC_ASSERT(_obj.vm_version == __obj.vm_version, "vm_version should not be changed within a chainbase modifier lambda");
                _obj.code.assign(__obj.code.data(), __obj.code.size());
                _obj.code_ref_count = __obj.code_ref_count;
                _obj.first_block_used = __obj.first_block_used;
            MODIFY_END()
        }
        if (database_header_object_type == tp) {
            const database_header_object *obj = nullptr;
        // by_id
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(database_header)
            MODIFY_BEGIN(database_header)
                _obj.version = __obj.version;
            MODIFY_END()
        }
        FC_ASSERT(0, "unhandled object ${tp}, ${index_position}", ("tp", tp)("index_position", index_position));
    } CATCH_AND_LOG_EXCEPTION();
    return -2;
}
