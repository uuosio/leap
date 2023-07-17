#pragma once

#include <string>
#include <memory>

using namespace std;

namespace eosio {
    namespace chain {
        class snapshot_scheduler;
    }
}

namespace boost {
    namespace signals2 {
        class scoped_connection;
    }
}

class snapshot_proxy {
public:
    snapshot_proxy(void *chain);
    virtual ~snapshot_proxy();

    // snapshot scheduler handlers
    virtual uint32_t schedule_snapshot(uint32_t block_spacing, uint32_t start_block_num, uint32_t end_block_num, const string& snapshot_description);
    virtual string unschedule_snapshot(uint32_t sri);
    virtual string get_snapshot_requests();

    // initialize with storage
    virtual void set_db_path(const string& db_path);

    // set snapshot path
    virtual void set_snapshots_path(const string& sn_path);

    // add pending snapshot info to inflight snapshot request
    virtual void add_pending_snapshot_info(const string& head_block_id, uint32_t head_block_num, int64_t head_block_time, uint32_t version, const string& snapshot_name);
private:
    std::unique_ptr<eosio::chain::snapshot_scheduler> _snapshot_scheduler;
    std::unique_ptr<boost::signals2::scoped_connection> _block_start_connection;
    std::unique_ptr<boost::signals2::scoped_connection> _irreversible_block_connection;
};
