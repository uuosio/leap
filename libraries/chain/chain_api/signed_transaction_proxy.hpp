#include <stdint.h>
#include <vector>
#include <memory>

using namespace std;
namespace eosio {
    namespace chain {
        struct signed_transaction;
        using signed_transaction_ptr = std::shared_ptr<signed_transaction>;
    }
};

using namespace std;
using namespace eosio::chain;

class signed_transaction_impl;

class signed_transaction_proxy {
public:
    signed_transaction_proxy(
        uint32_t expiration,
        const char* ref_block_id,
        size_t ref_block_id_size,
        uint32_t max_net_usage_words, //fc::unsigned_int
        uint8_t  max_cpu_usage_ms,    //
        uint32_t delay_sec            //fc::unsigned_int
    );
    signed_transaction_proxy(signed_transaction_ptr& transaction);
    std::shared_ptr<signed_transaction> get_transaction();
    virtual ~signed_transaction_proxy();
    virtual string first_authorizer();
    virtual void id(vector<char>& result);
    virtual void add_action(uint64_t account, uint64_t name, const char *data, size_t size, vector<std::pair<uint64_t, uint64_t>>& auths);
    virtual bool sign(const char *private_key, size_t size, const char *chain_id, size_t chain_id_size);
    virtual void pack(bool compress, int pack_type, vector<char>& packed_transaction);
    virtual bool to_json(int result_type, bool compressed, string& result);
private:
    std::shared_ptr<signed_transaction_impl> impl;
};
