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


template<int object_type, typename object_index, typename index_name>
int32_t database_object_walk(chainbase::database& db, fn_data_handler handler, void *custom_data) {
    const auto& idx = db.get_index<object_index, index_name>();

    const auto& first = idx.begin();
    const auto& last = idx.end();
    for (auto itr = first; itr != last; ++itr) {
        vector<char> data = pack_database_object(*itr);
        auto ret = handler(object_type, data.data(), data.size(), custom_data);
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
        FC_ASSERT(0, "index position out of range ${index_position}", ("index_position", index_position)); \
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

int32_t database_proxy::walk(void *_db, int32_t tp, int32_t index_position) {
    if (this->handler == nullptr) {
        elog("database handler not set");
        return -1;
    }

    auto& db = *static_cast<chainbase::database *>(_db);

    try {
        // by_name account_name
        HANDLE_DATABASE_OBJECT_WALK(account, (by_id)(by_name))
        // by_name account_name
        HANDLE_DATABASE_OBJECT_WALK(account_metadata, (by_id)(by_name))
    //    struct by_parent;
    //    struct by_owner;
    //    struct by_name;
        HANDLE_DATABASE_OBJECT_WALK(permission, (by_id)(by_parent)(eosio::chain::by_owner)(by_name))
        HANDLE_DATABASE_OBJECT_WALK(permission_usage, (by_id))
    //    struct by_action_name;
    //    struct by_permission_name;
        HANDLE_DATABASE_OBJECT_WALK(permission_link, (by_id)(by_action_name)(by_permission_name))
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

        //by_trx_id
        //by_expiration
        HANDLE_DATABASE_OBJECT_WALK_EX(transaction, (by_id)(by_trx_id)(by_expiration))

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

        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_limits_state, (by_id))

        HANDLE_DATABASE_OBJECT_WALK_RESOURCE_LIMITS(resource_limits_config, (by_id))

        HANDLE_DATABASE_OBJECT_WALK_EX(protocol_state, (by_id))
    // by_id
    // by_name
        HANDLE_DATABASE_OBJECT_WALK(account_ram_correction, (by_id)(by_name))
    // by_id
    // by_code_hash
        HANDLE_DATABASE_OBJECT_WALK(code, (by_id)(by_code_hash))

        HANDLE_DATABASE_OBJECT_WALK_EX(database_header, (by_id))

        FC_ASSERT(0, "unhandled walk request: ${tp} ${pos}", ("tp", tp)("pos", index_position));

    } CATCH_AND_LOG_EXCEPTION();

    return -2;
}
