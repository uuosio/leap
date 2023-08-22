#include <fc/io/json.hpp>
#include <eosio/chain/block_log.hpp>

#include "signed_block_proxy.hpp"
#include "block_log_proxy.hpp"
#include "chain_macro.hpp"

block_log_proxy::block_log_proxy(const string& data_dir) {
    _block_log = std::make_unique<eosio::chain::block_log>(data_dir);
}

block_log_proxy::~block_log_proxy() {

}

void *block_log_proxy::get_block_log_ptr() {
    return _block_log.get();
}

signed_block_proxy *block_log_proxy::read_block_by_num(uint32_t block_num) {
    try {
        auto block = _block_log->read_block_by_num(block_num);
        if (!block) {
            return nullptr;
        }
        auto ret = new signed_block_proxy(block);
        return ret;
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
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

signed_block_proxy *block_log_proxy::read_block_by_id(const string& id) {
    try {
        auto block = _block_log->read_block_by_id(fc::variant(id).as<block_id_type>());
        if (!block) {
            return nullptr;
        }
        return new signed_block_proxy(block);
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

signed_block_proxy *block_log_proxy::head() {
    try {
        auto block = _block_log->head();
        if (!block) {
            return nullptr;
        }
        return new signed_block_proxy(block);
    } CATCH_AND_LOG_EXCEPTION();
    return nullptr;
}

uint32_t block_log_proxy::head_block_num() {
    try {
        auto _head = _block_log->head();
        if (!_head) {
            return 0;
        }
        auto block_num = _head->block_num();
        return block_num;
    } CATCH_AND_LOG_EXCEPTION();
    return 0;
}

string block_log_proxy::head_id() {
    try {
        auto block_id = _block_log->head_id();
        if (!block_id) {
            return "";
        }
        return block_id->str();
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

uint32_t block_log_proxy::first_block_num() {
    return _block_log->first_block_num();
}
