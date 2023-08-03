#include <vector>
#include <eosio/ship_protocol.hpp>

#include <ipyeos_proxy.hpp>
#include <chain_macro.hpp>

string eos_cb::unpack_native_object(int type, const char *packed_native_object, size_t packed_native_object_size) {
    try {
        eosio::input_stream is(packed_native_object, packed_native_object_size);
        std::vector<eosio::ship_protocol::transaction_trace> obj;
        eosio::from_bin(obj, is);

        std::vector<char> data;
        eosio::vector_stream os(data);
        eosio::to_json(obj, os);
        return string(data.begin(), data.end());
    } CATCH_AND_LOG_EXCEPTION()
    return "";
}

