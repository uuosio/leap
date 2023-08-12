#include <stdint.h>

#include <vector>
#include <memory>

using namespace std;

struct key_value_data {
    std::vector<uint64_t> key;
    std::vector<uint64_t> non_unique_key;
    std::vector<char> value;

    key_value_data(const std::vector<uint64_t>& k, const std::vector<uint64_t>& k2, const std::vector<char>& v) : key(k), non_unique_key(k2), value(v) {}
};

class key_value_index_impl;

class key_value_index_proxy {
    public:
        key_value_index_proxy();
        virtual ~key_value_index_proxy();

        key_value_index_proxy(const key_value_index_proxy& other) = delete;
        key_value_index_proxy(key_value_index_proxy&& other) = delete;
        key_value_index_proxy& operator=(const key_value_index_proxy& other) = delete;
        key_value_index_proxy& operator=(key_value_index_proxy&& other) = delete;

        virtual bool create(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value);
        virtual bool modify(const vector<uint64_t>& key, const vector<uint64_t>& non_unique_key, const vector<char>& value);
        virtual bool remove(int key_index, const vector<uint64_t>& key);

        virtual bool find_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result);
        virtual bool lower_bound_by_key(int key_index, vector<uint64_t>& key, std::vector<char>& result);
        virtual bool upper_boundby_key(int key_index, vector<uint64_t>& key, std::vector<char>& result);
        virtual uint64_t row_count();
    private:
        std::unique_ptr<key_value_index_impl> impl;
};
