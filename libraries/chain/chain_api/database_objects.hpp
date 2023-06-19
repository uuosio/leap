#pragma once
#include <eosio/chain/account_object.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/resource_limits_private.hpp>
#include <eosio/chain/protocol_state_object.hpp>
#include <eosio/chain/code_object.hpp>
#include <eosio/chain/database_header_object.hpp>

using namespace eosio::chain;
using namespace std;

struct generated_transaction_
{
    transaction_id_type           trx_id;
    account_name                  sender;
    uint128_t                     sender_id;
    account_name                  payer;
    time_point                    delay_until; /// this generated transaction will not be applied until the specified time
    time_point                    expiration; /// this generated transaction will not be applied after this time
    time_point                    published;
    vector<char>                  packed_trx;
};


template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
unpack_tuple(fc::datastream<const char*>& binary, std::tuple<Tp...>& t)
{ }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
unpack_tuple(fc::datastream<const char*>& binary, std::tuple<Tp...>& t)
{
   fc::raw::unpack(binary, std::get<I>(t));
    unpack_tuple<I + 1, Tp...>(binary, t);
}

template<typename database_object_type>
inline vector<char> pack_database_object(const database_object_type& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 8);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<key_value_object>(const key_value_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index64_object>(const index64_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    elog("+++++++++pack_database_object<index64_object>: ${n}", ("n", obj.id));
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index128_object>(const index128_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index256_object>(const index256_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index_double_object>(const index_double_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<>
inline vector<char> pack_database_object<index_long_double_object>(const index_long_double_object& obj) {
    std::vector<char> vec(fc::raw::pack_size(obj) + 16);
    fc::datastream<char*>  ds(vec.data(), vec.size());
    fc::raw::pack(ds, obj.id);
    fc::raw::pack(ds, obj.t_id);
    fc::raw::pack(ds, obj);
    return vec;
}

template<typename database_object>
inline database_object unpack_database_object(const char *raw_data, size_t raw_data_size) {
    std::vector<char> vec(raw_data, raw_data + raw_data_size);
    return fc::raw::unpack<database_object>(vec);
}

FC_REFLECT(generated_transaction_, (trx_id)(sender)(sender_id)(payer)(delay_until)(expiration)(published)(packed_trx))
