#pragma once

#include <stdint.h>
#include <vector>

namespace chainbase {
    class database;
}

typedef int32_t (*fn_data_handler)(int32_t tp, char *data, size_t size, void* custom_data);

class database_proxy {
public:
    database_proxy() {}
    virtual ~database_proxy(){}

    virtual void set_data_handler(fn_data_handler handler, void *custom_data);

    template<typename database_object, typename database_object2>
    int32_t create(chainbase::database& db, database_object2& obj);

    virtual int32_t create(void *_db, int32_t tp, const char *raw_data, size_t raw_data_size);
    virtual int32_t modify(void *_db, int32_t tp, int32_t index_position, char *raw_key, size_t raw_key_size, char *raw_data, size_t size);

    virtual int32_t walk(void *_db, int32_t tp, int32_t index_position);
    virtual int32_t walk_range(void *db, int32_t tp, int32_t index_position, char *raw_lower_bound, size_t raw_lower_bound_size, char *raw_upper_bound, size_t raw_upper_bound_size);
    
    int32_t find(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t raw_data_size);
    virtual int32_t find(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size);

    virtual int32_t lower_bound(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size);
    virtual int32_t upper_bound(void *_db, int32_t tp, int32_t index_position, char *raw_data, size_t size, char **out, size_t *out_size);
    virtual uint64_t row_count(void *_db, int32_t tp);
private:
    fn_data_handler handler = nullptr;
    void *custom_data = nullptr;
    std::vector<char> find_buffer;
};