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
    }
}

class block_state_proxy {
public:
    block_state_proxy(const eosio::chain::block_state_ptr& bsp);
    // block_state_proxy(const block_state_proxy& other);

    virtual ~block_state_proxy();

    uint32_t get_block_num();

private:
    std::shared_ptr<block_state_impl> impl;
};

