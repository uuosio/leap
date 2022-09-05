#pragma once

#include "ipyeos_proxy.hpp"
extern "C" void ipyeos_init_proxy(fn_eos_init init, fn_eos_exec exec);
extern "C" void ipyeos_init_chain(fn_eos_init init, fn_eos_exec exec);
extern "C" void init_new_chain_api();
extern "C" ipyeos_proxy *get_ipyeos_proxy();
extern "C" apply_context_proxy *get_apply_context_proxy();
extern "C" vm_api_proxy *get_vm_api_proxy();
