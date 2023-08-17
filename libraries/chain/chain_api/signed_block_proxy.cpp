#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/block.hpp>

#include "signed_block_proxy.hpp"

using namespace std;
using namespace eosio::chain;

class signed_block_impl {
public:
    signed_block_impl(const signed_block_ptr& sbp) {
        _sbp = new signed_block_ptr(sbp);
    }

    signed_block_impl(signed_block_ptr *sbp) {
        _sbp = sbp;
    }

    ~signed_block_impl() {
        delete _sbp;
    }

    uint32_t block_num() {
        return (*_sbp)->block_num();
    }

    vector<char> pack() {
        return fc::raw::pack(**_sbp);
    }

    size_t transactions_size() {
        return (*_sbp)->transactions.size();
    }

    signed_block_ptr *get() {
        return _sbp;
    }

private:
    signed_block_ptr *_sbp;
};

signed_block_proxy::signed_block_proxy(const signed_block_ptr& bsp): impl(std::make_shared<signed_block_impl>(bsp)) 
{

}

signed_block_proxy::signed_block_proxy(signed_block_ptr *sbp): impl(std::make_shared<signed_block_impl>(sbp))
{

}


signed_block_proxy::~signed_block_proxy() {

}

uint32_t signed_block_proxy::block_num() {
    return impl->block_num();
}

vector<char> signed_block_proxy::pack() {
    return impl->pack();
}

size_t signed_block_proxy::transactions_size() {
    return impl->transactions_size();
}

signed_block_ptr *signed_block_proxy::get() {
    return impl->get();
}
