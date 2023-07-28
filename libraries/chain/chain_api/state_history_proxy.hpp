#pragma once
#include <memory>

using namespace std;

struct state_history_proxy_impl;

class state_history_proxy {

public:
    state_history_proxy();
    ~state_history_proxy() = default;

    virtual bool initialize(void *controller,
        const string& data_dir,
        const string& state_history_dir, //state_history
        const string& state_history_retained_dir, //empty string
        const string& state_history_archive_dir, //empty string
        uint32_t state_history_stride, //0
        uint32_t max_retained_history_files, //0
        bool delete_state_history, //false
        bool trace_history, //false
        bool chain_state_history, //false
        const string& state_history_endpoint, //127.0.0.1:8080
        const string& state_history_unix_socket_path,
        bool trace_history_debug_mode,
        uint32_t state_history_log_retain_blocks
    );

    virtual bool startup();
    virtual void shutdown();

private:
    std::shared_ptr<state_history_proxy_impl> my;
};

