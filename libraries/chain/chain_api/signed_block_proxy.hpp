#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class signed_block_impl;
namespace eosio { 
    namespace chain {
        struct signed_block;
        using signed_block_ptr = std::shared_ptr<signed_block>;
    }
}

using namespace eosio::chain;

class signed_block_proxy {
public:
    signed_block_proxy(const signed_block_ptr& sbp);
    signed_block_proxy(signed_block_ptr *sbp);

    virtual ~signed_block_proxy();

    virtual uint32_t block_num();
    virtual vector<char> pack();

private:
    std::shared_ptr<signed_block_impl> impl;
};

