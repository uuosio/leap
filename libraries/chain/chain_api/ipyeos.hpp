#pragma once

#include "ipyeos_proxy.hpp"

extern "C" {
    void ipyeos_init_proxy(eos_cb* cb);
    void ipyeos_init_chain(eos_cb* cb);
    void init_new_chain_api();

    ipyeos_proxy *get_ipyeos_proxy();
    ipyeos_proxy *get_ipyeos_proxy_ex();

    database_proxy *get_database_proxy();

    apply_context_proxy *get_apply_context_proxy();
    vm_api_proxy *get_vm_api_proxy();
}
