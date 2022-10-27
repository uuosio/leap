#include <string.h>
#include <softfloat.hpp>
#include "ipyeos.hpp"
#include "intrinsics.h"
extern "C" void extF80M_to_f128M( const extFloat80_t *, float128_t * );

static uint32_t _get_active_producers( capi_name* producers, uint32_t datalen ){
    return get_vm_api_proxy()->get_active_producers(producers, datalen);
}

static int32_t _db_store_i64(uint64_t scope, capi_name table, capi_name payer, uint64_t id,  const void* data, uint32_t len){
    return get_vm_api_proxy()->db_store_i64(scope, table, payer, id, data, len);
}

static void _db_update_i64(int32_t iterator, capi_name payer, const void* data, uint32_t len){
    get_vm_api_proxy()->db_update_i64(iterator, payer, data, len);
}

static void _db_remove_i64(int32_t iterator){
    get_vm_api_proxy()->db_remove_i64(iterator);
}

static int32_t _db_get_i64(int32_t iterator, const void* data, uint32_t len){
    return get_vm_api_proxy()->db_get_i64(iterator, (void *)data, len);
}

static int32_t _db_next_i64(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_next_i64(iterator, primary);
}

static int32_t _db_previous_i64(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_previous_i64(iterator, primary);
}

static int32_t _db_find_i64(capi_name code, uint64_t scope, capi_name table, uint64_t id){
    return get_vm_api_proxy()->db_find_i64(code, scope, table, id);
}

static int32_t _db_lowerbound_i64(capi_name code, uint64_t scope, capi_name table, uint64_t id){
    return get_vm_api_proxy()->db_lowerbound_i64(code, scope, table, id);
}

static int32_t _db_upperbound_i64(capi_name code, uint64_t scope, capi_name table, uint64_t id){
    return get_vm_api_proxy()->db_upperbound_i64(code, scope, table, id);
}

static int32_t _db_end_i64(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_end_i64(code, scope, table);
}

static int32_t _db_idx64_store(uint64_t scope, capi_name table, capi_name payer, uint64_t id, const uint64_t* secondary){
    return get_vm_api_proxy()->db_idx64_store(scope, table, payer, id, secondary);
}

static void _db_idx64_update(int32_t iterator, capi_name payer, const uint64_t* secondary){
    get_vm_api_proxy()->db_idx64_update(iterator, payer, secondary);
}

static void _db_idx64_remove(int32_t iterator){
    get_vm_api_proxy()->db_idx64_remove(iterator);
}

static int32_t _db_idx64_next(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx64_next(iterator, primary);
}

static int32_t _db_idx64_previous(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx64_previous(iterator, primary);
}

static int32_t _db_idx64_find_primary(capi_name code, uint64_t scope, capi_name table, uint64_t* secondary, uint64_t primary){
    return get_vm_api_proxy()->db_idx64_find_primary(code, scope, table, secondary, primary);
}

static int32_t _db_idx64_find_secondary(capi_name code, uint64_t scope, capi_name table, const uint64_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx64_find_secondary(code, scope, table, secondary, primary);
}

static int32_t _db_idx64_lowerbound(capi_name code, uint64_t scope, capi_name table, uint64_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx64_lowerbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx64_upperbound(capi_name code, uint64_t scope, capi_name table, uint64_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx64_upperbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx64_end(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_idx64_end(code, scope, table);
}

static int32_t _db_idx128_store(uint64_t scope, capi_name table, capi_name payer, uint64_t id, const uint128_t* secondary){
    return get_vm_api_proxy()->db_idx128_store(scope, table, payer, id, secondary);
}

static void _db_idx128_update(int32_t iterator, capi_name payer, const uint128_t* secondary){
    get_vm_api_proxy()->db_idx128_update(iterator, payer, secondary);
}

static void _db_idx128_remove(int32_t iterator){
    get_vm_api_proxy()->db_idx128_remove(iterator);
}

static int32_t _db_idx128_next(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx128_next(iterator, primary);
}

static int32_t _db_idx128_previous(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx128_previous(iterator, primary);
}

static int32_t _db_idx128_find_primary(capi_name code, uint64_t scope, capi_name table, uint128_t* secondary, uint64_t primary){
    return get_vm_api_proxy()->db_idx128_find_primary(code, scope, table, secondary, primary);
}

static int32_t _db_idx128_find_secondary(capi_name code, uint64_t scope, capi_name table, const uint128_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx128_find_secondary(code, scope, table, secondary, primary);
}

static int32_t _db_idx128_lowerbound(capi_name code, uint64_t scope, capi_name table, uint128_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx128_lowerbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx128_upperbound(capi_name code, uint64_t scope, capi_name table, uint128_t* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx128_upperbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx128_end(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_idx128_end(code, scope, table);
}

static int32_t _db_idx256_store(uint64_t scope, capi_name table, capi_name payer, uint64_t id, const uint128_t* data, uint32_t data_len ){
    return get_vm_api_proxy()->db_idx256_store(scope, table, payer, id, data, data_len);
}

static void _db_idx256_update(int32_t iterator, capi_name payer, const uint128_t* data, uint32_t data_len){
    get_vm_api_proxy()->db_idx256_update(iterator, payer, data, data_len);
}

static void _db_idx256_remove(int32_t iterator){
    get_vm_api_proxy()->db_idx256_remove(iterator);
}

static int32_t _db_idx256_next(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx256_next(iterator, primary);
}

static int32_t _db_idx256_previous(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx256_previous(iterator, primary);
}

static int32_t _db_idx256_find_primary(capi_name code, uint64_t scope, capi_name table, uint128_t* data, uint32_t data_len, uint64_t primary){
    return get_vm_api_proxy()->db_idx256_find_primary(code, scope, table, data, data_len, primary);
}

static int32_t _db_idx256_find_secondary(capi_name code, uint64_t scope, capi_name table, const uint128_t* data, uint32_t data_len, uint64_t* primary){
    return get_vm_api_proxy()->db_idx256_find_secondary(code, scope, table, data, data_len, primary);
}

static int32_t _db_idx256_lowerbound(capi_name code, uint64_t scope, capi_name table, uint128_t* data, uint32_t data_len, uint64_t* primary){
    return get_vm_api_proxy()->db_idx256_lowerbound(code, scope, table, data, data_len, primary);
}

static int32_t _db_idx256_upperbound(capi_name code, uint64_t scope, capi_name table, uint128_t* data, uint32_t data_len, uint64_t* primary){
    return get_vm_api_proxy()->db_idx256_upperbound(code, scope, table, data, data_len, primary);
}

static int32_t _db_idx256_end(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_idx256_end(code, scope, table);
}

static int32_t _db_idx_double_store(uint64_t scope, capi_name table, capi_name payer, uint64_t id, const double* secondary){
    return get_vm_api_proxy()->db_idx_double_store(scope, table, payer, id, secondary);
}

static void _db_idx_double_update(int32_t iterator, capi_name payer, const double* secondary){
    get_vm_api_proxy()->db_idx_double_update(iterator, payer, secondary);
}

static void _db_idx_double_remove(int32_t iterator){
    get_vm_api_proxy()->db_idx_double_remove(iterator);
}

static int32_t _db_idx_double_next(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_double_next(iterator, primary);
}

static int32_t _db_idx_double_previous(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_double_previous(iterator, primary);
}

static int32_t _db_idx_double_find_primary(capi_name code, uint64_t scope, capi_name table, double* secondary, uint64_t primary){
    return get_vm_api_proxy()->db_idx_double_find_primary(code, scope, table, secondary, primary);
}

static int32_t _db_idx_double_find_secondary(capi_name code, uint64_t scope, capi_name table, const double* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_double_find_secondary(code, scope, table, secondary, primary);
}

static int32_t _db_idx_double_lowerbound(capi_name code, uint64_t scope, capi_name table, double* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_double_lowerbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx_double_upperbound(capi_name code, uint64_t scope, capi_name table, double* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_double_upperbound(code, scope, table, secondary, primary);
}

static int32_t _db_idx_double_end(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_idx_double_end(code, scope, table);
}

static int32_t _db_idx_long_double_store(uint64_t scope, capi_name table, capi_name payer, uint64_t id, const long double* secondary){
    float128_t _secondary;
    extF80M_to_f128M((extFloat80_t *)secondary, &_secondary);
    return get_vm_api_proxy()->db_idx_long_double_store(scope, table, payer, id, (long double *)&_secondary);
}

static void _db_idx_long_double_update(int32_t iterator, capi_name payer, const long double* secondary){
    float128_t _secondary;
    extF80M_to_f128M((extFloat80_t *)secondary, &_secondary);
    get_vm_api_proxy()->db_idx_long_double_update(iterator, payer, (long double *)&_secondary);
}

static void _db_idx_long_double_remove(int32_t iterator){
    get_vm_api_proxy()->db_idx_long_double_remove(iterator);
}

static int32_t _db_idx_long_double_next(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_long_double_next(iterator, primary);
}

static int32_t _db_idx_long_double_previous(int32_t iterator, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_long_double_previous(iterator, primary);
}

static int32_t _db_idx_long_double_find_primary(capi_name code, uint64_t scope, capi_name table, long double* secondary, uint64_t primary){
    int32_t ret = get_vm_api_proxy()->db_idx_long_double_find_primary(code, scope, table, secondary, primary);
    if (ret >= 0) {
        float128_t _secondary;
        f128M_to_extF80M((float128_t *)secondary, (extFloat80_t *)&_secondary);
        memcpy(secondary, &_secondary, sizeof(_secondary));
    }
    return ret;
}

static int32_t _db_idx_long_double_find_secondary(capi_name code, uint64_t scope, capi_name table, const long double* secondary, uint64_t* primary){
    return get_vm_api_proxy()->db_idx_long_double_find_secondary(code, scope, table, secondary, primary);
}

static int32_t _db_idx_long_double_lowerbound(capi_name code, uint64_t scope, capi_name table, long double* secondary, uint64_t* primary){
    float128_t _secondary;
    extF80M_to_f128M((extFloat80_t *)secondary, &_secondary);
    int32_t ret = get_vm_api_proxy()->db_idx_long_double_lowerbound(code, scope, table, (long double *)&_secondary, primary);
    if (ret >= 0) {
        f128M_to_extF80M(&_secondary, (extFloat80_t *)secondary);
    }
    return ret;
}

static int32_t _db_idx_long_double_upperbound(capi_name code, uint64_t scope, capi_name table, long double* secondary, uint64_t* primary){
    float128_t _secondary;
    extF80M_to_f128M((extFloat80_t *)secondary, &_secondary);
    int32_t ret = get_vm_api_proxy()->db_idx_long_double_upperbound(code, scope, table, (long double*)&_secondary, primary);
    if (ret >= 0) {
        f128M_to_extF80M(&_secondary, (extFloat80_t *)secondary);
    }
    return ret;
}

static int32_t _db_idx_long_double_end(capi_name code, uint64_t scope, capi_name table){
    return get_vm_api_proxy()->db_idx_long_double_end(code, scope, table);
}

static int32_t _check_transaction_authorization( const char* trx_data,     uint32_t trx_size, const char* pubkeys_data, uint32_t pubkeys_size,const char* perms_data,   uint32_t perms_size){
    return get_vm_api_proxy()->check_transaction_authorization(trx_data, trx_size, pubkeys_data, pubkeys_size, perms_data, perms_size);
}

static int32_t _check_permission_authorization( capi_name account, capi_name permission, const char* pubkeys_data, uint32_t pubkeys_size, const char* perms_data,   uint32_t perms_size, uint64_t delay_us){
    return get_vm_api_proxy()->check_permission_authorization(account, permission, pubkeys_data, pubkeys_size, perms_data, perms_size, delay_us);
}

int64_t _get_permission_last_used( capi_name account, capi_name permission ){
    return get_vm_api_proxy()->get_permission_last_used(account, permission);
}

int64_t _get_account_creation_time( capi_name account ){
    return get_vm_api_proxy()->get_account_creation_time(account);
}

static void _assert_sha256( const char* data, uint32_t length, const capi_checksum256* hash ){
    get_vm_api_proxy()->assert_sha256(data, length, hash);
}

static void _assert_sha1( const char* data, uint32_t length, const capi_checksum160* hash ){
    get_vm_api_proxy()->assert_sha1(data, length, hash);
}

static void _assert_sha512( const char* data, uint32_t length, const capi_checksum512* hash ){
    get_vm_api_proxy()->assert_sha512(data, length, hash);
}

static void _assert_ripemd160( const char* data, uint32_t length, const capi_checksum160* hash ){
    get_vm_api_proxy()->assert_ripemd160(data, length, hash);
}

static void _sha256( const char* data, uint32_t length, capi_checksum256* hash ){
    get_vm_api_proxy()->sha256(data, length, hash);
}

void  _sha1( const char* data, uint32_t length, capi_checksum160* hash ){
    get_vm_api_proxy()->sha1(data, length, hash);
}

void  _sha512( const char* data, uint32_t length, capi_checksum512* hash ){
    get_vm_api_proxy()->sha512(data, length, hash);
}

void  _ripemd160( const char* data, uint32_t length, capi_checksum160* hash ){
    get_vm_api_proxy()->ripemd160(data, length, hash);
}

static int _recover_key( const capi_checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen ){
    return get_vm_api_proxy()->recover_key(digest, sig, siglen, pub, publen);
}

static void _assert_recover_key( const capi_checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen ){
    get_vm_api_proxy()->assert_recover_key(digest, sig, siglen, pub, publen);
}

static uint32_t _read_action_data( void* msg, uint32_t len ){
    return get_vm_api_proxy()->read_action_data(msg, len);
}

static uint32_t _action_data_size(){
    return get_vm_api_proxy()->action_data_size();
}

static void _require_recipient( capi_name name ){
    get_vm_api_proxy()->require_recipient(name);
}

static void _require_auth( capi_name name ){
    get_vm_api_proxy()->require_auth(name);
}

static bool _has_auth( capi_name name ){
    return get_vm_api_proxy()->has_auth(name);
}

static void _require_auth2( capi_name name, capi_name permission ){
    get_vm_api_proxy()->require_auth2(name, permission);
}

static bool _is_account( capi_name name ){
    return get_vm_api_proxy()->is_account(name);
}

static void _send_inline(char *serialized_action, size_t size){
    get_vm_api_proxy()->send_inline(serialized_action, size);
}

static void _send_context_free_inline(char *serialized_action, size_t size){
    get_vm_api_proxy()->send_context_free_inline(serialized_action, size);
}

static uint64_t _publication_time(){
    return get_vm_api_proxy()->publication_time();
}

static capi_name _current_receiver(){
    return get_vm_api_proxy()->current_receiver();
}

static void _prints( const char* cstr ){
    get_vm_api_proxy()->prints(cstr);
}

static void _prints_l( const char* cstr, uint32_t len){
    get_vm_api_proxy()->prints_l(cstr, len);
}

static void _printi( int64_t value ){
    get_vm_api_proxy()->printi(value);
}

static void _printui( uint64_t value ){
    get_vm_api_proxy()->printui(value);
}

static void _printi128( const int128_t* value ){
    get_vm_api_proxy()->printi128(value);
}

static void _printui128( const uint128_t* value ){
    get_vm_api_proxy()->printui128(value);
}

static void _printsf(float value){
    get_vm_api_proxy()->printsf(value);
}

static void _printdf(double value){
    get_vm_api_proxy()->printdf(value);
}

static void _printqf(const long double* value){
    get_vm_api_proxy()->printqf(value);
}

static void _printn( uint64_t name ){
    get_vm_api_proxy()->printn(name);
}

static void _printhex( const void* data, uint32_t datalen ){
    get_vm_api_proxy()->printhex(data, datalen);
}

static void _eosio_assert( uint32_t test, const char* msg ){
    get_vm_api_proxy()->eosio_assert(test, msg);
}

static void  _eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len ){
    get_vm_api_proxy()->eosio_assert_message(test, msg, msg_len);
}

static void  _eosio_assert_code( uint32_t test, uint64_t code ){
    get_vm_api_proxy()->eosio_assert_code(test, code);
}

static uint64_t _current_time(){
    return get_vm_api_proxy()->current_time();
}

static bool _is_feature_activated( const capi_checksum256* feature_digest ){
    return get_vm_api_proxy()->is_feature_activated(feature_digest);
}

static capi_name _get_sender(){
    return get_vm_api_proxy()->get_sender();
}

static void _get_resource_limits( capi_name account, int64_t* ram_bytes, int64_t* net_weight, int64_t* cpu_weight ){
    get_vm_api_proxy()->get_resource_limits(account, ram_bytes, net_weight, cpu_weight);
}

static void _set_resource_limits( capi_name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ){
    get_vm_api_proxy()->set_resource_limits(account, ram_bytes, net_weight, cpu_weight);
}

static int64_t _set_proposed_producers( char *producer_data, uint32_t producer_data_size ){
    return get_vm_api_proxy()->set_proposed_producers(producer_data, producer_data_size);
}

static int64_t _set_proposed_producers_ex( uint64_t producer_data_format, char *producer_data, uint32_t producer_data_size ){
    return get_vm_api_proxy()->set_proposed_producers_ex(producer_data_format, producer_data, producer_data_size);
}

static bool _is_privileged( capi_name account ){
    return get_vm_api_proxy()->is_privileged(account);
}

static void _set_privileged( capi_name account, bool is_priv ){
    get_vm_api_proxy()->set_privileged(account, is_priv);
}

static void _set_blockchain_parameters_packed( char* data, uint32_t datalen ){
    get_vm_api_proxy()->set_blockchain_parameters_packed(data, datalen);
}

static uint32_t _get_blockchain_parameters_packed( char* data, uint32_t datalen ){
    return get_vm_api_proxy()->get_blockchain_parameters_packed(data, datalen);
}

static void _preactivate_feature( const capi_checksum256* feature_digest ){
    get_vm_api_proxy()->preactivate_feature(feature_digest);
}

static void _send_deferred(const uint128_t& sender_id, capi_name payer, const char *serialized_transaction, size_t size, uint32_t replace_existing){
    get_vm_api_proxy()->send_deferred(sender_id, payer, serialized_transaction, size, 0);
}

static int _cancel_deferred(const uint128_t& sender_id){
    return get_vm_api_proxy()->cancel_deferred(sender_id);
}

static size_t _read_transaction(char *buffer, size_t size){
    return get_vm_api_proxy()->read_transaction(buffer, size);
}

static size_t _transaction_size(){
    return get_vm_api_proxy()->transaction_size();
}

static int _tapos_block_num(){
    return get_vm_api_proxy()->tapos_block_num();
}

static int _tapos_block_prefix(){
    return get_vm_api_proxy()->tapos_block_prefix();
}

static uint32_t _expiration(){
    return get_vm_api_proxy()->expiration();
}

static int _get_action( uint32_t type, uint32_t index, char* buff, size_t size ){
    return get_vm_api_proxy()->get_action(type, index, buff, size);
}

static int _get_context_free_data( uint32_t index, char* buff, size_t size ){
    return get_vm_api_proxy()->get_context_free_data(index, buff, size);
}

static void _set_action_return_value(const char *data, uint32_t data_size) {
    get_vm_api_proxy()->set_action_return_value(data, data_size);
}

static uint32_t _get_code_hash(uint64_t account, uint32_t struct_version, char* packed_result, uint32_t packed_result_len) {
    return get_vm_api_proxy()->get_code_hash(account, struct_version, packed_result, packed_result_len);
}

static uint32_t _get_block_num() {
    return get_vm_api_proxy()->get_block_num();
}

static void _sha3( const char* data, uint32_t data_len, char* hash, uint32_t hash_len, int32_t keccak ) {
    get_vm_api_proxy()->sha3(data, data_len, hash, hash_len, keccak);
}

static int32_t _blake2_f( uint32_t rounds, const char* state, uint32_t state_len, const char* msg, uint32_t msg_len, 
                const char* t0_offset, uint32_t t0_len, const char* t1_offset, uint32_t t1_len, int32_t final, char* result, uint32_t result_len) {
    return get_vm_api_proxy()->blake2_f(rounds, state, state_len, msg, msg_len, 
                t0_offset, t0_len, t1_offset, t1_len, final, result, result_len);
}

static int32_t _k1_recover( const char* sig, uint32_t sig_len, const char* dig, uint32_t dig_len, char* pub, uint32_t pub_len) {
    return get_vm_api_proxy()->k1_recover(sig, sig_len, dig, dig_len, pub, pub_len);
}

static int32_t _alt_bn128_add( const char* op1, uint32_t op1_len, const char* op2, uint32_t op2_len, char* result, uint32_t result_len) {
    return get_vm_api_proxy()->alt_bn128_add(op1, op1_len, op2, op2_len, result, result_len);
}

static int32_t _alt_bn128_mul( const char* g1, uint32_t g1_len, const char* scalar, uint32_t scalar_len, char* result, uint32_t result_len) {
    return get_vm_api_proxy()->alt_bn128_mul(g1, g1_len, scalar, scalar_len, result, result_len);
}

static int32_t _alt_bn128_pair( const char* pairs, uint32_t pairs_len) {
    return get_vm_api_proxy()->alt_bn128_pair(pairs, pairs_len);
}

static int32_t _mod_exp( const char* base, uint32_t base_len, const char* exp, uint32_t exp_len, const char* mod, uint32_t mod_len, char* result, uint32_t result_len) {
    return get_vm_api_proxy()->mod_exp(base, base_len, exp, exp_len, mod, mod_len, result, result_len);
}

static struct IntrinsicsFuncs g_intrinsics_func;
static bool initialized = false;

void init_intrinsics_func() {
    if (initialized) {
        return;
    }
    initialized = true;

    g_intrinsics_func.get_active_producers = _get_active_producers;
    g_intrinsics_func.db_store_i64 = _db_store_i64;
    g_intrinsics_func.db_update_i64 = _db_update_i64;
    g_intrinsics_func.db_remove_i64 = _db_remove_i64;
    g_intrinsics_func.db_get_i64 = _db_get_i64;
    g_intrinsics_func.db_next_i64 = _db_next_i64;
    g_intrinsics_func.db_previous_i64 = _db_previous_i64;
    g_intrinsics_func.db_find_i64 = _db_find_i64;
    g_intrinsics_func.db_lowerbound_i64 = _db_lowerbound_i64;
    g_intrinsics_func.db_upperbound_i64 = _db_upperbound_i64;
    g_intrinsics_func.db_end_i64 = _db_end_i64;
    g_intrinsics_func.db_idx64_store = _db_idx64_store;
    g_intrinsics_func.db_idx64_update = _db_idx64_update;
    g_intrinsics_func.db_idx64_remove = _db_idx64_remove;
    g_intrinsics_func.db_idx64_next = _db_idx64_next;
    g_intrinsics_func.db_idx64_previous = _db_idx64_previous;
    g_intrinsics_func.db_idx64_find_primary = _db_idx64_find_primary;
    g_intrinsics_func.db_idx64_find_secondary = _db_idx64_find_secondary;
    g_intrinsics_func.db_idx64_lowerbound = _db_idx64_lowerbound;
    g_intrinsics_func.db_idx64_upperbound = _db_idx64_upperbound;
    g_intrinsics_func.db_idx64_end = _db_idx64_end;
    g_intrinsics_func.db_idx128_store = _db_idx128_store;
    g_intrinsics_func.db_idx128_update = _db_idx128_update;
    g_intrinsics_func.db_idx128_remove = _db_idx128_remove;
    g_intrinsics_func.db_idx128_next = _db_idx128_next;
    g_intrinsics_func.db_idx128_previous = _db_idx128_previous;
    g_intrinsics_func.db_idx128_find_primary = _db_idx128_find_primary;
    g_intrinsics_func.db_idx128_find_secondary = _db_idx128_find_secondary;
    g_intrinsics_func.db_idx128_lowerbound = _db_idx128_lowerbound;
    g_intrinsics_func.db_idx128_upperbound = _db_idx128_upperbound;
    g_intrinsics_func.db_idx128_end = _db_idx128_end;
    g_intrinsics_func.db_idx256_store = _db_idx256_store;
    g_intrinsics_func.db_idx256_update = _db_idx256_update;
    g_intrinsics_func.db_idx256_remove = _db_idx256_remove;
    g_intrinsics_func.db_idx256_next = _db_idx256_next;
    g_intrinsics_func.db_idx256_previous = _db_idx256_previous;
    g_intrinsics_func.db_idx256_find_primary = _db_idx256_find_primary;
    g_intrinsics_func.db_idx256_find_secondary = _db_idx256_find_secondary;
    g_intrinsics_func.db_idx256_lowerbound = _db_idx256_lowerbound;
    g_intrinsics_func.db_idx256_upperbound = _db_idx256_upperbound;
    g_intrinsics_func.db_idx256_end = _db_idx256_end;
    g_intrinsics_func.db_idx_double_store = _db_idx_double_store;
    g_intrinsics_func.db_idx_double_update = _db_idx_double_update;
    g_intrinsics_func.db_idx_double_remove = _db_idx_double_remove;
    g_intrinsics_func.db_idx_double_next = _db_idx_double_next;
    g_intrinsics_func.db_idx_double_previous = _db_idx_double_previous;
    g_intrinsics_func.db_idx_double_find_primary = _db_idx_double_find_primary;
    g_intrinsics_func.db_idx_double_find_secondary = _db_idx_double_find_secondary;
    g_intrinsics_func.db_idx_double_lowerbound = _db_idx_double_lowerbound;
    g_intrinsics_func.db_idx_double_upperbound = _db_idx_double_upperbound;
    g_intrinsics_func.db_idx_double_end = _db_idx_double_end;
    g_intrinsics_func.db_idx_long_double_store = _db_idx_long_double_store;
    g_intrinsics_func.db_idx_long_double_update = _db_idx_long_double_update;
    g_intrinsics_func.db_idx_long_double_remove = _db_idx_long_double_remove;
    g_intrinsics_func.db_idx_long_double_next = _db_idx_long_double_next;
    g_intrinsics_func.db_idx_long_double_previous = _db_idx_long_double_previous;
    g_intrinsics_func.db_idx_long_double_find_primary = _db_idx_long_double_find_primary;
    g_intrinsics_func.db_idx_long_double_find_secondary = _db_idx_long_double_find_secondary;
    g_intrinsics_func.db_idx_long_double_lowerbound = _db_idx_long_double_lowerbound;
    g_intrinsics_func.db_idx_long_double_upperbound = _db_idx_long_double_upperbound;
    g_intrinsics_func.db_idx_long_double_end = _db_idx_long_double_end;
    g_intrinsics_func.check_transaction_authorization = _check_transaction_authorization;
    g_intrinsics_func.check_permission_authorization = _check_permission_authorization;
    g_intrinsics_func.get_permission_last_used = _get_permission_last_used;
    g_intrinsics_func.get_account_creation_time = _get_account_creation_time;
    g_intrinsics_func.assert_sha256 = _assert_sha256;
    g_intrinsics_func.assert_sha1 = _assert_sha1;
    g_intrinsics_func.assert_sha512 = _assert_sha512;
    g_intrinsics_func.assert_ripemd160 = _assert_ripemd160;
    g_intrinsics_func.sha256 = _sha256;
    g_intrinsics_func.sha1 = _sha1;
    g_intrinsics_func.sha512 = _sha512;
    g_intrinsics_func.ripemd160 = _ripemd160;
    g_intrinsics_func.recover_key = _recover_key;
    g_intrinsics_func.assert_recover_key = _assert_recover_key;
    g_intrinsics_func.read_action_data = _read_action_data;
    g_intrinsics_func.action_data_size = _action_data_size;
    g_intrinsics_func.require_recipient = _require_recipient;
    g_intrinsics_func.require_auth = _require_auth;
    g_intrinsics_func.has_auth = _has_auth;
    g_intrinsics_func.require_auth2 = _require_auth2;
    g_intrinsics_func.is_account = _is_account;
    g_intrinsics_func.send_inline = _send_inline;
    g_intrinsics_func.send_context_free_inline = _send_context_free_inline;
    g_intrinsics_func.publication_time = _publication_time;
    g_intrinsics_func.current_receiver = _current_receiver;
    g_intrinsics_func.prints = _prints;
    g_intrinsics_func.prints_l = _prints_l;
    g_intrinsics_func.printi = _printi;
    g_intrinsics_func.printui = _printui;
    g_intrinsics_func.printi128 = _printi128;
    g_intrinsics_func.printui128 = _printui128;
    g_intrinsics_func.printsf = _printsf;
    g_intrinsics_func.printdf = _printdf;
    g_intrinsics_func.printqf = _printqf;
    g_intrinsics_func.printn = _printn;
    g_intrinsics_func.printhex = _printhex;
    g_intrinsics_func.eosio_assert = _eosio_assert;
    g_intrinsics_func.eosio_assert_message = _eosio_assert_message;
    g_intrinsics_func.eosio_assert_code = _eosio_assert_code;
    g_intrinsics_func.current_time = _current_time;
    g_intrinsics_func.is_feature_activated = _is_feature_activated;
    g_intrinsics_func.get_sender = _get_sender;
    g_intrinsics_func.get_resource_limits = _get_resource_limits;
    g_intrinsics_func.set_resource_limits = _set_resource_limits;
    g_intrinsics_func.set_proposed_producers = _set_proposed_producers;
    g_intrinsics_func.set_proposed_producers_ex = _set_proposed_producers_ex;
    g_intrinsics_func.is_privileged = _is_privileged;
    g_intrinsics_func.set_privileged = _set_privileged;
    g_intrinsics_func.set_blockchain_parameters_packed = _set_blockchain_parameters_packed;
    g_intrinsics_func.get_blockchain_parameters_packed = _get_blockchain_parameters_packed;
    g_intrinsics_func.preactivate_feature = _preactivate_feature;
    g_intrinsics_func.send_deferred = _send_deferred;
    g_intrinsics_func.cancel_deferred = _cancel_deferred;
    g_intrinsics_func.read_transaction = _read_transaction;
    g_intrinsics_func.transaction_size = _transaction_size;
    g_intrinsics_func.tapos_block_num = _tapos_block_num;
    g_intrinsics_func.tapos_block_prefix = _tapos_block_prefix;
    g_intrinsics_func.expiration = _expiration;
    g_intrinsics_func.get_action = _get_action;
    g_intrinsics_func.get_context_free_data = _get_context_free_data;
    g_intrinsics_func.set_action_return_value = _set_action_return_value;

    g_intrinsics_func.set_action_return_value = _set_action_return_value;
    g_intrinsics_func.get_code_hash = _get_code_hash;
    g_intrinsics_func.get_block_num = _get_block_num;

    g_intrinsics_func.sha3 = _sha3;
    g_intrinsics_func.blake2_f = _blake2_f;
    g_intrinsics_func.k1_recover = _k1_recover;
    g_intrinsics_func.alt_bn128_add = _alt_bn128_add;
    g_intrinsics_func.alt_bn128_mul = _alt_bn128_mul;
    g_intrinsics_func.alt_bn128_pair = _alt_bn128_pair;
    g_intrinsics_func.mod_exp = _mod_exp;
}

IntrinsicsFuncs* get_intrinsics() {
    init_intrinsics_func();
    return &g_intrinsics_func;
}
