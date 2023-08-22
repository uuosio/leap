#include <eosio/chain/transaction.hpp>

#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/variant.hpp>

#include "signed_transaction_proxy.hpp"
#include "chain_macro.hpp"

using namespace fc;
using namespace eosio::chain;

signed_transaction_proxy::signed_transaction_proxy(
    uint32_t expiration,
    const char* ref_block_id,
    size_t ref_block_id_size,
    uint32_t max_net_usage_words, //fc::unsigned_int
    uint8_t  max_cpu_usage_ms,    //
    uint32_t delay_sec            //fc::unsigned_int
) {
    trx = std::make_shared<signed_transaction>();
    trx->expiration = fc::time_point_sec(expiration);
    trx->set_reference_block(fc::raw::unpack<block_id_type>(ref_block_id, ref_block_id_size));
    trx->max_net_usage_words = max_net_usage_words;
    trx->max_cpu_usage_ms = max_cpu_usage_ms;
    trx->delay_sec = delay_sec;
}

signed_transaction_proxy::signed_transaction_proxy(signed_transaction_ptr& transaction) {
    trx = transaction;
}

signed_transaction_proxy::~signed_transaction_proxy() {

}

void signed_transaction_proxy::id(vector<char>& result) {
    result = fc::raw::pack(trx->id());
}

void signed_transaction_proxy::add_action(uint64_t account, uint64_t name, const char *data, size_t size, vector<std::pair<uint64_t, uint64_t>>& auths) {
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

bool signed_transaction_proxy::sign(const char *private_key, size_t size, const char *chain_id, size_t chain_id_size) {
    try {
        auto _private_key = fc::raw::unpack<fc::crypto::private_key>(private_key, size);
        trx->sign(_private_key, chain_id_type(chain_id, chain_id_size));
        return true;
    }CATCH_AND_LOG_EXCEPTION()
    return false;
}

void signed_transaction_proxy::pack(bool compress, int pack_type, vector<char>& result) {
    FC_ASSERT(pack_type == 0 || pack_type == 1, "unknown pack type");

    if (pack_type == 0) { //signed_transaction
        result = fc::raw::pack(*trx);
        return;
    }

    packed_transaction::compression_type type;
    if (compress) {
        type = packed_transaction::compression_type::zlib;
    } else {
        type = packed_transaction::compression_type::none;
    }
    auto packed_trx = packed_transaction(*trx, type);
    result = fc::raw::pack(packed_trx);
}

bool signed_transaction_proxy::to_json(int result_type, bool compressed, string& result) {
    try {
        if (result_type == 0) {
            result = fc::json::to_string(*trx, fc::time_point::maximum());
        } else if (result_type == 1) {// packed_transaction
            if (compressed) {
                packed_transaction packed_trx(*trx, packed_transaction::compression_type::zlib);
                result = fc::json::to_string(*trx, fc::time_point::maximum());
            } else {
                packed_transaction packed_trx(*trx, packed_transaction::compression_type::none);
                result = fc::json::to_string(*trx, fc::time_point::maximum());
            }
        } else {
            FC_ASSERT(false, "unknown unpack result type");
        }
        return true;
    }CATCH_AND_LOG_EXCEPTION()
    return false;
}
