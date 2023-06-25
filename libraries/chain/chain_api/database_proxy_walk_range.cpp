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

// walk range by id_type
template<int tp, typename object_index, typename object_id_type>
int32_t database_object_walk_range_by_id_type(chainbase::database& db, fc::datastream<const char*>& lower_bound_stream, fc::datastream<const char*>& upper_bound_stream, fn_data_handler handler, void *custom_data) {
    object_id_type lower_bound;
    object_id_type upper_bound;
    fc::raw::unpack(lower_bound_stream, lower_bound._id);
    fc::raw::unpack(upper_bound_stream, upper_bound._id);
    const auto& idx = db.get_index<object_index, by_id>();
    auto begin_itr = idx.lower_bound(lower_bound);
    auto end_itr = idx.lower_bound(upper_bound);
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        vector<char> data = pack_database_object(*itr);
        auto ret = handler(tp, data.data(), data.size(), custom_data);
        if (!ret) {
            return ret;
        }
    }
    return 1;
}

// walk range by index type
template<int tp, typename object_index, typename index_name, typename key_type>
int32_t database_object_walk_range(chainbase::database& db, fc::datastream<const char*>& lower_bound_stream, fc::datastream<const char*>& upper_bound_stream, fn_data_handler handler, void *custom_data) {
    key_type lower_bound;
    key_type upper_bound;
    fc::raw::unpack(lower_bound_stream, lower_bound);
    fc::raw::unpack(upper_bound_stream, upper_bound);
    const auto& idx = db.get_index<object_index, index_name>();
    auto begin_itr = idx.lower_bound(lower_bound);
    auto end_itr = idx.lower_bound(upper_bound);
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        vector<char> data = pack_database_object(*itr);
        auto ret = handler(tp, data.data(), data.size(), custom_data);
        if (!ret) {
            return ret;
        }
    }
    return 1;
}

// walk range by index tuple type
template<int tp, typename object_index, typename index_name, typename... Ts>
int32_t database_object_walk_range_by_composite_key(chainbase::database& db, fc::datastream<const char*>& lower_bound_stream, fc::datastream<const char*>& upper_bound_stream, fn_data_handler handler, void *custom_data) {
    std::tuple<Ts...> lower_bound;
    std::tuple<Ts...> upper_bound;
    unpack_tuple(lower_bound_stream, lower_bound);
    unpack_tuple(upper_bound_stream, upper_bound);
    const auto& idx = db.get_index<object_index, index_name>();
    auto begin_itr = idx.lower_bound(lower_bound);
    auto end_itr = idx.lower_bound(upper_bound);
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        vector<char> data = pack_database_object(*itr);
        auto ret = handler(tp, data.data(), data.size(), custom_data);
        if (!ret) {
            return ret;
        }
    }
    return 1;
}

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_EX(OBJECT_NAME, OBJECT_INDEX) \
    if (tp == OBJECT_NAME##_object_type) { \
        OBJECT_NAME##_object::id_type lower_bound; \
        OBJECT_NAME##_object::id_type upper_bound; \
        fc::raw::unpack(lower_bound_stream, lower_bound._id); \
        fc::raw::unpack(upper_bound_stream, upper_bound._id); \
        const auto& idx = db.get_index<eosio::chain::OBJECT_INDEX, by_id>(); \
        auto begin_itr = idx.lower_bound(lower_bound); \
        auto end_itr = idx.lower_bound(upper_bound); \
        for (auto itr = begin_itr; itr != end_itr; ++itr) { \
            vector<char> data = pack_database_object(*itr); \
            auto ret = this->handler(tp, data.data(), data.size(), this->custom_data); \
            if (!ret) { \
                return ret; \
            } \
        } \
    }

#define HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(OBJECT_NAME, SECONDARY_KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp) { \
        if (index_position == 0) { \
            return database_object_walk_range_by_id_type<OBJECT_NAME##_object_type, OBJECT_NAME##_index, OBJECT_NAME##_object::id_type>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
        } \
        if (index_position == 1) { \
            return database_object_walk_range_by_composite_key<OBJECT_NAME##_object_type, OBJECT_NAME##_index, by_primary, table_id, uint64_t>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
        } \
        if (index_position == 2) { \
            return database_object_walk_range_by_composite_key<OBJECT_NAME##_object_type, OBJECT_NAME##_index, by_secondary, table_id, SECONDARY_KEY_TYPE, uint64_t>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
        } \
        FC_ASSERT(0, "HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE: invalid index position"); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(OBJECT_NAME, OBJECT_INDEX) \
    if (OBJECT_NAME##_object_type == tp) { \
        if (index_position == 0) { \
            return database_object_walk_range_by_id_type<OBJECT_NAME##_object_type, OBJECT_INDEX, OBJECT_NAME##_object::id_type>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
        } \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(OBJECT_NAME) \
    HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(OBJECT_NAME, OBJECT_NAME##_index)


#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp && index_position == INDEX_POSITION) { \
        return database_object_walk_range<OBJECT_NAME##_object_type, OBJECT_NAME##_index, INDEX_NAME, KEY_TYPE>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE_EX(OBJECT_NAME, OBJECT_INDEX, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp && index_position == INDEX_POSITION) { \
        return database_object_walk_range<OBJECT_NAME##_object_type, OBJECT_INDEX, INDEX_NAME, KEY_TYPE>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(OBJECT_NAME, OBJECT_INDEX, INDEX_POSITION, INDEX_NAME, ...) \
    if (OBJECT_NAME##_object_type == tp && index_position == INDEX_POSITION) { \
        return database_object_walk_range_by_composite_key<OBJECT_NAME##_object_type, OBJECT_INDEX, INDEX_NAME, __VA_ARGS__>(db, lower_bound_stream, upper_bound_stream, handler, custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(OBJECT_NAME, OBJECT_NAME##_index, INDEX_POSITION, INDEX_NAME, __VA_ARGS__)

int32_t database_proxy::walk_range(void *_db, int32_t tp, int32_t index_position, const char *raw_lower_bound, size_t raw_lower_bound_size, const char *raw_upper_bound, size_t raw_upper_bound_size) {
    try {
        if (this->handler == nullptr) {
            elog("database handler not set");
            return -1;
        }

        fc::datastream<const char*> lower_bound_stream(raw_lower_bound, raw_lower_bound_size);
        fc::datastream<const char*> upper_bound_stream(raw_upper_bound, raw_upper_bound_size);
        auto& db = *static_cast<chainbase::database *>(_db);

        // by_name>, account_name
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(account)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE(account, 1, by_name, account_name)
        // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
        // ordered_unique<tag<by_name>, member<account_metadata_object, account_name, &account_metadata_object::name>>
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(account_metadata)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE(account_metadata, 1, by_name, account_name)
        // by_parent
        //     permission_object::id_type, &permission_object::parent
        //     permission_object::id_type, &permission_object::id
        // by_owner
        //     account_name, &permission_object::owner
        //     permission_name, &permission_object::name
        // by_name
        //     permission_name, &permission_object::name
        //     permission_object::id_type, &permission_object::id
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(permission)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(permission, 1, by_parent, permission_object::id_type, permission_object::id_type)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(permission, 2, eosio::chain::by_owner, account_name, permission_name)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(permission, 3, by_name, permission_name, permission_object::id_type)

        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(permission_usage)
        // ordered_unique<tag<by_id>,
        // ordered_unique<tag<by_action_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
        // ordered_unique<tag<by_permission_name>,
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_name, required_permission),
        //     BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_link_object::id_type, id)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(permission_link)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(permission_link, 1, by_action_name, account_name, account_name, account_name)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(permission_link, 2, by_permission_name, account_name, permission_name, permission_link_object::id_type)

        // ordered_unique<tag<by_id>, member<key_value_object, key_value_object::id_type, &key_value_object::id>>,
        // ordered_unique<tag<by_scope_primary>,
        //     member<key_value_object, table_id, &key_value_object::t_id>,
        //     member<key_value_object, uint64_t, &key_value_object::primary_key>
        // >,
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(key_value)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(key_value, 1, by_scope_primary, table_id, uint64_t)

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
        HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(index64, uint64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(index128, uint128_t)
        HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(index256, key256_t)
        HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(index_double, float64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_WALK_RANGE(index_long_double, float128_t)

        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(global_property, global_property_multi_index)
        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(dynamic_global_property, dynamic_global_property_multi_index)

    // ordered_unique<tag<by_id>
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(block_summary, block_summary_multi_index)

        // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_object::id_type, id)>,
        // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_id_type, trx_id)>,
        // ordered_unique< tag<by_expiration>,
        // composite_key< transaction_object,
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, time_point_sec, expiration ),
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, transaction_object::id_type, id)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(transaction, transaction_multi_index)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE_EX(transaction, transaction_multi_index, 1, by_trx_id, transaction_id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(transaction, transaction_multi_index, 2, by_expiration, time_point_sec, transaction_object::id_type);

    //    by_id
    //    struct by_code_scope_table;
        // ordered_unique<tag<by_id>,
        // ordered_unique<tag<by_code_scope_table>,
        //     member<table_id_object, account_name, &table_id_object::code>,
        //     member<table_id_object, scope_name,   &table_id_object::scope>,
        //     member<table_id_object, table_name,   &table_id_object::table>
        // >
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(table_id, table_id_multi_index)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(table_id, table_id_multi_index, 1, by_code_scope_table, account_name, scope_name, table_name);

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
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(generated_transaction, generated_transaction_multi_index)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE_EX(generated_transaction, generated_transaction_multi_index, 1, by_trx_id, transaction_id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 2, by_expiration, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 3, by_delay, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 4, by_sender_id, account_name, uint128_t);

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(resource_limits, resource_limits::resource_limits_index)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(resource_limits, resource_limits::resource_limits_index, 1, resource_limits::by_owner, bool, account_name);

    // by_id
    // by_owner
        // ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
        // ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(resource_usage, resource_limits::resource_usage_index)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE_EX(resource_usage, resource_limits::resource_usage_index, 1, resource_limits::by_owner, account_name);

    // by_id
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(resource_limits_state, resource_limits::resource_limits_state_index)
    // by_id
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(resource_limits_config, resource_limits::resource_limits_config_index)

    // by_id
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(protocol_state, protocol_state_multi_index)
    // by_id
    // by_name
        // ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
        // ordered_unique<tag<by_name>, member<account_ram_correction_object, account_name, &account_ram_correction_object::name>>
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(account_ram_correction)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_INDEX_TYPE(account_ram_correction, 1, by_name, account_name)

    // by_id
    // by_code_hash
        // ordered_unique<tag<by_id>, member<code_object, code_object::id_type, &code_object::id>>,
        // ordered_unique<tag<by_code_hash>,
        //     member<code_object, digest_type, &code_object::code_hash>,
        //     member<code_object, uint8_t,     &code_object::vm_type>,
        //     member<code_object, uint8_t,     &code_object::vm_version>

        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID(code)
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY(code, 1, by_code_hash, digest_type, uint8_t, uint8_t)

    // by_id
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(database_header, database_header_multi_index)
        FC_ASSERT(0, "unhandled walk range request: ${tp} ${pos}", ("tp", tp)("pos", index_position));
    } CATCH_AND_LOG_EXCEPTION();
    return -2;
}
