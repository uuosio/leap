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
        using signed_transaction_ptr = std::shared_ptr<signed_transaction>;
        using packed_transaction_ptr = std::shared_ptr<const packed_transaction>;
    }
}

using namespace eosio::chain;

class packed_transaction_proxy {
public:
    packed_transaction_proxy(packed_transaction_ptr *sbp, bool attach);
    packed_transaction_proxy(const char *packed_tx, size_t packed_tx_size);

    virtual ~packed_transaction_proxy();

    signed_transaction_ptr *get_signed_transaction();
    vector<char> pack();
    string to_json();

private:
    std::shared_ptr<packed_transaction_impl> impl;
};

