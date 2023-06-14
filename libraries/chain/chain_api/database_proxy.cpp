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

#include "database_proxy.hpp"
#include "chain_macro.hpp"

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::chain::resource_limits;

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

void database_proxy::set_data_handler(fn_data_handler handler, void *custom_data) {
    this->handler = handler;
    this->custom_data = custom_data;
}

template<int object_type, typename object_index, typename index_name>
int32_t database_object_walk(chainbase::database& db, fn_data_handler handler, void *custom_data) {
    const auto& idx = db.get_index<object_index, index_name>();

    const auto& first = idx.begin();
    const auto& last = idx.end();
    for (auto itr = first; itr != last; ++itr) {
        vector<char> data = fc::raw::pack(*itr);
        auto ret = handler(object_type, itr->id._id, data.data(), data.size(), custom_data);
        if (!ret) {
            return ret;
        }
    }
    return 1;
}

#define GENERATE_DB_WALK(r, OBJECT_NAME, i, INDEX_NAME) \
    if (index_position == i) { \
        return database_object_walk<OBJECT_NAME##_object_type, eosio::chain::OBJECT_NAME##_index, INDEX_NAME>(db, this->handler, this->custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK(OBJECT_NAME, INDEX_NAMES) \
    if (OBJECT_NAME##_object_type == tp) { \
        BOOST_PP_SEQ_FOR_EACH_I(GENERATE_DB_WALK, OBJECT_NAME, INDEX_NAMES) \
        FC_ASSERT(0, "index position out of range"); \
    }

#define GENERATE_DB_WALK_EX(r, OBJECT_NAME, i, INDEX_NAME) \
    if (index_position == i) { \
        return database_object_walk<OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, INDEX_NAME>(db, this->handler, this->custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_EX(OBJECT_NAME, INDEX_NAMES) \
    if (OBJECT_NAME##_object_type == tp) { \
        BOOST_PP_SEQ_FOR_EACH_I(GENERATE_DB_WALK_EX, OBJECT_NAME, INDEX_NAMES) \
        FC_ASSERT(0, "index position out of range"); \
    }


#define GENERATE_DB_WALK_RESOURCE_LIMITS(r, OBJECT_NAME, i, INDEX_NAME) \
    if (index_position == i) { \
        return database_object_walk<OBJECT_NAME##_object_type, resource_limits::OBJECT_NAME##_index, INDEX_NAME>(db, this->handler, this->custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(OBJECT_NAME, INDEX_NAMES) \
    if (OBJECT_NAME##_object_type == tp) { \
        BOOST_PP_SEQ_FOR_EACH_I(GENERATE_DB_WALK_RESOURCE_LIMITS, OBJECT_NAME, INDEX_NAMES) \
        FC_ASSERT(0, "index position out of range"); \
    }

#define GENERATE_DB_WALK_RESOURCE_LIMITS_EX(r, OBJECT_NAME, i, INDEX_NAME) \
    if (index_position == i) { \
        return database_object_walk<OBJECT_NAME##_object_type, resource_limits::OBJECT_NAME##_multi_index, INDEX_NAME>(db, this->handler, this->custom_data); \
    }

#define HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS_EX(OBJECT_NAME, INDEX_NAMES) \
    if (OBJECT_NAME##_object_type == tp) { \
        BOOST_PP_SEQ_FOR_EACH_I(GENERATE_DB_WALK_RESOURCE_LIMITS_EX, OBJECT_NAME, INDEX_NAMES) \
        FC_ASSERT(0, "index position out of range"); \
    }
//    enum object_type
//    {
//       null_object_type = 0,
//       account_object_type,
//       account_metadata_object_type,
//       permission_object_type,
//       permission_usage_object_type,
//       permission_link_object_type,
//       UNUSED_action_code_object_type,
//       key_value_object_type,
//       index64_object_type,
//       index128_object_type,
//       index256_object_type,
//       index_double_object_type,
//       index_long_double_object_type,
//       global_property_object_type,

//       dynamic_global_property_object_type,
//       block_summary_object_type,
//       transaction_object_type,
//       generated_transaction_object_type,
//       UNUSED_producer_object_type,
//       UNUSED_chain_property_object_type,
//       account_control_history_object_type,     ///< Defined by history_plugin
//       UNUSED_account_transaction_history_object_type,
//       UNUSED_transaction_history_object_type,
//       public_key_history_object_type,          ///< Defined by history_plugin
//       UNUSED_balance_object_type,
//       UNUSED_staked_balance_object_type,
//       UNUSED_producer_votes_object_type,
//       UNUSED_producer_schedule_object_type,
//       UNUSED_proxy_vote_object_type,
//       UNUSED_scope_sequence_object_type,
//       table_id_object_type,
//       resource_limits_object_type,
//       resource_usage_object_type,
//       resource_limits_state_object_type,
//       resource_limits_config_object_type,
//       UNUSED_account_history_object_type,              ///< Defined by history_plugin
//       UNUSED_action_history_object_type,               ///< Defined by history_plugin
//       UNUSED_reversible_block_object_type,
//       protocol_state_object_type,
//       account_ram_correction_object_type,
//       code_object_type,
//       database_header_object_type,
//       OBJECT_TYPE_COUNT ///< Sentry value which contains the number of different object types
//    };

int32_t database_proxy::walk(void *_db, int32_t tp, int32_t index_position) {
    if (this->handler == nullptr) {
        elog("database handler not set");
        return -1;
    }

    auto& db = *static_cast<chainbase::database *>(_db);

    try {
        // by_name account_name
        HANDLE_DATABASE_OBJECT_WALK(account, (by_id))
        // by_name account_name
        HANDLE_DATABASE_OBJECT_WALK(account_metadata, (by_id)(by_name))
    //    struct by_parent;
    //    struct by_owner;
    //    struct by_name;
        HANDLE_DATABASE_OBJECT_WALK(permission, (by_id)(by_parent)(eosio::chain::by_owner)(by_name))
        HANDLE_DATABASE_OBJECT_WALK(permission_usage, (by_id))
    //    struct by_action_name;
    //    struct by_permission_name;
        HANDLE_DATABASE_OBJECT_WALK(permission_link, (by_id)(by_action_name))
    //     by_id
    //     by_scope_primary
        HANDLE_DATABASE_OBJECT_WALK(key_value, (by_id)(by_scope_primary))
    //   by_id
    //   by_primary
    //   by_secondary
        HANDLE_DATABASE_OBJECT_WALK(index64, (by_id)(by_primary)(by_secondary))
        HANDLE_DATABASE_OBJECT_WALK(index128, (by_id)(by_primary))
        HANDLE_DATABASE_OBJECT_WALK(index256, (by_id)(by_primary))
        HANDLE_DATABASE_OBJECT_WALK(index_double, (by_id)(by_primary))
        HANDLE_DATABASE_OBJECT_WALK(index_long_double, (by_id)(by_primary))

        HANDLE_DATABASE_OBJECT_WALK_EX(global_property, (by_id))
        HANDLE_DATABASE_OBJECT_WALK_EX(dynamic_global_property, (by_id))
        HANDLE_DATABASE_OBJECT_WALK_EX(block_summary, (by_id))
        HANDLE_DATABASE_OBJECT_WALK_EX(transaction, (by_id))
    //    by_id
    //    struct by_code_scope_table;
        HANDLE_DATABASE_OBJECT_WALK_EX(table_id, (by_id)(by_code_scope_table))

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
        HANDLE_DATABASE_OBJECT_WALK_EX(generated_transaction, (by_id)(by_trx_id)(by_expiration)(by_delay)(by_sender_id))

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_limits, (by_id)(resource_limits::by_owner))

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_usage, (by_id)(resource_limits::by_owner))

        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_limits, (by_id))
        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_limits_config, (by_id))

        HANDLE_DATABASE_OBJECT_WALK_EX(protocol_state, (by_id))
    // by_id
    // by_name
        HANDLE_DATABASE_OBJECT_WALK(account_ram_correction, (by_id)(by_name))
    // by_id
    // by_code_hash
        HANDLE_DATABASE_OBJECT_WALK(code, (by_id))
        HANDLE_DATABASE_OBJECT_WALK_EX(database_header, (by_id))

        FC_ASSERT(0, "unhandled walk request: ${tp} ${pos}", ("tp", tp)("pos", index_position));

    } CATCH_AND_LOG_EXCEPTION();

    return -2;
}

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
        vector<char> data = fc::raw::pack(*itr);
        auto ret = handler(tp, itr->id._id, data.data(), data.size(), custom_data);
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
        vector<char> data = fc::raw::pack(*itr);
        auto ret = handler(tp, itr->id._id, data.data(), data.size(), custom_data);
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
        vector<char> data = fc::raw::pack(*itr);
        auto ret = handler(tp, itr->id._id, data.data(), data.size(), custom_data);
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
            vector<char> data = fc::raw::pack(*itr); \
            auto ret = this->handler(tp, itr->id._id, data.data(), data.size(), this->custom_data); \
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

int32_t database_proxy::walk_range(void *_db, int32_t tp, int32_t index_position, char *raw_lower_bound, size_t raw_lower_bound_size, char *raw_upper_bound, size_t raw_upper_bound_size) {
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
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 1, by_expiration, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 2, by_delay, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_COMPOSITE_KEY_EX(generated_transaction, generated_transaction_multi_index, 3, by_sender_id, account_name, uint128_t);

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_WALK_RANGE_BY_ID_EX(resource_limits, resource_limits::resource_limits_index)

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

#define HANDLE_DATABASE_OBJECT_FIND_BY_ID(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        int64_t _id; \
        fc::raw::unpack(stream, _id); \
        auto key = OBJECT_NAME##_object::id_type(_id); \
        const auto* obj = db.find<OBJECT_NAME##_object, by_id>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = fc::raw::pack(*obj); \
        *out = find_buffer.data(); \
        *out_size = find_buffer.size(); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_FIND_BY_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        KEY_TYPE key; \
        fc::raw::unpack(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = fc::raw::pack(*obj); \
        *out = find_buffer.data(); \
        *out_size = find_buffer.size(); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_FIND(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, INDEX_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        INDEX_TYPE key; \
        fc::raw::unpack(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = fc::raw::pack(*obj); \
        *out = find_buffer.data(); \
        *out_size = find_buffer.size(); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        std::tuple<__VA_ARGS__> key; \
        unpack_tuple(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = fc::raw::pack(*obj); \
        *out = find_buffer.data(); \
        *out_size = find_buffer.size(); \
        return 1; \
    }

#define HANDLE_CONTRACT_TABLE_OBJECT_FIND(OBJECT_NAME, SECONDARY_KEY_TYPE) \
    if (OBJECT_NAME##_object_type == tp) { \
        if (index_position == 0) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_ID(OBJECT_NAME); \
        } \
        if (index_position == 1) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, 1, by_primary, table_id, uint64_t) \
        } \
        if (index_position == 2) { \
            HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(OBJECT_NAME, 2, by_secondary, table_id, SECONDARY_KEY_TYPE, uint64_t); \
        } \
        FC_ASSERT(0, "HANDLE_CONTRACT_TABLE_OBJECT_FIND: invalid index position"); \
    }

// template<typename database_object, typename index_name, typename... Ts>
// int32_t database_object_find_composite_key(chainbase::database& db, fc::datastream<const char*>& stream, char **out, size_t *out_size){
//     std::tuple<Ts...> key;

//     unpack_tuple(stream, key);
//     const auto* obj = db.find<database_object, index_name>(key);
//     if (!obj) {
//         return 0;
//     }
//     find_buffer = fc::raw::pack(*obj);
//     *out = find_buffer.data();
//     *out_size = find_buffer.size();
//     return 1;
// }

int32_t database_proxy::find(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size) {
    fc::datastream<const char*> stream(raw_data, size);
    auto& db = *static_cast<chainbase::database *>(_db);

    try {
        // name code, scope, table;
        // const auto* t_id = db.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(code, scope, table));

        // HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(table_id, 1, by_code_scope_table, name, name, name);

    //     // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
        // ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(account)
        HANDLE_DATABASE_OBJECT_FIND(account, 1, by_name, account_name)
    //     // by_name>, account_name
        // ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
        // ordered_unique<tag<by_name>, member<account_metadata_object, account_name, &account_metadata_object::name>>
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(account_metadata)
        HANDLE_DATABASE_OBJECT_FIND(account_metadata, 1, by_name, account_name)
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
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission)
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 1, by_parent, permission_object::id_type, permission_object::id_type)
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 2, eosio::chain::by_owner, account_name, permission_name)
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(permission, 3, by_name, permission_name, permission_object::id_type)

        // ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission_usage)
    //    struct by_action_name;
    //    struct by_permission_name;
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(permission_link)
    //     by_id
    //     by_scope_primary
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(key_value)

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
        HANDLE_CONTRACT_TABLE_OBJECT_FIND(index64, uint64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_FIND(index128, uint128_t)
        HANDLE_CONTRACT_TABLE_OBJECT_FIND(index256, key256_t)
        HANDLE_CONTRACT_TABLE_OBJECT_FIND(index_double, float64_t)
        HANDLE_CONTRACT_TABLE_OBJECT_FIND(index_long_double, float128_t)

        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(global_property)
        // ordered_unique<tag<by_id>,
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(dynamic_global_property)

    // ordered_unique<tag<by_id>
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(block_summary)

        // ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_object::id_type, id)>,
        // ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_id_type, trx_id)>,
        // ordered_unique< tag<by_expiration>,
        // composite_key< transaction_object,
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, time_point_sec, expiration ),
        //      BOOST_MULTI_INDEX_MEMBER( transaction_object, transaction_object::id_type, id)
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(transaction)
        HANDLE_DATABASE_OBJECT_FIND_BY_KEY(transaction, 1, by_trx_id, transaction_id_type);
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(transaction, 2, by_expiration, time_point_sec, transaction_object::id_type);

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
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(generated_transaction)
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 1, by_expiration, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 2, by_delay, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(generated_transaction, 3, by_sender_id, account_name, uint128_t);

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits)

    // by_id
    // by_owner
        // ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
        // ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_usage)
        HANDLE_DATABASE_OBJECT_FIND_BY_KEY(resource_usage, 1, resource_limits::by_owner, account_name);

    // by_id
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits_state)
    // by_id
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(resource_limits_config)

    // by_id
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(protocol_state)
    // by_id
    // by_name
        // ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
        // ordered_unique<tag<by_name>, member<account_ram_correction_object, account_name, &account_ram_correction_object::name>>
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(account_ram_correction)
        HANDLE_DATABASE_OBJECT_FIND_BY_KEY(account_ram_correction, 1, by_name, account_name)

    // by_id
    // by_code_hash
        // ordered_unique<tag<by_id>, member<code_object, code_object::id_type, &code_object::id>>,
        // ordered_unique<tag<by_code_hash>,
        //     member<code_object, digest_type, &code_object::code_hash>,
        //     member<code_object, uint8_t,     &code_object::vm_type>,
        //     member<code_object, uint8_t,     &code_object::vm_version>

        HANDLE_DATABASE_OBJECT_FIND_BY_ID(code)
        HANDLE_DATABASE_OBJECT_FIND_BY_COMPOSITE_KEY(code, 1, by_code_hash, digest_type, uint8_t, uint8_t)

    // by_id
        HANDLE_DATABASE_OBJECT_FIND_BY_ID(database_header)
        return 1;
    } CATCH_AND_LOG_EXCEPTION();
    return -2;
}


// bound by id_type
template<int bound_type, int tp, typename object_index, typename object_id_type>
vector<char> database_object_bound_by_id_type(chainbase::database& db, fc::datastream<const char*>& key_stream) {
    object_id_type bound;
    fc::raw::unpack(key_stream, bound._id);
    const auto& idx = db.get_index<object_index, by_id>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    }
    return vector<char>();
}

// bound by index type
template<int bound_type, int tp, typename object_index, typename index_name, typename key_type>
vector<char> database_object_bound(chainbase::database& db, fc::datastream<const char*>& key_stream) {
    key_type bound;
    fc::raw::unpack(key_stream, bound);
    const auto& idx = db.get_index<object_index, index_name>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    }
    return vector<char>();
}

// bound by index tuple type
template<int bound_type, int tp, typename object_index, typename index_name, typename... Ts>
vector<char> database_object_bound_by_composite_key(chainbase::database& db, fc::datastream<const char*>& bound_stream) {
    std::tuple<Ts...> bound;
    unpack_tuple(bound_stream, bound);
    const auto& idx = db.get_index<object_index, index_name>();
    if (bound_type == 0) {
        auto itr = idx.lower_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    } else {
        auto itr = idx.upper_bound(bound);
        if (itr != idx.end()) {
            return fc::raw::pack(*itr);
        }
    }
    return vector<char>();
}

#define HANDLE_DATABASE_OBJECT_BOUND_BY_ID_EX(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        find_buffer = database_object_bound_by_id_type<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, OBJECT_NAME##_object::id_type>(db, bound_stream); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_ID(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        find_buffer = database_object_bound_by_id_type<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, OBJECT_NAME##_object::id_type>(db, bound_stream); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        find_buffer = database_object_bound<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, INDEX_NAME, KEY_TYPE>(db, bound_stream); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_KEY_EX(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        find_buffer = database_object_bound<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, INDEX_NAME, KEY_TYPE>(db, bound_stream); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        find_buffer = database_object_bound_by_composite_key<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_index, INDEX_NAME, __VA_ARGS__>(db, bound_stream); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        find_buffer = database_object_bound_by_composite_key<bound_type, OBJECT_NAME##_object_type, OBJECT_NAME##_multi_index, INDEX_NAME, __VA_ARGS__>(db, bound_stream); \
        return 1; \
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
int32_t bound(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t size, vector<char>& find_buffer) {
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
    //     by_id
    //     by_scope_primary
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(key_value)

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
        // HANDLE_DATABASE_OBJECT_BOUND_BY_KEY_EX(transaction, 1, by_trx_id, transaction_id_type);
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
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 1, by_expiration, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 2, by_delay, time_point, generated_transaction_object::id_type);
        HANDLE_DATABASE_OBJECT_BOUND_BY_COMPOSITE_KEY_EX(generated_transaction, 3, by_sender_id, account_name, uint128_t);

    // by_id
    // by_owner
        HANDLE_DATABASE_OBJECT_BOUND_BY_ID(resource_limits)

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

int32_t database_proxy::lower_bound(void *db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size) {
    int32_t ret = bound<0>(db, tp, index_position, raw_data, size, find_buffer);
    if (ret == -2) {
        return -2;
    }
    *out = find_buffer.data();
    *out_size = find_buffer.size();
    return 1;
}

int32_t database_proxy::upper_bound(void *db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size) {
    int32_t ret = bound<1>(db, tp, index_position, raw_data, size, find_buffer);
    if (ret == -2) {
        return -2;
    }
    *out = find_buffer.data();
    *out_size = find_buffer.size();
    return 1;
}

#define DATABASE_OBJECT_ROW_COUNT_EX(OBJECT_NAME, OBJECT_INDEX) \
    if (tp == OBJECT_NAME##_object_type) { \
        return db.get_index<OBJECT_INDEX>().indices().size(); \
    }

#define DATABASE_OBJECT_ROW_COUNT(OBJECT_NAME) \
    DATABASE_OBJECT_ROW_COUNT_EX(OBJECT_NAME, OBJECT_NAME##_index)

uint64_t database_proxy::row_count(void *_db, int32_t tp) {
    auto& db = *static_cast<chainbase::database *>(_db);

    DATABASE_OBJECT_ROW_COUNT(account)
    DATABASE_OBJECT_ROW_COUNT(account_metadata)
    DATABASE_OBJECT_ROW_COUNT(permission)
    DATABASE_OBJECT_ROW_COUNT(permission_usage)
    DATABASE_OBJECT_ROW_COUNT(permission_link)
    DATABASE_OBJECT_ROW_COUNT(key_value)
    DATABASE_OBJECT_ROW_COUNT(index64)
    DATABASE_OBJECT_ROW_COUNT(index128)
    DATABASE_OBJECT_ROW_COUNT(index256)
    DATABASE_OBJECT_ROW_COUNT(index_double)
    DATABASE_OBJECT_ROW_COUNT(index_long_double)
    DATABASE_OBJECT_ROW_COUNT_EX(global_property, global_property_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(dynamic_global_property, dynamic_global_property_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(block_summary, block_summary_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(transaction, transaction_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(generated_transaction, generated_transaction_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(table_id, table_id_multi_index)
    DATABASE_OBJECT_ROW_COUNT_EX(resource_limits, resource_limits::resource_limits_index)
    DATABASE_OBJECT_ROW_COUNT_EX(resource_usage, resource_limits::resource_usage_index)
    DATABASE_OBJECT_ROW_COUNT_EX(resource_limits_state, resource_limits::resource_limits_state_index)
    DATABASE_OBJECT_ROW_COUNT_EX(resource_limits_config, resource_limits::resource_limits_config_index)
    DATABASE_OBJECT_ROW_COUNT_EX(protocol_state, protocol_state_multi_index)
    DATABASE_OBJECT_ROW_COUNT(account_ram_correction)
    DATABASE_OBJECT_ROW_COUNT(code)
    DATABASE_OBJECT_ROW_COUNT_EX(database_header, database_header_multi_index)
    return -2;
}
