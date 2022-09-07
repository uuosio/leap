#include <eosio/chain/webassembly/interface.hpp>
#include <eosio/vm/span.hpp>
#include "vm_api_proxy.hpp"

void vm_api_proxy::set_action_return_value(const char *data, uint32_t data_size) {
   span<const char> packed_blob(data, data_size);
    _interface->set_action_return_value(std::move(packed_blob));
}

uint32_t vm_api_proxy::get_code_hash(capi_name account, uint32_t struct_version, char* packed_result, uint32_t packed_result_len) {
   span<char> _packed_result(packed_result, packed_result_len);
    return _interface->get_code_hash(account_name(account), struct_version, std::move(_packed_result));
}

uint32_t vm_api_proxy::get_block_num() {
    return _interface->get_block_num();
}

void vm_api_proxy::sha3( const char* data, uint32_t data_len, char* hash, uint32_t hash_len, int32_t keccak ) {
   span<const char> _data(data, data_len);
   span<char> _hash(hash, hash_len);
    _interface->sha3(std::move(_data), std::move(_hash), keccak);
}

int32_t vm_api_proxy::blake2_f( uint32_t rounds,
                const char* state, uint32_t state_len,
                const char* msg, uint32_t msg_len, 
                const char* t0_offset, uint32_t t0_len,
                const char* t1_offset, uint32_t t1_len,
                int32_t final,
                char* result, uint32_t result_len) {
    span<const char> _state(state, state_len);
    span<const char> _message(msg, msg_len);
    span<const char> _t0_offset(t0_offset, t0_len);
    span<const char> _t1_offset(t1_offset, t1_len);
    span<char> _result(result, result_len);
    return _interface->blake2_f(rounds, std::move(_state), std::move(_message), std::move(_t0_offset), std::move(_t1_offset), final, std::move(_result));
}

int32_t vm_api_proxy::k1_recover( const char* sig, uint32_t sig_len, const char* dig, uint32_t dig_len, char* pub, uint32_t pub_len) {
    span<const char> _sig(sig, sig_len);
    span<const char> _digest(dig, dig_len);
    span<char> _pub(pub, pub_len);
    return _interface->k1_recover(std::move(_sig), std::move(_digest), std::move(_pub));
}

        //  int32_t alt_bn128_add(span<const char> op1, span<const char> op2, span<char> result) const;

int32_t vm_api_proxy::alt_bn128_add( const char* op1, uint32_t op1_len, const char* op2, uint32_t op2_len, char* result, uint32_t result_len) {
    span<const char> _op1(op1, op1_len);
    span<const char> _op2(op2, op2_len);
    span<char> _result(result, result_len);
    return _interface->alt_bn128_add(std::move(_op1), std::move(_op2), std::move(_result));
}

int32_t vm_api_proxy::alt_bn128_mul( const char* g1, uint32_t g1_len, const char* scalar, uint32_t scalar_len, char* result, uint32_t result_len) {
    span<const char> _g1(g1, g1_len);
    span<const char> _scalar(scalar, scalar_len);
    span<char> _result(result, result_len);
    return _interface->alt_bn128_mul(std::move(_g1), std::move(_scalar), std::move(_result));
}

int32_t vm_api_proxy::alt_bn128_pair( const char* pairs, uint32_t pairs_len) {
    span<const char> _pairs(pairs, pairs_len);
    return _interface->alt_bn128_pair(std::move(_pairs));
}

int32_t vm_api_proxy::mod_exp(
            const char* base, uint32_t base_len,
            const char* exp, uint32_t exp_len,
            const char* mod, uint32_t mod_len,
            char* result, uint32_t result_len) {
    span<const char> _base(base, base_len);
    span<const char> _exp(exp, exp_len);
    span<const char> _mod(mod, mod_len);
    span<char> _result(result, result_len);
    return _interface->mod_exp(std::move(_base), std::move(_exp), std::move(_mod), std::move(_result));
}

int64_t vm_api_proxy::add_security_group_participants(const char* data, uint32_t datalen) {
    _interface->eosio_assert(false, "not implemented");
    return 0;
    // span<const char> _data(data, datalen);
    // return _interface->add_security_group_participants(std::move(_data));
}

int64_t vm_api_proxy::remove_security_group_participants(const char* data, uint32_t datalen) {
    _interface->eosio_assert(false, "not implemented");
    // span<const char> _data(data, datalen);
    // return _interface->remove_security_group_participants(std::move(_data));
    return 0;
}

bool vm_api_proxy::in_active_security_group(const char* data, uint32_t datalen) {
    _interface->eosio_assert(false, "not implemented");
    // span<const char> _data(data, datalen);
    // return _interface->in_active_security_group(std::move(_data));
    return 0;
}

uint32_t vm_api_proxy::get_active_security_group(char* data, uint32_t datalen) {
    _interface->eosio_assert(false, "not implemented");
    // span<char> _data(data, datalen);
    // return _interface->get_active_security_group(std::move(_data));
    return 0;
}
