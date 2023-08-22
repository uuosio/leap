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

class packed_transaction_proxy;

class signed_block_proxy {
public:
    signed_block_proxy(const signed_block_ptr& sbp);
    signed_block_proxy(signed_block_ptr *sbp);

    virtual ~signed_block_proxy();

    virtual uint32_t block_num();
    virtual vector<char> pack();
    virtual size_t transactions_size();

    virtual vector<char> get_transaction_id(int index);
    virtual bool is_packed_transaction(int index);
    virtual packed_transaction_proxy *get_packed_transaction(int index);
    virtual string to_json();

    signed_block_ptr get();

private:
    std::shared_ptr<signed_block_impl> impl;
};

