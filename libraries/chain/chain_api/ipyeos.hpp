#pragma once

#include "ipyeos_proxy.hpp"

extern "C" {
    void ipyeos_init_proxy(eos_cb* cb);
    void ipyeos_init_chain(eos_cb* cb);

    ipyeos_proxy *get_ipyeos_proxy();
    ipyeos_proxy *get_ipyeos_proxy_ex();

    apply_context_proxy *get_apply_context_proxy();
    vm_api_proxy *get_vm_api_proxy();

}

bool is_worker_process();
bool set_chain_config(void *id, const string& config);
string get_debug_producer_key(void *id);
