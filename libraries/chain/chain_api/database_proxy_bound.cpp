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

// bound by id_type
template<int bound_type, int tp, typename object_index, typename object_id_type>
int database_object_bound_by_id_type(chainbase::database& db, fc::datastream<const char*>& key_stream, vector<char>& buffer) {
    object_id_type bound;
    fc::raw::unpack(key_stream, bound._id);
    const auto& idx = db.get_index<object_index, by_id>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    }
    return 0;
}

// bound by index type
template<int bound_type, int tp, typename object_index, typename index_name, typename key_type>
int database_object_bound(chainbase::database& db, fc::datastream<const char*>& key_stream, vector<char>& buffer) {
    key_type bound;
    fc::raw::unpack(key_stream, bound);
    const auto& idx = db.get_index<object_index, index_name>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    }
    return 0;
}

// bound by index tuple type
template<int bound_type, int tp, typename object_index, typename index_name, typename... Ts>
int database_object_bound_by_composite_key(chainbase::database& db, fc::datastream<const char*>& bound_stream, vector<char>& buffer) {
    std::tuple<Ts...> bound;
    unpack_tuple(bound_stream, bound);
    const auto& idx = db.get_index<object_index, index_name>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            buffer = pack_database_object(*itr);
            return 1;
        }
    }
    return 0;
}

#define HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        return database_object_bound_by_id_type<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, OBJECT_NAME##_object::id_type>(db, bound_stream, out); \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_ID(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        return database_object_bound_by_id_type<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, OBJECT_NAME##_object::id_type>(db, bound_stream, out); \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        return database_object_bound<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, INDEX_NAME, KEY_TYPE>(db, bound_stream, out); \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_KEY_EX(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        return database_object_bound<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, INDEX_NAME, KEY_TYPE>(db, bound_stream, out); \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        return database_object_bound_by_composite_key<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, INDEX_NAME, __VA_ARGS__>(db, bound_stream, out); \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        return database_object_bound_by_composite_key<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, INDEX_NAME, __VA_ARGS__>(db, bound_stream, out); \
    }

#define HANDLE_CONTRACT_TABLE_OBJECT_BOUND(OBJECT_NAME, SECONDARY_KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp) { \
        if (index_position == 0) { \
            HANDLE_DATABASE_OBJECT_BOUND_BY_ID(OBJECT_NAME); \
        } \
        if (index_position == 1) { \
            HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(OBJECT_NAME, 1, by_primary, table_id, uint64_t) \
        } \
        if (index_position == 2) { \
            HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(OBJECT_NAME, 2, by_secondary, table_id, SECONDARY_KEY_TYPE, uint64_t); \
        } \
        FC_ASSERT(0, "HANDLE_CONTRACT_TABLE_OBJECT_BOUND: invalid index position ${n}", ("n", index_position)); \
    }


template<int bound_type>
int32_t bound(void *_db, int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char>& out) {
    fc::datastream<const char*> bound_stream(raw_data, size);
    auto& db = *static_cast<chainbase::database *>(_db);

    try {
        // name code, scope, table;
        // const auto* t_id = db.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(code, scope, table));

        // HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(table_id, 1, by_code_scope_table, name, name, name);

    //     // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
        // ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(account)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(account, 1, by_name, account_name)
        // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
        // ordered_unique<tag<by_name>, member<account_metadata_object, account_name, &account_metadata_object::name>>
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(account_metadata)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(account_metadata, 1, by_name, account_name)
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
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(permission)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(permission, 1, by_parent, permission_object::id_type, permission_object::id_type)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(permission, 2, eosio::chain::by_owner, account_name, permission_name)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(permission, 3, by_name, permission_name, permission_object::id_type)

        // ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(permission_usage)
    //    struct by_action_name;
    //    struct by_permission_name;
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(permission_link)
        // ordered_unique<tag<by_action_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
        // ordered_unique<tag<by_permission_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_name, required_permission),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_link_object::id_type, id)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(permission_link, 1, by_action_name, account_name, account_name, account_name)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(permission_link, 2, by_permission_name, account_name, permission_name, permission_link_object::id_type)

    //     by_id
    //     by_scope_primary
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(key_value)
    // ordered_unique<tag<by_scope_primary>,
    //     member<key_value_object, table_id, &key_value_object::t_id>,
    //     member<key_value_object, uint64_t, &key_value_object::primary_key>
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(key_value, 1, by_scope_primary, table_id, uint64_t)

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
        HANDLE_CONTRACT_TABLE_OBJECT_BOUND(index64, uint64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_BOUND(index128, uint128_t)
        HANDLE_CONTRACT_TABLE_OBJECT_BOUND(index256, key256_t)
        HANDLE_CONTRACT_TABLE_OBJECT_BOUND(index_double, float64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_BOUND(index_long_double, float128_t)

        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(global_property)
        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(dynamic_global_property)

    // ordered_unique<tag<by_id>
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(block_summary)

        // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_object::id_type, id)>,
        // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_id_type, trx_id)>,
        // ordered_unique< tag<by_expiration>,
        // composite_key< transaction_object,
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, time_point_sec, expiration ),
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, transaction_object::id_type, id)
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(transaction)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY_EX(transaction, 1, by_trx_id, transaction_id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(transaction, 2, by_expiration, time_point_sec, transaction_object::id_type);

    //    by_id
    //    struct by_code_scope_table;
        // ordered_unique<tag<by_id>,
        // ordered_unique<tag<by_code_scope_table>,
        //     member<table_id_object, account_name, &table_id_object::code>,
        //     member<table_id_object, scope_name,   &table_id_object::scope>,
        //     member<table_id_object, table_name,   &table_id_object::table>
        // >
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(table_id)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(table_id, 1, by_code_scope_table, account_name, scope_name, table_name);

    //    struct by_trx_id;
    //    struct by_expiration;
    //    struct by_delay;
    //    struct by_sender_id;

        // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(generated_transaction_object, generated_transaction_object::id_type, id)>,
        // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, transaction_id_type, trx_id)>,
        // ordered_unique< tag<by_expiration>,
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, expiration),
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
        // >
        // >,
        // ordered_unique< tag<by_delay>,
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, delay_until),
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
        // >
        // >,
        // ordered_unique< tag<by_sender_id>,
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, account_name, sender),
        //     BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, uint128_t, sender_id)
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(generated_transaction)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY_EX(generated_transaction, 1, by_trx_id, transaction_id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 2, by_expiration, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 3, by_delay, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 4, by_sender_id, account_name, uint128_t);

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(resource_limits)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(resource_limits, 1, resource_limits::by_owner, bool, account_name);

    // by_id
    // by_owner
        // ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
        // ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(resource_usage)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(resource_usage, 1, resource_limits::by_owner, account_name);

    // by_id
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(resource_limits_state)
    // by_id
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(resource_limits_config)

    // by_id
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(protocol_state)
    // by_id
    // by_name
        // ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
        // ordered_unique<tag<by_name>, member<account_ram_correction_object, account_name, &account_ram_correction_object::name>>
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(account_ram_correction)
        HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(account_ram_correction, 1, by_name, account_name)

    // by_id
    // by_code_hash
        // ordered_unique<tag<by_id>, member<code_object, code_object::id_type, &code_object::id>>,
        // ordered_unique<tag<by_code_hash>,
        //     member<code_object, digest_type, &code_object::code_hash>,
        //     member<code_object, uint8_t,     &code_object::vm_type>,
        //     member<code_object, uint8_t,     &code_object::vm_version>

        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(code)
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(code, 1, by_code_hash, digest_type, uint8_t, uint8_t)

    // by_id
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(database_header)

        FC_ASSERT(0, "unhandled bound: ${bound_type} ${tp} ${pos}", ("bound_type", bound_type)("tp", tp)("pos", index_position));

    } CATCH_AND_LOG_EXCEPTION();
    return -2;
}

int32_t database_proxy::lower_bound(void *db, int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char> &out) {
    return bound<0>(db, tp, index_position, raw_data, size, out);
}

int32_t database_proxy::upper_bound(void *db, int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char> &out) {
    return bound<1>(db, tp, index_position, raw_data, size, out);
}
