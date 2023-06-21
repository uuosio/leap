#include <fc/io/json.hpp>
#include <eosio/chain/block_log.hpp>

#include "block_log_proxy.hpp"

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
    auto block = _block_log->read_block_by_num(block_num);
    return fc::json::to_string(*block, fc::time_point::maximum());
}

string block_log_proxy::read_block_header_by_num(uint32_t block_num) {
    auto block_header = _block_log->read_block_header_by_num(block_num);
    return fc::json::to_string(block_header, fc::time_point::maximum());
}

string block_log_proxy::read_block_id_by_num(uint32_t block_num) {
    auto block_id = _block_log->read_block_id_by_num(block_num);
    return fc::json::to_string(block_id, fc::time_point::maximum());
}

string block_log_proxy::read_block_by_id(const string& id) {
    auto block = _block_log->read_block_by_id(fc::variant(id).as<block_id_type>());
    return fc::json::to_string(block, fc::time_point::maximum());
}

string block_log_proxy::head() {
    auto block = _block_log->head();
    return fc::json::to_string(block, fc::time_point::maximum());
}

string block_log_proxy::head_id() {
    auto block_id = _block_log->head_id();
    return block_id.str();
}

uint32_t block_log_proxy::first_block_num() {
    return _block_log->first_block_num();
}
