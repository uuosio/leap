#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class block_state_impl;
namespace eosio { 
    namespace chain {
        struct block_state;
        using block_state_ptr = std::shared_ptr<block_state>;
        struct signed_block;
        using signed_block_ptr = std::shared_ptr<signed_block>;
    }
}

using namespace std;
using namespace eosio::chain;

class block_state_proxy {
public:
    block_state_proxy(const block_state_ptr& bsp);
    // block_state_proxy(const block_state_proxy& other);

    virtual ~block_state_proxy();

    virtual uint32_t block_num();
    virtual signed_block_ptr *block();

private:
    std::shared_ptr<block_state_impl> impl;
};

