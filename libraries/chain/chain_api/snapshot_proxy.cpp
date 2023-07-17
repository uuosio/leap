#include <eosio/chain/controller.hpp>
#include <eosio/chain/snapshot_scheduler.hpp>

#include <boost/signals2/signal.hpp>
#include "snapshot_proxy.hpp"

#include <fc/io/json.hpp>

#include "chain_macro.hpp"

using namespace eosio::chain;
using namespace boost::signals2;

snapshot_proxy::snapshot_proxy(void *_chain) {
    FC_ASSERT(_chain != nullptr, "chain is null");

    eosio::chain::controller& chain = *(eosio::chain::controller *)_chain;
    _snapshot_scheduler = std::make_unique<eosio::chain::snapshot_scheduler>();

    _irreversible_block_connection = std::make_unique<scoped_connection>(chain.irreversible_block.connect([&](const auto& bsp) {
        EOS_ASSERT(chain.is_write_window(), producer_exception, "write window is expected for on_irreversible_block signal");
        _snapshot_scheduler->on_irreversible_block(bsp->block, chain);
    }));

    // _snapshot_scheduler.on_irreversible_block(lib, chain);

    _block_start_connection = std::make_unique<scoped_connection>(chain.block_start.connect([this, &chain](uint32_t bs) {
        try {
            _snapshot_scheduler->on_start_block(bs, chain);
        } catch (const snapshot_execution_exception& e) {
            elog("Exception during snapshot execution: ${e}", ("e", e.to_detail_string()));
            throw e;
            // app().quit();
        }
    }));
}

snapshot_proxy::~snapshot_proxy() {
}

// snapshot scheduler handlers
uint32_t snapshot_proxy::schedule_snapshot(uint32_t block_spacing, uint32_t start_block_num, uint32_t end_block_num, const string& snapshot_description) {
    try {
        auto ret = _snapshot_scheduler->schedule_snapshot(snapshot_scheduler::snapshot_request_information {
            block_spacing,
            start_block_num,
            end_block_num,
            snapshot_description
        });
        return ret.snapshot_request_id;
    } CATCH_AND_LOG_EXCEPTION();
    return std::numeric_limits<uint32_t>::max();
}

string snapshot_proxy::unschedule_snapshot(uint32_t sri) {
    try {
        auto ret = _snapshot_scheduler->unschedule_snapshot(sri);
        return fc::json::to_string(ret, fc::time_point::maximum());
    } CATCH_AND_LOG_EXCEPTION();
    return "";
}

string snapshot_proxy::get_snapshot_requests() {
    auto ret = _snapshot_scheduler->get_snapshot_requests();
    return fc::json::to_string(ret, fc::time_point::maximum());
}

// initialize with storage
void snapshot_proxy::set_db_path(const string& db_path) {
    _snapshot_scheduler->set_db_path(db_path);
}

// set snapshot path
void snapshot_proxy::set_snapshots_path(const string& sn_path) {
    _snapshot_scheduler->set_snapshots_path(sn_path);
}

// add pending snapshot info to inflight snapshot request
void snapshot_proxy::add_pending_snapshot_info(const string& head_block_id, uint32_t head_block_num, int64_t head_block_time, uint32_t version, const string& snapshot_name) {
    _snapshot_scheduler->add_pending_snapshot_info(snapshot_scheduler::snapshot_information {
        fc::variant(head_block_id).as<block_id_type>(),
        head_block_num,
        fc::time_point(fc::microseconds(head_block_time)),
        version,
        snapshot_name
    });
}
