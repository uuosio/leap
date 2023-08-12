#include "multi_index_proxy.hpp"

#include <exception>

struct secondary_double {
    uint64_t primary;
    double secondary;
};

struct first_sort_by_secondary {
    bool operator()(const secondary_double& lhs, const secondary_double& rhs) const {
        if (lhs.secondary < rhs.secondary) {
            return true;
        } else if (lhs.secondary > rhs.secondary) {
            return false;
        } else {
            return lhs.primary < rhs.primary;
        }
    }
};

struct first_sort_by_primary {
    bool operator()(const secondary_double& lhs, const secondary_double& rhs) const {
        if (lhs.primary > rhs.primary) {
            return false;
        } else if (lhs.primary < rhs.primary) {
            return true;
        } else {
            return lhs.secondary < rhs.secondary;
        }
    }
};

using u64_double_index = std::variant<std::set<secondary_double, first_sort_by_primary>, std::set<secondary_double, first_sort_by_secondary>>;

class u64_double_index_impl_base {
public:
    u64_double_index_impl_base() {}
    virtual ~u64_double_index_impl_base() {}
    virtual bool create(uint64_t key, double value) { return false; }
    virtual bool modify(uint64_t key, double old_value, double new_value) { return false; }
    virtual bool remove(uint64_t key, double old_value) { return false; }
    virtual bool find(double secondary, uint64_t& primary) { return false; }
    virtual bool lower_bound(double& secondary, uint64_t& primary) { return false; }
    virtual bool upper_bound(double& secondary, uint64_t& primary) { return false; }
    virtual bool first(double& secondary, uint64_t& primary) { return false; }
    virtual bool last(double& secondary, uint64_t& primary) { return false; }
    virtual bool pop_first(double& secondary, uint64_t& primary) { return false; }
    virtual bool pop_last(double& secondary, uint64_t& primary) { return false;}
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

    bool find(double secondary, uint64_t& primary) {
        auto itr = idx.lower_bound({primary, secondary});
        if (itr == idx.end()) {
            return false;
        }
        if (itr->secondary != secondary) {
            return false;
        }
        primary = itr->primary;
        return true;
    }

    bool lower_bound(double& secondary, uint64_t& primary) {
        auto itr = idx.lower_bound({primary, secondary});
        if (itr == idx.end()) {
            return false;
        }
        primary = itr->primary;
        secondary = itr->secondary;
        return true;
    }

    bool upper_bound(double& secondary, uint64_t& primary) {
        auto itr = idx.upper_bound({primary, secondary});
        if (itr == idx.end()) {
            return false;
        }
        primary = itr->primary;
        secondary = itr->secondary;
        return true;
    }

    bool first(double& secondary, uint64_t& primary) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.begin();
        secondary = itr->secondary;
        primary = itr->primary;
        return true;
    }

    bool last(double& secondary, uint64_t& primary) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.end();
        --itr;
        secondary = itr->secondary;
        primary = itr->primary;
        return true;
    }

    bool pop_first(double& secondary, uint64_t& primary) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.begin();
        secondary = itr->secondary;
        primary = itr->primary;
        idx.erase(itr);
        return true;
    }

    bool pop_last(double& secondary, uint64_t& primary) {
        if (idx.empty()) {
            return false;
        }
        auto itr = idx.end();
        --itr;
        secondary = itr->secondary;
        primary = itr->primary;
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
        impl = std::make_unique<u64_double_index_impl<std::set<secondary_double, first_sort_by_secondary>>>();
    } else if (sort_type == 1) {
        impl = std::make_unique<u64_double_index_impl<std::set<secondary_double, first_sort_by_primary>>>();
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

bool u64_double_index_proxy::find(double secondary, uint64_t& primary) {
    return impl->find(secondary, primary);
}

bool u64_double_index_proxy::lower_bound(double& secondary, uint64_t& primary) {
    return impl->lower_bound(secondary, primary);
}

bool u64_double_index_proxy::upper_bound(double& secondary, uint64_t& primary) {
    return impl->upper_bound(secondary, primary);
}

bool u64_double_index_proxy::first(double& secondary, uint64_t& primary) {
    return impl->first(secondary, primary);
}

bool u64_double_index_proxy::last(double& secondary, uint64_t& primary) {
    return impl->last(secondary, primary);
}

bool u64_double_index_proxy::pop_first(double& secondary, uint64_t& primary) {
    return impl->pop_first(secondary, primary);
}

bool u64_double_index_proxy::pop_last(double& secondary, uint64_t& primary) {
    return impl->pop_last(secondary, primary);
}

uint64_t u64_double_index_proxy::row_count() {
    return impl->row_count();
}
