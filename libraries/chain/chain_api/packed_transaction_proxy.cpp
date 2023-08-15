#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/block.hpp>

#include "packed_transaction_proxy.hpp"

using namespace std;
using namespace eosio::chain;

class packed_transaction_impl {
public:
    packed_transaction_impl(packed_transaction_ptr *sbp, bool attach) {
        if (attach)
            _sbp = sbp;
        else
            _sbp = new packed_transaction_ptr(*sbp);
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

private:
    packed_transaction_ptr *_sbp;
};

packed_transaction_proxy::packed_transaction_proxy(packed_transaction_ptr *bsp, bool attach): impl(std::make_shared<packed_transaction_impl>(bsp, attach)) 
{

}

packed_transaction_proxy::~packed_transaction_proxy() {

}

signed_transaction_ptr *packed_transaction_proxy::get_signed_transaction() {
    return impl->get_signed_transaction();
}

vector<char> packed_transaction_proxy::pack() {
    return impl->pack();
}
