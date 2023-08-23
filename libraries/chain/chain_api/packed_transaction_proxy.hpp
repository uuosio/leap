#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

using namespace std;

class packed_transaction_impl;
namespace eosio { 
    namespace chain {
        struct packed_transaction;
        struct signed_transaction;
        struct signed_block;
        using signed_transaction_ptr = std::shared_ptr<signed_transaction>;
        using packed_transaction_ptr = std::shared_ptr<const packed_transaction>;
        using signed_block_ptr = std::shared_ptr<signed_block>;
    }
}

using namespace eosio::chain;

class signed_transaction_proxy;

class packed_transaction_proxy {
public:
    packed_transaction_proxy(packed_transaction_ptr *sbp, bool attach);
    packed_transaction_proxy(signed_transaction_proxy *stp, bool compressed);
    packed_transaction_proxy(signed_block_ptr& bsp, int index);
    packed_transaction_proxy(const char *packed_tx, size_t packed_tx_size);

    virtual ~packed_transaction_proxy();

    virtual string first_authorizer();
    virtual signed_transaction_proxy *get_signed_transaction();
    virtual vector<char> pack();
    virtual string to_json();

private:
    std::shared_ptr<packed_transaction_impl> impl;
};

