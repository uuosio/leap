#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/block.hpp>
#include <eosio/chain/transaction.hpp>

#include <fc/io/raw.hpp>
#include <fc/io/json.hpp>
#include <fc/variant.hpp>

#include "packed_transaction_proxy.hpp"

using namespace std;
using namespace fc;
using namespace eosio::chain;

class packed_transaction_impl {
public:
    packed_transaction_impl(packed_transaction_ptr *sbp, bool attach) {
        if (attach)
            _sbp = sbp;
        else
            _sbp = new packed_transaction_ptr(*sbp);
    }

    packed_transaction_impl(const char *packed_tx, size_t packed_tx_size) {
        auto trx = fc::raw::unpack<packed_transaction>(packed_tx, packed_tx_size);
        _sbp = new packed_transaction_ptr(std::make_shared<packed_transaction>(std::move(trx)));
    }

    ~packed_transaction_impl() {
        delete _sbp;
    }

    signed_transaction_ptr *get_signed_transaction() {
        auto tx = (*_sbp)->get_signed_transaction();
        return new signed_transaction_ptr(std::make_shared<signed_transaction>(tx));
    }

    vector<char> pack() {
        return fc::raw::pack(**_sbp);
    }

    string to_json() {
        return fc::json::to_string(**_sbp, fc::time_point::maximum());
    }

private:
    packed_transaction_ptr *_sbp;
};

packed_transaction_proxy::packed_transaction_proxy(packed_transaction_ptr *bsp, bool attach): impl(std::make_shared<packed_transaction_impl>(bsp, attach)) 
{

}

packed_transaction_proxy::packed_transaction_proxy(const char *packed_tx, size_t packed_tx_size): impl(std::make_shared<packed_transaction_impl>(packed_tx, packed_tx_size)) {
}

packed_transaction_proxy::~packed_transaction_proxy() {

}

signed_transaction_ptr *packed_transaction_proxy::get_signed_transaction() {
    return impl->get_signed_transaction();
}

vector<char> packed_transaction_proxy::pack() {
    return impl->pack();
}

string packed_transaction_proxy::to_json() {
    return impl->to_json();
}
