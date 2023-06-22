#include <fc/io/json.hpp>
#include <eosio/chain/block_log.hpp>

#include "block_log_proxy.hpp"
#include "chain_macro.hpp"

using namespace eosio::chain;

block_log_proxy::block_log_proxy(const string& data_dir) {
    _block_log = std::make_unique<eosio::chain::block_log>(data_dir);
}

block_log_proxy::~block_log_proxy() {

}

void *block_log_proxy::get_block_log_ptr() {
    return _block_log.get();
}

string block_log_proxy::read_block_by_num(uint32_t block_num) {
    try {
        auto block = _block_log->read_block_by_num(block_num);
        if (!block) {
            return "";
        }
        return fc::json::to_string(*block, fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string block_log_proxy::read_block_header_by_num(uint32_t block_num) {
    try {
        auto block_header = _block_log->read_block_header_by_num(block_num);
        if (!block_header) {
            return "";
        }
        return fc::json::to_string(block_header, fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string block_log_proxy::read_block_id_by_num(uint32_t block_num) {
    try {
        auto block_id = _block_log->read_block_id_by_num(block_num);
        return block_id.str();
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string block_log_proxy::read_block_by_id(const string& id) {
    try {
        auto block = _block_log->read_block_by_id(fc::variant(id).as<block_id_type>());
        if (!block) {
            return "";
        }
        return fc::json::to_string(block, fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string block_log_proxy::head() {
    try {
        auto block = _block_log->head();
        if (!block) {
            return "";
        }
        return fc::json::to_string(block, fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

uint32_t block_log_proxy::head_block_num() {
    try {
        auto block_num = _block_log->head()->block_num();
        return block_num;
    } CATCH_AND_LOG_EXCEPTION();
    return 0;
}

string block_log_proxy::head_id() {
    try {
        auto block_id = _block_log->head_id();
        return block_id.str();
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

uint32_t block_log_proxy::first_block_num() {
    return _block_log->first_block_num();
}
