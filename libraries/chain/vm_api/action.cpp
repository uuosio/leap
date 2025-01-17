#include <eosio/chain/webassembly/interface.hpp>
#include <eosio/vm/span.hpp>
#include "vm_api_proxy.hpp"

uint32_t vm_api_proxy::read_action_data(void *msg, uint32_t len) {
    legacy_span<char> s2(msg, len);
    return _interface->read_action_data(std::move(s2));
}

uint32_t vm_api_proxy::action_data_size() {
    return _interface->action_data_size();
}

uint64_t vm_api_proxy::current_receiver() {
   return _interface->current_receiver().to_uint64_t();
}
