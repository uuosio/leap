#include <eosio/chain/transaction.hpp>

#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/variant.hpp>

#include "transaction_proxy.hpp"
#include "chain_macro.hpp"

using namespace fc;
using namespace eosio::chain;

transaction_proxy::transaction_proxy() {

}

transaction_proxy::~transaction_proxy() {

}

void *transaction_proxy::new_transaction(
    uint32_t expiration,
    const char* ref_block_id,
    size_t ref_block_id_size,
    uint32_t max_net_usage_words, //fc::unsigned_int
    uint8_t  max_cpu_usage_ms,    //
    uint32_t delay_sec            //fc::unsigned_int
) {
    signed_transaction *trx = new signed_transaction();
    trx->expiration = fc::time_point_sec(expiration);
    trx->set_reference_block(fc::raw::unpack<block_id_type>(ref_block_id, ref_block_id_size));
    trx->max_net_usage_words = max_net_usage_words;
    trx->max_cpu_usage_ms = max_cpu_usage_ms;
    trx->delay_sec = delay_sec;
    return trx;
}

void transaction_proxy::free(void *transaction) {
    delete static_cast<signed_transaction *>(transaction);
}

void transaction_proxy::add_action(void *transaction, uint64_t account, uint64_t name, const char *data, size_t size, vector<std::pair<uint64_t, uint64_t>>& auths) {
    signed_transaction *trx = static_cast<signed_transaction *>(transaction);
    vector<permission_level> auths_;
    for (auto auth : auths) {
        auths_.emplace_back(permission_level{eosio::chain::name(auth.first), eosio::chain::name(auth.second)});
    }
    trx->actions.emplace_back(action(
        auths_,
        eosio::chain::name(account),
        eosio::chain::name(name),
        std::vector<char>(data, data+size)
    ));
}

bool transaction_proxy::sign(void *transaction, const char *private_key, size_t size, const char *chain_id, size_t chain_id_size) {
    try {
        signed_transaction *trx = static_cast<signed_transaction *>(transaction);
        auto _private_key = fc::raw::unpack<fc::crypto::private_key>(private_key, size);
        trx->sign(_private_key, chain_id_type(chain_id, chain_id_size));
        return true;
    }CATCH_AND_LOG_EXCEPTION()
    return false;
}

void transaction_proxy::pack(void *transaction, bool compress, vector<char>& result) {
    signed_transaction *trx = static_cast<signed_transaction *>(transaction);
    packed_transaction::compression_type type;
    if (compress) {
        type = packed_transaction::compression_type::zlib;
    } else {
        type = packed_transaction::compression_type::none;
    }
    auto packed_trx = packed_transaction(*trx, type);
    result = fc::raw::pack(packed_trx);
}

bool transaction_proxy::unpack(const char *raw_tx, size_t size, int result_type, string& result) {
    try {
        auto trx = fc::raw::unpack<packed_transaction>(raw_tx, size);
        if (result_type == 0) { //packed_transaction
            result = fc::json::to_string(fc::variant(trx), fc::time_point::maximum());
        } else if (result_type == 1) {
            result = fc::json::to_string(fc::variant(trx.get_signed_transaction()), fc::time_point::maximum());
        } else {
            FC_ASSERT(false, "unknown unpack result type");
        }
        return true;
    }CATCH_AND_LOG_EXCEPTION()
    return false;
}
