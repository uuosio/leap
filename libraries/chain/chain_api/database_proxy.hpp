#pragma once

#include <stdint.h>

namespace chainbase {
    class database;
}

typedef int32_t (*fn_data_handler)(int32_t tp, int64_t id, char *data, size_t size, void* custom_data);

class database_proxy {
public:
    database_proxy(void *_db):db(_db) {}
    virtual ~database_proxy(){}

    virtual void set_database(void *db);
    virtual void *get_database();

    virtual void set_data_handler(fn_data_handler handler, void *custom_data);

    virtual int32_t walk(int32_t tp, int32_t index_position);
    virtual int32_t walk_range(int32_t tp, int32_t index_position, char *raw_lower_bound, size_t raw_lower_bound_size, char *raw_upper_bound, size_t raw_upper_bound_size);
    virtual int32_t find(int32_t tp, int32_t index_position, char *raw_data, size_t size, char *out, size_t out_size);

private:
    void *db = nullptr;
    fn_data_handler handler = nullptr;
    void *custom_data = nullptr;
};
