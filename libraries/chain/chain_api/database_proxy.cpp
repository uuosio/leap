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

class session_impl_proxy {
public:
    session_impl_proxy(chainbase::database::session&& session): session(std::move(session)) {
    }
    
    void squash() {
        session.squash();
    }
    
    void undo() {
        session.undo();
    }

    void push() {
        session.push();
    }

    chainbase::database::session session;
};


database_proxy::database_proxy(chainbase::database *db_ptr, bool attach):
attach(attach),
db_ptr(db_ptr),
db(*db_ptr)
{
}

database_proxy::~database_proxy() {
    if (!attach) {
        delete db_ptr;
    }
}

uint64_t database_proxy::get_free_memory() {
    return db.get_segment_manager()->get_free_memory();
}

uint64_t database_proxy::get_total_memory() {
    return db.get_segment_manager()->get_size();
}

int64_t database_proxy::revision() {
    return db.revision();
}

void database_proxy::set_revision(int64_t revision) {
    db.set_revision(revision);
}

void database_proxy::undo() {
    db.undo();
}

void database_proxy::undo_all() {
    db.undo_all();
}

void database_proxy::start_undo_session(bool enabled) {
    _session_impl_proxy = std::make_unique<session_impl_proxy>(db.start_undo_session(enabled));
}

void database_proxy::session_squash() {
    _session_impl_proxy->squash();
}

void database_proxy::session_undo() {
    _session_impl_proxy->undo();
}

void database_proxy::session_push() {
    _session_impl_proxy->push();
}

void database_proxy::set_data_handler(fn_data_handler handler, void *custom_data) {
    this->handler = handler;
    this->custom_data = custom_data;
}

#define DATABASE_OBJECT_ROW_COUNT_EX(OBJECT_NAME, OBJECT_INDEX) \
    if (tp == OBJECT_NAME##_object_type) { \
        return db.get_index<OBJECT_INDEX>().indices().size(); \
    }

#define DATABASE_OBJECT_ROW_COUNT(OBJECT_NAME) \
    DATABASE_OBJECT_ROW_COUNT_EX(OBJECT_NAME, OBJECT_NAME##_index)

uint64_t database_proxy::row_count(int32_t tp) {
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
