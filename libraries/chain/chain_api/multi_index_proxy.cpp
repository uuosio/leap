#include <variant>

#include "multi_index_proxy.hpp"

#include <exception>

struct secondary_double {
    uint64_t first;
    double second;
};

struct first_sort_by_second {
    bool operator()(const secondary_double& lhs, const secondary_double& rhs) const {
        if (lhs.second < rhs.second) {
            return true;
        } else if (lhs.second > rhs.second) {
            return false;
        } else {
            return lhs.first < rhs.first;
        }
    }
};

struct first_sort_by_first {
    bool operator()(const secondary_double& lhs, const secondary_double& rhs) const {
        if (lhs.first > rhs.first) {
            return false;
        } else if (lhs.first < rhs.first) {
            return true;
        } else {
            return lhs.second < rhs.second;
        }
    }
};

using u64_double_index = std::variant<std::set<secondary_double, first_sort_by_first>, std::set<secondary_double, first_sort_by_second>>;

class u64_double_index_impl_base {
public:
    u64_double_index_impl_base() {}
    virtual ~u64_double_index_impl_base() {}
    virtual bool create(uint64_t key, double value) { return false; }
    virtual bool modify(uint64_t key, double old_value, double new_value) { return false; }
    virtual bool remove(uint64_t key, double old_value) { return false; }
    virtual bool find(uint64_t first, double second) { return false; }
    virtual bool lower_bound(uint64_t& first, double& second) { return false; }
    virtual bool upper_bound(uint64_t& first, double& second) { return false; }
    virtual bool first(uint64_t& first, double& second) { return false; }
    virtual bool last(uint64_t& first, double& second) { return false; }
    virtual bool pop_first(uint64_t& first, double& second) { return false; }
    virtual bool pop_last(uint64_t& first, double& second) { return false;}
    virtual uint64_t row_count() { return 0;}
};

template <typename T>
class u64_double_index_impl: public u64_double_index_impl_base {
public:
    u64_double_index_impl() {
        idx = T();
    }

    ~u64_double_index_impl() {}

    bool create(uint64_t key, double value) {
        return idx.insert({key, value}).second;
    }

    bool modify(uint64_t key, double old_value, double new_value) {
        auto itr = idx.find({key, old_value});
        if (itr == idx.end()) {
            return false;
        }
        idx.erase(itr);
        return idx.insert({key, new_value}).second;
    }

    bool remove(uint64_t key, double old_value) {
        return idx.erase({key, old_value}) == 1;
    }

    bool find(uint64_t first, double second) {
        auto itr = idx.lower_bound({first, second});
        if (itr == idx.end()) {
            return false;
        }
        if (itr->second != second) {
            return false;
        }
        return true;
    }

    bool lower_bound(uint64_t& first, double& second) {
        auto itr = idx.lower_bound({first, second});
        if (itr == idx.end()) {
            return false;
        }
        first = itr->first;
        second = itr->second;
        return true;
    }

    bool upper_bound(uint64_t& first, double& second) {
        auto itr = idx.upper_bound({first, second});
        if (itr == idx.end()) {
            return false;
        }
        first = itr->first;
        second = itr->second;
        return true;
    }

    bool first(uint64_t& first, double& second) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.begin();
        second = itr->second;
        first = itr->first;
        return true;
    }

    bool last(uint64_t& first, double& second) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.end();
        --itr;
        second = itr->second;
        first = itr->first;
        return true;
    }

    bool pop_first(uint64_t& first, double& second) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.begin();
        second = itr->second;
        first = itr->first;
        idx.erase(itr);
        return true;
    }

    bool pop_last(uint64_t& first, double& second) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.end();
        --itr;
        second = itr->second;
        first = itr->first;
        idx.erase(itr);
        return true;
    }

    uint64_t row_count() {
        return idx.size();
    }
private:
    T idx;
};

u64_double_index_proxy::u64_double_index_proxy(int sort_type) {
    if (sort_type == 0) {
        impl = std::make_unique<u64_double_index_impl<std::set<secondary_double, first_sort_by_first>>>();
    } else if (sort_type == 1) {
        impl = std::make_unique<u64_double_index_impl<std::set<secondary_double, first_sort_by_second>>>();
    } else {
        throw std::runtime_error("invalid sort type");
    }
}

u64_double_index_proxy::~u64_double_index_proxy() {
}

bool u64_double_index_proxy::create(uint64_t key, double value) {
    return impl->create(key, value);
}

bool u64_double_index_proxy::modify(uint64_t key, double old_value, double new_value) {
    return impl->modify(key, old_value, new_value);
}

bool u64_double_index_proxy::remove(uint64_t key, double old_value) {
    return impl->remove(key, old_value);
}

bool u64_double_index_proxy::find(uint64_t first, double second) {
    return impl->find(first, second);
}

bool u64_double_index_proxy::lower_bound(uint64_t& first, double& second) {
    return impl->lower_bound(first, second);
}

bool u64_double_index_proxy::upper_bound(uint64_t& first, double& second) {
    return impl->upper_bound(first, second);
}

bool u64_double_index_proxy::first(uint64_t& first, double& second) {
    return impl->first(first, second);
}

bool u64_double_index_proxy::last(uint64_t& first, double& second) {
    return impl->last(first, second);
}

bool u64_double_index_proxy::pop_first(uint64_t& first, double& second) {
    return impl->pop_first(first, second);
}

bool u64_double_index_proxy::pop_last(uint64_t& first, double& second) {
    return impl->pop_last(first, second);
}

uint64_t u64_double_index_proxy::row_count() {
    return impl->row_count();
}
