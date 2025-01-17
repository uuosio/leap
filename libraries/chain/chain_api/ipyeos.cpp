#include <dlfcn.h>

#include "ipyeos.hpp"
#include "apply_context_proxy.hpp"
#include "../vm_api/vm_api_proxy.hpp"
#include <stacktrace.h>

static ipyeos_proxy *s_proxy = nullptr;

extern "C" {

void ipyeos_init_proxy(eos_cb* cb) {
    if (s_proxy) {
        return;
    }
    s_proxy = new ipyeos_proxy(cb);
}

void ipyeos_init_chain(eos_cb* cb) {
    ipyeos_init_proxy(cb);

    const char *chain_api_lib = getenv("CHAIN_API_LIB");
    if (!chain_api_lib) {
        printf("++++CHAIN_API_LIB environment variable not set!\n");
        exit(-1);
    }

    void *handle = dlopen(chain_api_lib, RTLD_LAZY | RTLD_GLOBAL);
    if (handle == 0) {
        printf("Failed to load %s! error: %s\n", chain_api_lib, dlerror());
        exit(-1);
        return;
    }

    fn_init_ipyeos_proxy init_ipyeos_proxy = (fn_init_ipyeos_proxy)dlsym(handle, "init_ipyeos_proxy");
    if (!init_ipyeos_proxy) {
        printf("Failed to load init_ipyeos_proxy! error: %s\n", dlerror());
        exit(-1);
        return;
    }
    init_ipyeos_proxy(s_proxy);

    const char *vm_api_lib = getenv("VM_API_LIB");
    if (!vm_api_lib) {
        printf("++++VM_API_LIB environment variable not set!\n");
        exit(-1);
    }

    handle = dlopen(vm_api_lib, RTLD_LAZY | RTLD_GLOBAL);
    if (handle == 0) {
        printf("Failed to load %s! error: %s\n", vm_api_lib, dlerror());
        exit(-1);
        return;
    }

    fn_init_vm_api init_vm_api = (fn_init_vm_api)dlsym(handle, "init_vm_api");
    if (!init_vm_api) {
        printf("Failed to load init_vm_api! error: %s\n", dlerror());
        exit(-1);
        return;
    }

    init_vm_api(s_proxy->get_apply_context_proxy()->get_vm_api_proxy());
}

ipyeos_proxy *get_ipyeos_proxy() {
    if (s_proxy == nullptr) {
        print_stacktrace();
        printf("ipyeos_proxy ptr is null\n");
        exit(-1);
    }
    return s_proxy;
}

ipyeos_proxy *get_ipyeos_proxy_ex() {
    return s_proxy;
}

apply_context_proxy *get_apply_context_proxy() {
    return get_ipyeos_proxy()->get_apply_context_proxy();
}

vm_api_proxy *get_vm_api_proxy() {
    return get_apply_context_proxy()->get_vm_api_proxy();
}

}

bool is_worker_process() {
    auto *proxy = get_ipyeos_proxy_ex();
    if (!proxy) {
        return false;
    }
    return proxy->is_worker_process();
}

bool set_chain_config(void *id, const string& config) {
    auto *proxy = get_ipyeos_proxy_ex();
    if (!proxy) {
        return false;
    }
    return proxy->set_chain_config(id, config);
}

string get_debug_producer_key(void *id) {
    auto *proxy = get_ipyeos_proxy_ex();
    if (!proxy) {
        return "";
    }
    return proxy->get_debug_producer_key(id);
}
