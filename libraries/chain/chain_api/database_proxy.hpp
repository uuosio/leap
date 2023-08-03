#pragma once

#include <stdint.h>

#include <memory>
#include <vector>
#include <string>

using namespace std;

namespace chainbase {
    class database;
}

class session_impl;

typedef int32_t (*fn_data_handler)(int32_t tp, char *data, size_t size, void* custom_data);

class database_proxy {
public:
    database_proxy(chainbase::database *db_ptr, bool attach = true);
    virtual ~database_proxy();

    virtual uint64_t get_free_memory();
    virtual uint64_t get_total_memory();

    virtual int64_t revision();
    virtual void set_revision(int64_t revision);

    virtual void undo();
    virtual void undo_all();

    virtual void start_undo_session(bool enabled);
    virtual void session_squash();
    virtual void session_undo();
    virtual void session_push();

    virtual void set_data_handler(fn_data_handler handler, void *custom_data);

    template<typename database_object, typename database_object2>
    int32_t create(chainbase::database& db, database_object2& obj);

    virtual int32_t create(int32_t tp, const char *raw_data, size_t raw_data_size);
    virtual int32_t modify(int32_t tp, int32_t index_position, const char*raw_key, size_t raw_key_size, const char *raw_data, size_t size);

    virtual int32_t walk(int32_t tp, int32_t index_position);
    virtual int32_t walk_range(int32_t tp, int32_t index_position, const char *raw_lower_bound, size_t raw_lower_bound_size, const char *raw_upper_bound, size_t raw_upper_bound_size);
    
    virtual int32_t find(int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char>& out);

    virtual int32_t lower_bound(int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char>& out);
    virtual int32_t upper_bound(int32_t tp, int32_t index_position, const char *raw_data, size_t size, vector<char>& out);
    virtual uint64_t row_count(int32_t tp);

private:
    fn_data_handler handler = nullptr;
    void *custom_data = nullptr;
    bool attach = false;
    chainbase::database *db_ptr = nullptr;
    chainbase::database& db;
    std::unique_ptr<session_impl> _session_impl;
};
