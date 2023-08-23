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
#include "signed_transaction_proxy.hpp"

using namespace std;
using namespace fc;
using namespace eosio::chain;

class packed_transaction_impl {
public:
    packed_transaction_impl(packed_transaction_ptr *ptpp) {
        _ptpp = ptpp;
        _ptp = _ptpp->get();
    }

    packed_transaction_impl(packed_transaction_ptr& ptp) {
        _ptpp = new packed_transaction_ptr(ptp);
        _ptp = _ptpp->get();
    }

    packed_transaction_impl(signed_transaction_proxy *stp, bool compressed) {
        if (compressed) {
            _ptpp = new packed_transaction_ptr(std::make_shared<packed_transaction>(*stp->get_transaction(), packed_transaction::compression_type::zlib));
        } else {
            _ptpp = new packed_transaction_ptr(std::make_shared<packed_transaction>(*stp->get_transaction(), packed_transaction::compression_type::none));
        }
        _ptp = _ptpp->get();
    }

    packed_transaction_impl(signed_block_ptr& sbp, int index) {
        _sbp = sbp;
        auto& t = sbp->transactions[index];
        if (std::holds_alternative<packed_transaction>(t.trx)) {
            _ptp = &std::get<packed_transaction>(t.trx);
        } else {
            FC_ASSERT(false, "transaction is not packed_transaction");
        }
        _ptpp = nullptr;
    }

    packed_transaction_impl(const char *packed_tx, size_t packed_tx_size) {
        auto trx = fc::raw::unpack<packed_transaction>(packed_tx, packed_tx_size);
        _ptpp = new packed_transaction_ptr(std::make_shared<packed_transaction>(std::move(trx)));
    }

    ~packed_transaction_impl() {
        if (_ptpp) {
            delete _ptpp;
        }
    }

    string first_authorizer() {
        return _ptp->get_signed_transaction().first_authorizer().to_string();
    }

    signed_transaction_proxy *get_signed_transaction() {
        auto tx = _ptp->get_signed_transaction();
        auto stp = std::make_shared<signed_transaction>(tx);
        return new signed_transaction_proxy(stp);
    }

    vector<char> pack() {
        return fc::raw::pack(*_ptp);
    }

    string to_json() {
        return fc::json::to_string(*_ptp, fc::time_point::maximum());
    }

private:
    packed_transaction_ptr *_ptpp;
    const packed_transaction *_ptp;
    signed_block_ptr _sbp;
};

packed_transaction_proxy::packed_transaction_proxy(packed_transaction_ptr *bsp, bool attach) {
    if (attach) {
        impl = std::make_shared<packed_transaction_impl>(bsp);
    } else {
        impl = std::make_shared<packed_transaction_impl>(*bsp);
    }
}

packed_transaction_proxy::packed_transaction_proxy(signed_block_ptr& bsp, int index) {
    impl = std::make_shared<packed_transaction_impl>(bsp, index);
}

packed_transaction_proxy::packed_transaction_proxy(signed_transaction_proxy *stp, bool compressed) {
    impl = std::make_shared<packed_transaction_impl>(stp, compressed);
}

packed_transaction_proxy::packed_transaction_proxy(const char *packed_tx, size_t packed_tx_size): impl(std::make_shared<packed_transaction_impl>(packed_tx, packed_tx_size)) {
}

packed_transaction_proxy::~packed_transaction_proxy() {

}

string packed_transaction_proxy::first_authorizer() {
    return impl->first_authorizer();
}

signed_transaction_proxy *packed_transaction_proxy::get_signed_transaction() {
    return impl->get_signed_transaction();
}

vector<char> packed_transaction_proxy::pack() {
    return impl->pack();
}

string packed_transaction_proxy::to_json() {
    return impl->to_json();
}
