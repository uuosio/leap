#include <stdint.h>
#include <vector>

using namespace std;

class transaction_proxy {
public:
    transaction_proxy();
    ~transaction_proxy();
    virtual void *new_transaction(
        uint32_t expiration,
        const char* ref_block_id,
        size_t ref_block_id_size,
        uint32_t max_net_usage_words, //fc::unsigned_int
        uint8_t  max_cpu_usage_ms,    //
        uint32_t delay_sec            //fc::unsigned_int
    );
    virtual void free(void *transaction);
    virtual void add_action(void *transaction, uint64_t account, uint64_t name, const char *data, size_t size, vector<std::pair<uint64_t, uint64_t>>& auths);
    virtual bool sign(void *transaction, const char *private_key, size_t size, const char *chain_id, size_t chain_id_size);
    virtual void pack(void *transaction, bool compress, vector<char>& packed_transaction);
    virtual bool unpack(const char *packed_transaction, size_t size, int result_type, string& result);
};
