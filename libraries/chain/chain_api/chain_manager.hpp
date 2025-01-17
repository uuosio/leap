#pragma once
#include <chain_proxy.hpp>
#include <eosio/chain/controller.hpp>

#include <memory>

namespace eosio { namespace chain {

class controller;

class chain_manager {
    public:
        chain_manager(string& config, string& _genesis, string& _chain_id, string& protocol_features_dir, string& snapshot_file);
        ~chain_manager();
        void init();
        bool startup(bool init_db);
        void shutdown();
        eosio::chain::controller *get_controller();
    private:
        std::shared_ptr<controller> c;
        controller::config cfg;
        genesis_state genesis;
        chain_id_type chain_id = chain_id_type::empty_chain_id();
        string snapshot_file;
        string protocol_features_dir;
        bool _shutdown = false;
};

}}

namespace fc {
   void to_variant(const std::filesystem::path& p, fc::variant& v);
   void from_variant(const fc::variant& v, std::filesystem::path& p);
} // fc