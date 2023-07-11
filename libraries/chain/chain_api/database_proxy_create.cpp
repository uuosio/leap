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

#define CREATE_BEGIN(OBJECT_NAME) \
    if (OBJECT_NAME##_object_type == tp) { \
        auto __obj = unpack_database_object<OBJECT_NAME##_object_>(raw_data, raw_data_size); \
        db.create<OBJECT_NAME##_object>( [&]( auto& _obj ) {

#define CREATE_END() \
        } ); \
        return 1; \
    }

#define CREATE_CONTRACT_TABLE_OBJECT(OBJECT_NAME) \
    if (OBJECT_NAME##_object_type == tp) { \
        auto __obj = unpack_database_object<OBJECT_NAME##_object_>(raw_data, raw_data_size); \
        db.create<OBJECT_NAME##_object>( [&]( auto& _obj ) { \
            _obj.primary_key = __obj.primary_key; \
            _obj.payer = __obj.payer; \
            memcpy(&_obj.secondary_key, &__obj.secondary_key, sizeof(__obj.secondary_key)); \
        } ); \
        return 1; \
    }

int32_t database_proxy::create(int32_t tp, const char *raw_data, size_t raw_data_size) {
    try {
        CREATE_BEGIN(account)
            // account_name         name; //< name should not be changed within a chainbase modifier lambda
            // block_timestamp_type creation_date;
            // vector<char>          abi;
            _obj.name = __obj.name;
            _obj.creation_date = __obj.creation_date;
            _obj.abi.assign(__obj.abi.data(), __obj.abi.size());
        CREATE_END()

        CREATE_BEGIN(account_metadata)
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
            _obj.name = __obj.name; //< name should not be changed within a chainbase modifier lambda
            _obj.recv_sequence = __obj.recv_sequence;
            _obj.auth_sequence = __obj.auth_sequence;
            _obj.code_sequence = __obj.code_sequence;
            _obj.abi_sequence  = __obj.abi_sequence;
            _obj.code_hash = __obj.code_hash;
            _obj.last_code_update = __obj.last_code_update;
            _obj.flags = __obj.flags;
            _obj.vm_type = __obj.vm_type;
            _obj.vm_version = __obj.vm_version;
        CREATE_END()
        CREATE_BEGIN(permission)
            // permission_usage_object::id_type  usage_id;
            // int64_t                           parent; ///< parent permission
            // account_name                      owner; ///< the account this permission belongs to (should not be changed within a chainbase modifier lambda)
            // permission_name                   name; ///< human-readable name for the permission (should not be changed within a chainbase modifier lambda)
            // time_point                        last_updated; ///< the last time this authority was updated
            // authority_                        auth; ///< authority required to execute this permission
            _obj.usage_id = __obj.usage_id;
            _obj.parent = __obj.parent;
            _obj.owner = __obj.owner;
            _obj.name = __obj.name;
            _obj.last_updated = __obj.last_updated;

            _obj.auth.threshold = __obj.auth.threshold;
            _obj.auth = __obj.auth;
        CREATE_END()

        CREATE_BEGIN(permission_usage)
            // time_point        last_used;   ///< when this permission was last used
            _obj.last_used = __obj.last_used;
        CREATE_END()

        CREATE_BEGIN(permission_link)
            // account_name    account;
            // account_name    code; /// TODO: rename to scope
            // action_name       message_type;
            // permission_name required_permission;
            _obj.account = __obj.account;
            _obj.code = __obj.code;
            _obj.message_type = __obj.message_type;
            _obj.required_permission = __obj.required_permission;
        CREATE_END()

        CREATE_BEGIN(key_value)
            // int64_t              t_id; //< t_id should not be changed within a chainbase modifier lambda
            // uint64_t             primary_key; //< primary_key should not be changed within a chainbase modifier lambda
            // account_name         payer;
            // vector<char>         value;
            _obj.t_id = __obj.t_id;
            _obj.primary_key = __obj.primary_key;
            _obj.payer = __obj.payer;
            _obj.value.assign(__obj.value.data(), __obj.value.size());
        CREATE_END()

        CREATE_CONTRACT_TABLE_OBJECT(index64)
        CREATE_CONTRACT_TABLE_OBJECT(index128)
        CREATE_CONTRACT_TABLE_OBJECT(index256)
        CREATE_CONTRACT_TABLE_OBJECT(index_double)
        CREATE_CONTRACT_TABLE_OBJECT(index_long_double)

        CREATE_BEGIN(global_property)
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
        CREATE_END()

        CREATE_BEGIN(dynamic_global_property)
            _obj.global_action_sequence = __obj.global_action_sequence;
        CREATE_END()

        CREATE_BEGIN(block_summary)
            _obj.block_id = __obj.block_id;
        CREATE_END()

        CREATE_BEGIN(transaction)
            _obj.expiration = __obj.expiration;
            _obj.trx_id = __obj.trx_id;
        CREATE_END()

        CREATE_BEGIN(table_id)
            _obj.code = __obj.code;
            _obj.scope = __obj.scope;
            _obj.table = __obj.table;
            _obj.payer = __obj.payer;
            _obj.count = __obj.count;
        CREATE_END()

        CREATE_BEGIN(generated_transaction)
            _obj.trx_id = __obj.trx_id;
            _obj.sender = __obj.sender;
            _obj.sender_id = __obj.sender_id;
            _obj.payer = __obj.payer;
            _obj.delay_until = __obj.delay_until;
            _obj.expiration = __obj.expiration;
            _obj.published = __obj.published;
            _obj.packed_trx.assign(__obj.packed_trx.data(), __obj.packed_trx.size());
        CREATE_END();

        CREATE_BEGIN(resource_limits)
        // account_name owner; //< owner should not be changed within a chainbase modifier lambda
        // bool pending = false; //< pending should not be changed within a chainbase modifier lambda
        // int64_t net_weight = -1;
        // int64_t cpu_weight = -1;
        // int64_t ram_bytes = -1;
            _obj.owner = __obj.owner;
            _obj.net_weight = __obj.net_weight;
            _obj.cpu_weight = __obj.cpu_weight;
            _obj.ram_bytes = __obj.ram_bytes; 
        CREATE_END()

        CREATE_BEGIN(resource_usage)
        // account_name owner; //< owner should not be changed within a chainbase modifier lambda
        // usage_accumulator        net_usage;
        // usage_accumulator        cpu_usage;
        // uint64_t                 ram_usage = 0;
            _obj.owner = __obj.owner;
            _obj.net_usage = __obj.net_usage;
            _obj.cpu_usage = __obj.cpu_usage;
            _obj.ram_usage = __obj.ram_usage;
        CREATE_END()

        CREATE_BEGIN(resource_limits_state)
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
        CREATE_END()

        CREATE_BEGIN(resource_limits_config)
            _obj.cpu_limit_parameters = __obj.cpu_limit_parameters;
            _obj.net_limit_parameters = __obj.net_limit_parameters;
            _obj.account_cpu_usage_average_window = __obj.account_cpu_usage_average_window;
            _obj.account_net_usage_average_window = __obj.account_net_usage_average_window;
        CREATE_END()

        CREATE_BEGIN(protocol_state)
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
        CREATE_END()

        CREATE_BEGIN(account_ram_correction)
            _obj.name = __obj.name; //< name should not be changed within a chainbase modifier lambda
            _obj.ram_correction = __obj.ram_correction;
        CREATE_END()

        CREATE_BEGIN(code)
        // digest_type  code_hash; //< code_hash should not be changed within a chainbase modifier lambda
        // vector<char>  code;
        // uint64_t     code_ref_count;
        // uint32_t     first_block_used;
        // uint8_t      vm_type = 0; //< vm_type should not be changed within a chainbase modifier lambda
        // uint8_t      vm_version = 0; //< vm_version should not be changed within a chainbase modifier lambda
            _obj.code_hash = __obj.code_hash;
            _obj.code.assign(__obj.code.data(), __obj.code.size());
            _obj.code_ref_count = __obj.code_ref_count;
            _obj.first_block_used = __obj.first_block_used;
            _obj.vm_type = __obj.vm_type;
            _obj.vm_version = __obj.vm_version;
        CREATE_END()

        CREATE_BEGIN(database_header)
            _obj.version = __obj.version;
        CREATE_END()

        FC_ASSERT(0, "unhandled object type ${tp} in create", ("tp", tp));
    } CATCH_AND_LOG_EXCEPTION();
    return -2;
}
