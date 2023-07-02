// store = std::make_shared<store_provider>(
//    trace_dir,
//    slice_stride,
//    minimum_irreversible_history_blocks,
//    minimum_uncompressed_irreversible_history_blocks,
//    compression_seek_point_stride
// );
#pragma once
#include <string>
#include <stdint.h>

using namespace std;

namespace eosio {
   namespace chain {
      class controller;
   }
}

class _trace_api_proxy;
class trace_api_proxy {
public:
   trace_api_proxy(eosio::chain::controller *chain, string& trace_dir, uint32_t slice_stride, int32_t minimum_irreversible_history_blocks, int32_t minimum_uncompressed_irreversible_history_blocks, uint32_t compression_seek_point_stride);
   ~trace_api_proxy();
   virtual bool get_block_trace(uint32_t block_num, string& result);

private:
   _trace_api_proxy* _my;
};
