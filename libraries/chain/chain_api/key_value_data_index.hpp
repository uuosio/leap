#include <eosio/chain/types.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/thread_utils.hpp>
#include <eosio/chain/contract_types.hpp>

#include <fc/bitutil.hpp>
#include <fc/network/message_buffer.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>
#include <fc/time.hpp>
#include <fc/mutex.hpp>
#include <fc/network/listener.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>

#include <atomic>
#include <cmath>
#include <memory>
#include <new>

using namespace std;
using namespace boost;
using namespace eosio::chain;


struct key_value_data {
    uint64_t                first_key;
    vector<uint64_t>        other_key;
    vector<char>            value;
};

struct key_value_less {
    bool operator()( const vector<uint64_t>& lhs, const vector<uint64_t>& rhs ) const {
        for (int i=0; i<lhs.size(); i++) {
            if (lhs[i] < rhs[i]) {
                return true;
            } else if (lhs[i] > rhs[i]) {
                return false;
            }
        }
        return false;
    }
};

struct by_first_key;
struct by_key;

typedef multi_index_container<
    key_value_data,
    indexed_by<
        ordered_unique<
        tag<by_key>,
        composite_key< key_value_data,
            member<key_value_data, uint64_t, &key_value_data::first_key>,
            member<key_value_data, vector<uint64_t>, &key_value_data::other_key>
        >,
        composite_key_compare< std::less<>, key_value_less >
        >,
        ordered_non_unique<
            tag< by_first_key >,
            member< key_value_data, uint64_t, &key_value_data::first_key > >
        >
    > key_value_data_index;

FC_REFLECT(key_value_data, (first_key)(other_key)(value))
