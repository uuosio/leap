#include <eosio/chain/controller.hpp>
#include "block_state_proxy.hpp"

class block_state_impl {
public:
    block_state_impl(const block_state_ptr& bsp) {
        _block_state = bsp;
    }

    // block_state_impl(const block_state_impl& other) {
    //     _block_state = other._block_state;
    // }

    uint32_t block_num() {
        return _block_state->block_num;
    }


    signed_block_ptr *block() {
        return &_block_state->block;
    }

private:
    block_state_ptr _block_state;
};

block_state_proxy::block_state_proxy(const block_state_ptr& bsp): impl(std::make_shared<block_state_impl>(bsp)) {

}

// block_state_proxy::block_state_proxy(const block_state_proxy& other) {
//     impl = other.impl;
// }

block_state_proxy::~block_state_proxy() {

}

uint32_t block_state_proxy::block_num() {
    return impl->block_num();
}

signed_block_ptr *block_state_proxy::block() {
    return impl->block();
}
