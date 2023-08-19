#include <stdint.h>
#include <set>
#include <vector>
#include <memory>

#include <eosio/chain/block.hpp>

#include "signed_block_proxy.hpp"
#include "packed_transaction_proxy.hpp"

using namespace std;
using namespace eosio::chain;

class signed_block_impl {
public:
    signed_block_impl(const signed_block_ptr& sbp) {
        _sbp = new signed_block_ptr(sbp);
    }

    signed_block_impl(signed_block_ptr *sbp) {
        FC_ASSERT(sbp != nullptr, "signed_block_ptr *sbp must not be null");
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

    bool is_packed_transaction(int index) {
        if (index < 0 || index >= (*_sbp)->transactions.size()) {
            return false;
        }
        auto &t = (*_sbp)->transactions[index];
        if (std::holds_alternative<packed_transaction>(t.trx)) {
            return true;
        }
        return false;
    }

    vector<char> get_transaction_id(int index) {
        if (index < 0 || index >= (*_sbp)->transactions.size()) {
            return vector<char>();
        }
        transaction_id_type id;
        auto &t = (*_sbp)->transactions[index];
        if (std::holds_alternative<transaction_id_type>(t.trx)) {
            id = std::get<transaction_id_type>(t.trx);
        }
        else {
            id = std::get<packed_transaction>(t.trx).id();
        }
        return fc::raw::pack(id);
    }

    packed_transaction_proxy *get_packed_transaction(int index) {
        if (index < 0 || index >= (*_sbp)->transactions.size()) {
            return nullptr;
        }

        auto& t = (*_sbp)->transactions[index];
        if (std::holds_alternative<packed_transaction>(t.trx)) {
            return new packed_transaction_proxy(*_sbp, index);
        } else {
            return nullptr;
        }
        return nullptr;
    }

    signed_block_ptr get() {
        return *_sbp;
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

vector<char> signed_block_proxy::get_transaction_id(int index) {
    return impl->get_transaction_id(index);
}

bool signed_block_proxy::is_packed_transaction(int index) {
    return impl->is_packed_transaction(index);
}

packed_transaction_proxy *signed_block_proxy::get_packed_transaction(int index) {
    return impl->get_packed_transaction(index);
}

signed_block_ptr signed_block_proxy::get() {
    return impl->get();
}
