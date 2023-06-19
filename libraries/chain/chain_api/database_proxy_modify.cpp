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

#define HANDLE_DATABASE_OBJECT_MODIFY_BY_ID(OBJECT_NAME) \
    if (tp == OBJECT_NAME##_object_type && index_position == 0) { \
        int64_t _id; \
        fc::raw::unpack(stream, _id); \
        auto key = OBJECT_NAME##_object::id_type(_id); \
        const auto* obj = db.find<OBJECT_NAME##_object, by_id>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = pack_database_object(*obj); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_MODIFY_BY_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, KEY_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        KEY_TYPE key; \
        fc::raw::unpack(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = pack_database_object(*obj); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_MODIFY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, INDEX_TYPE) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        INDEX_TYPE key; \
        fc::raw::unpack(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = pack_database_object(*obj); \
        return 1; \
    }

#define HANDLE_DATABASE_OBJECT_MODIFY_BY_COMPOSITE_KEY(OBJECT_NAME, INDEX_POSITION, INDEX_NAME, ...) \
    if (tp == OBJECT_NAME##_object_type && index_position == INDEX_POSITION) { \
        std::tuple<__VA_ARGS__> key; \
        unpack_tuple(stream, key); \
        const auto* obj = db.find<OBJECT_NAME##_object, INDEX_NAME>(key); \
        if (!obj) { \
            return 0; \
        } \
        find_buffer = pack_database_object(*obj); \
        return 1; \
    }

#define HANDLE_CONTRACT_TABLE_OBJECT_MODIFY(OBJECT_NAME, SECONDARY_KEY_TYPE) \
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
        FC_ASSERT(0, "HANDLE_CONTRACT_TABLE_OBJECT_MODIFY: invalid index position"); \
    }

int32_t database_proxy::modify(void *_db, int32_t tp, int32_t index_position, char *raw_key, size_t raw_key_size, char *raw_data, size_t raw_data_size) {
    fc::datastream<const char*> stream(raw_key, raw_key_size);
    auto& db = *static_cast<chainbase::database *>(_db);

    if (tp == generated_transaction_object_type && index_position == 0) {
        int64_t _id;
        fc::raw::unpack(stream, _id);
        auto key = generated_transaction_object::id_type(_id);
        const auto* obj = db.find<generated_transaction_object, by_id>(key);
        if (!obj) {
            return 0;
        }

        auto __obj = unpack_database_object<generated_transaction_>(raw_data, raw_data_size); \

        db.modify<generated_transaction_object>( *obj, [&]( auto& _obj ) {
            _obj.trx_id = __obj.trx_id;
        } );

        find_buffer = pack_database_object(*obj);
        return 1;
    }
    return -1;
}
