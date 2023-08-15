FC_REFLECT_ENUM(eosio::chain::db_read_mode, 
                              (HEAD)
                              (IRREVERSIBLE)
                              (SPECULATIVE)
)

FC_REFLECT_ENUM(eosio::chain::validation_mode, (FULL)(LIGHT))

FC_REFLECT_ENUM(eosio::chain::wasm_interface::vm_oc_enable, (oc_auto)(oc_all)(oc_none))

FC_REFLECT_ENUM(chainbase::pinnable_mapped_file::map_mode, (mapped)(heap)(locked))

// FC_REFLECT(eosio::chain::block_log_prune_config, (prune_blocks)(prune_threshold)(vacuum_on_close))

FC_REFLECT(eosio::chain::eosvmoc::config,
                        (cache_size)
                        (threads)
)

FC_REFLECT(eosio::chain::controller::config,
   (sender_bypass_whiteblacklist)
   (actor_whitelist)
   (actor_blacklist)
   (contract_whitelist)
   (contract_blacklist)
   (action_blacklist)
   (key_blacklist)
   (blocks_dir)
   // (prune_config)
   (state_dir)
   (state_size)
   (state_guard_size)
   (sig_cpu_bill_pct)
   (thread_pool_size)
   (max_nonprivileged_inline_action_size)
   (read_only)
   (force_all_checks)
   (disable_replay_opts)
   (contracts_console)
   (allow_ram_billing_in_notify)
   (maximum_variable_signature_length)
   (disable_all_subjective_mitigations)
   (terminate_at_block)
   (integrity_hash_on_start)
   (integrity_hash_on_stop)
   (wasm_runtime)
   (eosvmoc_config)
   (eosvmoc_tierup)
   (read_mode)
   (block_validation_mode)
   (db_map_mode)
   (resource_greylist)
   (trusted_producers)
   (greylist_limit)
   (profile_accounts)
)