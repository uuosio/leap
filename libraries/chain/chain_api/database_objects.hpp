#pragma once
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

using namespace std;
using namespace eosio::chain;

// null_object_type = 0
// account_object_type = 1
struct account_object_ {
    account_name         name; //< name should not be changed within a chainbase modifier lambda
    block_timestamp_type creation_date;
    vector<char>          abi;
};

FC_REFLECT(account_object_, (name)(creation_date)(abi))

// account_metadata_object_type = 2

struct account_metadata_object_
{
    account_name          name; //< name should not be changed within a chainbase modifier lambda
    uint64_t              recv_sequence = 0;
    uint64_t              auth_sequence = 0;
    uint64_t              code_sequence = 0;
    uint64_t              abi_sequence  = 0;
    digest_type           code_hash;
    time_point            last_code_update;
    uint32_t              flags = 0;
    uint8_t               vm_type = 0;
    uint8_t               vm_version = 0;
};

FC_REFLECT(account_metadata_object_, (name)(recv_sequence)(auth_sequence)(code_sequence)(abi_sequence)
                                                  (code_hash)(last_code_update)(flags)(vm_type)(vm_version))

// permission_object_type = 3

struct permission_object_ {
    permission_usage_object::id_type  usage_id;
    int64_t                           parent; ///< parent permission
    account_name                      owner; ///< the account this permission belongs to (should not be changed within a chainbase modifier lambda)
    permission_name                   name; ///< human-readable name for the permission (should not be changed within a chainbase modifier lambda)
    time_point                        last_updated; ///< the last time this authority was updated
    authority                         auth; ///< authority required to execute this permission
};

FC_REFLECT(permission_object_, (usage_id)(parent)(owner)(name)(last_updated)(auth))

// permission_usage_object_type = 4
struct permission_usage_object_ {
    time_point        last_used;   ///< when this permission was last used
};

FC_REFLECT(permission_usage_object_, (last_used))

// permission_link_object_type = 5
struct permission_link_object_ {
    /// The account which is defining its permission requirements
    account_name    account;
    /// The contract which account requires @ref required_permission to invoke
    account_name    code; /// TODO: rename to scope
    /// The message type which account requires @ref required_permission to invoke
    /// May be empty; if so, it sets a default @ref required_permission for all messages to @ref code
    action_name       message_type;
    /// The permission level which @ref account requires for the specified message types
    /// all of the above fields should not be changed within a chainbase modifier lambda
    permission_name required_permission;
};

FC_REFLECT(permission_link_object_, (account)(code)(message_type)(required_permission))

// key_value_object_type = 7
struct key_value_object_ {
    int64_t              t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t             primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name         payer;
    vector<char>         value;
};

FC_REFLECT(key_value_object_, (primary_key)(payer)(value) )

// index64_object_type = 8
struct index64_object_ {
    int64_t       t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t      primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name  payer;
    uint64_t  secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
};
FC_REFLECT(index64_object_, (t_id)(primary_key)(payer)(secondary_key) )

// index128_object_type = 9
struct index128_object_ {
    int64_t       t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t      primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name  payer;
    uint128_t  secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
};
FC_REFLECT(index128_object_, (t_id)(primary_key)(payer)(secondary_key) )

// index256_object_type = 10
struct index256_object_ {
    int64_t       t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t      primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name  payer;
    key256_t  secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
};
FC_REFLECT(index256_object_, (t_id)(primary_key)(payer)(secondary_key) )

// index_double_object_type = 11
struct index_double_object_ {
    int64_t       t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t      primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name  payer;
    double  secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
};
FC_REFLECT(index_double_object_, (t_id)(primary_key)(payer)(secondary_key) )

// index_long_double_object_type = 12
struct index_long_double_object_ {
    int64_t       t_id; //< t_id should not be changed within a chainbase modifier lambda
    uint64_t      primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    account_name  payer;
    long double  secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
};
FC_REFLECT(index_long_double_object_, (t_id)(primary_key)(payer)(secondary_key) )

// global_property_object_type = 13
struct global_property_object_ {
    // id_type                             id;
    std::optional<block_num_type>       proposed_schedule_block_num;
    producer_authority_schedule         proposed_schedule;
    chain_config                        configuration;
    chain_id_type                       chain_id = chain_id_type::empty_chain_id();
    kv_database_config                  kv_configuration;
    wasm_config                         wasm_configuration;
};

FC_REFLECT(global_property_object_,
            (proposed_schedule_block_num)(proposed_schedule)(configuration)(chain_id)(kv_configuration)(wasm_configuration)
          )

// dynamic_global_property_object_type = 14
struct dynamic_global_property_object_ {
    uint64_t   global_action_sequence = 0;
};

FC_REFLECT(dynamic_global_property_object_,
            (global_action_sequence)
          )

// block_summary_object_type = 15
struct block_summary_object_ {
    block_id_type  block_id;
};

FC_REFLECT(block_summary_object_,
            (block_id)
          )

// transaction_object_type = 16
struct transaction_object_ {
    time_point_sec      expiration;
    transaction_id_type trx_id; //< trx_id should not be changed within a chainbase modifier lambda
};

FC_REFLECT(transaction_object_, (expiration)(trx_id))

// generated_transaction_object_type = 17
struct generated_transaction_object_ {
    transaction_id_type           trx_id;
    account_name                  sender;
    uint128_t                     sender_id;
    account_name                  payer;
    time_point                    delay_until; /// this generated transaction will not be applied until the specified time
    time_point                    expiration; /// this generated transaction will not be applied after this time
    time_point                    published;
    vector<char>                  packed_trx;
};

// # UNUSED_producer_object_type = 18
// # UNUSED_chain_property_object_type = 19
// # account_control_history_object_type = 20 #     ///< Defined by history_plugin
// # UNUSED_account_transaction_history_object_type = 21
// # UNUSED_transaction_history_object_type = 22
// # public_key_history_object_type = 23          ///< Defined by history_plugin
// # UNUSED_balance_object_type = 24
// # UNUSED_staked_balance_object_type = 25
// # UNUSED_producer_votes_object_type = 26
// # UNUSED_producer_schedule_object_type = 27
// # UNUSED_proxy_vote_object_type = 28
// # UNUSED_scope_sequence_object_type = 29
// table_id_object_type = 30
struct table_id_object_ {
    account_name   code;  //< code should not be changed within a chainbase modifier lambda
    scope_name     scope; //< scope should not be changed within a chainbase modifier lambda
    table_name     table; //< table should not be changed within a chainbase modifier lambda
    account_name   payer;
    uint32_t       count = 0; /// the number of elements in the table
};
FC_REFLECT(table_id_object_, (code)(scope)(table)(payer)(count) )

// resource_limits_object_type = 31
struct resource_limits_object_ {
    account_name owner; //< owner should not be changed within a chainbase modifier lambda
    bool pending = false; //< pending should not be changed within a chainbase modifier lambda

    int64_t net_weight = -1;
    int64_t cpu_weight = -1;
    int64_t ram_bytes = -1;
};
FC_REFLECT(resource_limits_object_, (owner)(net_weight)(cpu_weight)(ram_bytes))

// resource_usage_object_type = 32
// @ignore pending
struct resource_usage_object_ {
    account_name owner; //< owner should not be changed within a chainbase modifier lambda
    resource_limits::usage_accumulator        net_usage;
    resource_limits::usage_accumulator        cpu_usage;
    uint64_t                 ram_usage = 0;
};
FC_REFLECT(resource_usage_object_,  (owner)(net_usage)(cpu_usage)(ram_usage))
// resource_limits_state_object_type = 33
struct resource_limits_state_object_ {
    resource_limits::usage_accumulator average_block_net_usage;
    resource_limits::usage_accumulator average_block_cpu_usage;

    // void update_virtual_net_limit( const resource_limits_config_object& cfg );
    // void update_virtual_cpu_limit( const resource_limits_config_object& cfg );

    uint64_t pending_net_usage = 0ULL;
    uint64_t pending_cpu_usage = 0ULL;

    uint64_t total_net_weight = 0ULL;
    uint64_t total_cpu_weight = 0ULL;
    uint64_t total_ram_bytes = 0ULL;

    uint64_t virtual_net_limit = 0ULL;
    uint64_t virtual_cpu_limit = 0ULL;

};
FC_REFLECT(resource_limits_state_object_, (average_block_net_usage)(average_block_cpu_usage)(pending_net_usage)(pending_cpu_usage)(total_net_weight)(total_cpu_weight)(total_ram_bytes)(virtual_net_limit)(virtual_cpu_limit))

// resource_limits_config_object_type = 34
struct resource_limits_config_object_ {

    resource_limits::elastic_limit_parameters cpu_limit_parameters = {EOS_PERCENT(config::default_max_block_cpu_usage, config::default_target_block_cpu_usage_pct), config::default_max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};
    resource_limits::elastic_limit_parameters net_limit_parameters = {EOS_PERCENT(config::default_max_block_net_usage, config::default_target_block_net_usage_pct), config::default_max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};

    uint32_t account_cpu_usage_average_window = config::account_cpu_usage_average_window_ms / config::block_interval_ms;
    uint32_t account_net_usage_average_window = config::account_net_usage_average_window_ms / config::block_interval_ms;
};
FC_REFLECT(resource_limits_config_object_, (cpu_limit_parameters)(net_limit_parameters)(account_cpu_usage_average_window)(account_net_usage_average_window))

// # UNUSED_account_history_object_type = 35 #              ///< Defined by history_plugin
// # UNUSED_action_history_object_type = 36 #               ///< Defined by history_plugin
// # UNUSED_reversible_block_object_type = 37
// protocol_state_object_type = 38
struct activated_protocol_feature_ {
    digest_type feature_digest;
    uint32_t    activation_block_num = 0;
};

struct protocol_state_object_ {
    vector<activated_protocol_feature_>        activated_protocol_features;
    vector<digest_type>                        preactivated_protocol_features;
    vector<string>                             whitelisted_intrinsics;
    uint32_t                                   num_supported_key_types = 0;
};

FC_REFLECT(activated_protocol_feature_,
            (feature_digest)(activation_block_num)
          )

FC_REFLECT(protocol_state_object_,
            (activated_protocol_features)(preactivated_protocol_features)(whitelisted_intrinsics)(num_supported_key_types)
          )

// account_ram_correction_object_type = 39
struct account_ram_correction_object_ {
    account_name name; //< name should not be changed within a chainbase modifier lambda
    uint64_t     ram_correction = 0;
};
FC_REFLECT(account_ram_correction_object_, (name)(ram_correction))

// code_object_type = 40
struct code_object_ {
    digest_type  code_hash; //< code_hash should not be changed within a chainbase modifier lambda
    vector<char>  code;
    uint64_t     code_ref_count;
    uint32_t     first_block_used;
    uint8_t      vm_type = 0; //< vm_type should not be changed within a chainbase modifier lambda
    uint8_t      vm_version = 0; //< vm_version should not be changed within a chainbase modifier lambda
};
FC_REFLECT(code_object_, (code_hash)(code)(code_ref_count)(first_block_used)(vm_type)(vm_version))

// database_header_object_type = 41
struct database_header_object_ {
    uint32_t       version;
};
FC_REFLECT( database_header_object_, (version) )

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
unpack_tuple(fc::datastream<const char*>& binary, std::tuple<Tp...>& t)
{ }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
unpack_tuple(fc::datastream<const char*>& binary, std::tuple<Tp...>& t)
{
   fc::raw::unpack(binary, std::get<I>(t));
    unpack_tuple<I + 1, Tp...>(binary, t);
}

template<typename database_object_type>
inline vector<char> pack_database_object(const database_object_type& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 8);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<key_value_object>(const key_value_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index64_object>(const index64_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index128_object>(const index128_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index256_object>(const index256_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index_double_object>(const index_double_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index_long_double_object>(const index_long_double_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<resource_limits::resource_limits_object>(const resource_limits::resource_limits_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 8 + 1);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.owner);
    fc::raw::pack(ds, obj.pending);
    fc::raw::pack(ds, obj.net_weight);
    fc::raw::pack(ds, obj.cpu_weight);
    fc::raw::pack(ds, obj.ram_bytes);
    return vec;
}

template<typename database_object>
inline database_object unpack_database_object(const char *raw_data, size_t raw_data_size) {
    std::vector<char> vec(raw_data, raw_data + raw_data_size);
    return fc::raw::unpack<database_object>(vec);
}

FC_REFLECT(generated_transaction_object_, (trx_id)(sender)(sender_id)(payer)(delay_until)(expiration)(published)(packed_trx))
