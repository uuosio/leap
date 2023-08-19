#include <memory>
#include <string>

using namespace std;

namespace eosio {
    namespace chain {
        class block_log;
        struct signed_block;
        using signed_block_ptr = std::shared_ptr<signed_block>;
    }
}

using namespace eosio::chain;

class ipyeos_proxy;
class signed_block_proxy;

class block_log_proxy {
public:
    block_log_proxy(const string& data_dir);
    ~block_log_proxy();

    virtual void *get_block_log_ptr();
    virtual signed_block_proxy *read_block_by_num(uint32_t block_num);
    virtual string read_block_header_by_num(uint32_t block_num);
    virtual string read_block_id_by_num(uint32_t block_num);
    virtual string read_block_by_id(const string& id);
    virtual string head();
    virtual uint32_t head_block_num();
    virtual string head_id();
    virtual uint32_t first_block_num();

private:
    std::unique_ptr<eosio::chain::block_log> _block_log;
};
