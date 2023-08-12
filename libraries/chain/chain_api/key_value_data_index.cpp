#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <fc/reflect/reflect.hpp>
#include <fc/io/raw.hpp>
#include <eosio/chain/multi_index_includes.hpp>

#include "key_value_data_index.hpp"

using namespace std;
using namespace boost;

using namespace fc;

struct VectorKeyCompare {
    bool operator()(const std::vector<uint64_t>& lhs, const std::vector<uint64_t>& rhs) const {
        for (size_t i=0; i<lhs.size(); i++) {
            if (lhs[i] < rhs[i]) {
                return true;
            } else if (lhs[i] > rhs[i]) {
                return false;
            }
        }
        return false;
    }
};

struct by_key {};
struct by_non_unique_key {};

using key_value_data_index = multi_index_container<
    key_value_data,
    indexed_by<
        ordered_unique<tag<by_key>, member<key_value_data, std::vector<uint64_t>, &key_value_data::key>, VectorKeyCompare>,
        ordered_non_unique<tag<by_non_unique_key>, member<key_value_data, std::vector<uint64_t>, &key_value_data::non_unique_key>, VectorKeyCompare>
    >
>;

FC_REFLECT(key_value_data, (key)(non_unique_key)(value))

class key_value_index_impl {
public:
    key_value_index_impl() {

    }

    ~key_value_index_impl() {

    }

    bool create(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value) {
        idx.emplace(key_value_data{key, vector<uint64_t>{}, value});
        return false;
    }

    bool modify(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value) {
        return false;
    }

    bool remove(int key_index, const vector<uint64_t>& key) {
        if (key_index == 0) {
            auto& idx2 = idx.get<by_key>();
            auto it = idx2.find(key);
            if (it != idx2.end()) {
                idx2.erase(it);
                return true;
            }
            return false;
        }

        if (key_index == 1) {
            auto& idx2 = idx.get<by_non_unique_key>();
            auto it = idx2.find(key);
            if (it != idx2.end()) {
                idx2.erase(it);
                return true;
            }
            return false;
        }

        return false;
    }

    bool find_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
        if (key_index == 0) {
            auto& idx2 = idx.get<by_key>();
            auto it = idx2.find(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }

        if (key_index == 1) {
            auto& idx2 = idx.get<by_non_unique_key>();
            auto it = idx2.find(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }
        return true;
    }

    bool lower_bound_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
        if (key_index == 0) {
            auto& idx2 = idx.get<by_key>();
            auto it = idx2.lower_bound(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }

        if (key_index == 1) {
            auto& idx2 = idx.get<by_non_unique_key>();
            auto it = idx2.lower_bound(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }
        return false;
    }

    bool upper_boundby_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
        if (key_index == 0) {
            auto& idx2 = idx.get<by_key>();
            auto it = idx2.upper_bound(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }

        if (key_index == 1) {
            auto& idx2 = idx.get<by_non_unique_key>();
            auto it = idx2.upper_bound(key);
            if (it != idx2.end()) {
                result = fc::raw::pack(*it);
                return true;
            }
            return false;
        }
        return false;
    }
    
    uint64_t row_count() {
        return idx.size();
    }

    private:
        key_value_data_index idx;
};


key_value_index_proxy::key_value_index_proxy(): impl(std::make_unique<key_value_index_impl>()) {

}

key_value_index_proxy::~key_value_index_proxy() {
}

bool key_value_index_proxy::create(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value) {
    return impl->create(key, non_unique_key, value);
}

bool key_value_index_proxy::modify(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value) {
    return impl->modify(key, non_unique_key, value);
}

bool key_value_index_proxy::remove(int key_index, const vector<uint64_t>& key) {
    return impl->remove(key_index, key);
}

bool key_value_index_proxy::find_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
    return impl->find_by_key(key_index, key, result);
}

bool key_value_index_proxy::lower_bound_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
    return impl->lower_bound_by_key(key_index, key, result);
}

bool key_value_index_proxy::upper_boundby_key(int key_index, vector<uint64_t>& key, std::vector<char>& result) {
    return impl->upper_boundby_key(key_index, key, result);
}

uint64_t key_value_index_proxy::row_count() {
    return impl->row_count();
}
