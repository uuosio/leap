#include <eosio/chain/application.hpp>

#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/net_plugin/net_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>
#include <eosio/resource_monitor_plugin/resource_monitor_plugin.hpp>
#include <eosio/version/version.hpp>

#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/exception/exception.hpp>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <future>

#include "config.hpp"

#include <ipyeos.hpp>
#include <chain_macro.hpp>

using namespace appbase;
using namespace eosio;


enum return_codes {
   OTHER_FAIL        = -2,
   INITIALIZE_FAIL   = -1,
   SUCCESS           = 0,
   BAD_ALLOC         = 1,
   DATABASE_DIRTY    = 2,
   FIXED_REVERSIBLE  = SUCCESS,
   EXTRACTED_GENESIS = SUCCESS,
   NODE_MANAGEMENT_SUCCESS = 5
};


namespace eosio {
   fc::logger& get_net_plugin_logger(); // net_plugin_impl
   fc::logger& get_http_plugin_logger(); // http_plugin
   fc::logger& get_producer_plugin_logger();// producer_plugin

   fc::logger& get_trace_api_logger();// trace_api
   fc::logger& get_state_history_logger();// state_history
   fc::logger& get_deep_mind_logger();// deep_mind

   fc::logger& get_producer_plugin_logger();
   fc::logger& get_trx_successful_trace_log();
   fc::logger& get_trx_failed_trace_log();
   fc::logger& get_trx_trace_success_log();
   fc::logger& get_trx_trace_failure_log();
   fc::logger& get_trx_log();
   fc::logger& get_transient_trx_successful_trace_log();
   fc::logger& get_transient_trx_failed_trace_log();
}

namespace detail {

fc::logging_config& add_deep_mind_logger(fc::logging_config& config) {
   config.appenders.push_back(
      fc::appender_config( "deep-mind", "dmlog" )
   );

   fc::logger_config dmlc;
   dmlc.name = "deep-mind";
   dmlc.level = fc::log_level::debug;
   dmlc.enabled = true;
   dmlc.appenders.push_back("deep-mind");

   config.loggers.push_back( dmlc );

   return config;
}

void configure_logging(const std::filesystem::path& config_path)
{
   try {
      try {
         if( std::filesystem::exists( config_path ) ) {
            fc::configure_logging( config_path );
         } else {
            auto cfg = fc::logging_config::default_config();

            fc::configure_logging( ::detail::add_deep_mind_logger(cfg) );
         }
      } catch (...) {
         elog("Error reloading logging.json");
         throw;
      }
   } catch (const fc::exception& e) {
      elog("${e}", ("e",e.to_detail_string()));
   } catch (const boost::exception& e) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
   } catch (const std::exception& e) {
      elog("${e}", ("e",e.what()));
   } catch (...) {
      // empty
   }
}

} // namespace detail

static void logging_conf_handler()
{
   auto config_path = app().get_logging_conf();
   if( std::filesystem::exists( config_path ) ) {
      ilog( "Received HUP.  Reloading logging configuration from ${p}.", ("p", config_path.string()) );
   } else {
      ilog( "Received HUP.  No log config found at ${p}, setting to default.", ("p", config_path.string()) );
   }
   ::detail::configure_logging( config_path );
   fc::log_config::initialize_appenders();
}

static void _initialize_logging(string& config_path)
{
   //initialize logger_map in log_config
   fc::logger::get("deep-mind");
   fc::logger::get("net_plugin_impl");
   fc::logger::get("http_plugin");
   fc::logger::get("trace_api");
   fc::logger::get("state_history");


   fc::logger::get("producer_plugin");
   fc::logger::get("transaction_success_tracing");
   fc::logger::get("transaction_failure_tracing");
   fc::logger::get("transaction_trace_success");
   fc::logger::get("transaction_trace_failure");
   fc::logger::get("transaction");
   fc::logger::get("transient_trx_success_tracing");
   fc::logger::get("transient_trx_failure_tracing");

   if(std::filesystem::exists(config_path)) {
      ilog("Found logging configuration file at ${p}, using settings from this file.", ("p", config_path));
     fc::configure_logging(config_path); // intentionally allowing exceptions to escape
   }
   else {
      auto cfg = fc::logging_config::default_config();
      ilog("No logging configuration file found at ${p}, using default logging settings.", ("p", config_path));
      fc::configure_logging( ::detail::add_deep_mind_logger(cfg) );
   }

   fc::log_config::initialize_appenders();

   app().set_sighup_callback(logging_conf_handler);


   fc::logger::update("producer_plugin", eosio::get_producer_plugin_logger());
   fc::logger::update("transaction_success_tracing", get_trx_successful_trace_log());
   fc::logger::update("transaction_failure_tracing", get_trx_failed_trace_log());
   fc::logger::update("transaction_trace_success", get_trx_trace_success_log());
   fc::logger::update("transaction_trace_failure", get_trx_trace_failure_log());
   fc::logger::update("transaction", get_trx_log());
   fc::logger::update("transient_trx_success_tracing", get_transient_trx_successful_trace_log());
   fc::logger::update("transient_trx_failure_tracing", get_transient_trx_failed_trace_log());

   fc::logger::update("deep-mind", eosio::get_deep_mind_logger());
   fc::logger::update("net_plugin_impl", eosio::get_net_plugin_logger());
   fc::logger::update("http_plugin", eosio::get_http_plugin_logger());
   fc::logger::update("trace_api", eosio::get_trace_api_logger());
   fc::logger::update("state_history", eosio::get_state_history_logger());
}

static void initialize_logging()
{
   auto config_path = app().get_logging_conf().string();
   _initialize_logging(config_path);
}

extern "C" int start_python(int argc, char **argv);

int eos_init(int argc, char** argv);
int pyeos_init(int argc, char** argv);
int eos_exec();
int eos_exec_once();

void app_quit() {
    appbase::app().quit();
}

bool eos_cb::post_signed_block(const char *raw_block, size_t raw_block_size, bool _async) {
   auto block = std::make_shared<signed_block>();
   fc::raw::unpack(raw_block, raw_block_size, *block);

   if (!_async) {
      try {
         return app().get_plugin<producer_plugin>().on_incoming_block(block, {}, {});
      } CATCH_AND_LOG_EXCEPTION()
      return false;
   }

   std::promise<bool> promise;
   std::future<bool> future = promise.get_future();
   appbase::app().post(appbase::priority::high, [block, &promise]() {
         bool ret = false;
         try {
            ret = app().get_plugin<producer_plugin>().on_incoming_block(block, {}, {});
         } CATCH_AND_LOG_EXCEPTION()
         promise.set_value(ret);
      }
   );

   while (!should_exit()) {
      if (future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) {
         return future.get();
      }
   }
   return false;
}

bool eos_cb::set_on_produce_block_cb(fn_on_produce_block cb) {
   _on_produce_block = cb;
   return true;
}

bool eos_cb::on_produce_block(const char *raw_block, size_t raw_block_size) {
   if (_on_produce_block != nullptr) {
      return _on_produce_block(raw_block, raw_block_size);
   }
   return false;
}

eos_cb::eos_cb() {

}

eos_cb::~eos_cb() {

}

int eos_cb::init(int argc, char** argv) {
   return eos_init(argc, argv);
}

int eos_cb::init2(int argc, char** argv) {
   return pyeos_init(argc, argv);
}

int eos_cb::exec() {
   return eos_exec();
}

int eos_cb::exec_once() {
   return eos_exec_once();
}

void eos_cb::initialize_logging(string& config_path) {
   ::_initialize_logging(config_path);
}

// all, 
// debug, 
// info, 
// warn, 
// error, 
// off 
void eos_cb::print_log(int level, string& logger_name, string& message) {
   auto _logger = fc::logger::get(logger_name);
   if (_logger.is_enabled(fc::log_level(level))) {
      if (level == 0)
         _logger.log(FC_LOG_MESSAGE(all, message));
      else if (level == 1)
         _logger.log(FC_LOG_MESSAGE(debug, message));
      else if (level == 2)
         _logger.log(FC_LOG_MESSAGE(info, message));
      else if (level == 3)
         _logger.log(FC_LOG_MESSAGE(warn, message));
      else if (level == 4)
         _logger.log(FC_LOG_MESSAGE(error, message));
      // else if (level == 5)
      //    _logger->log(FC_LOG_MESSAGE(off, message));
   }
}

void eos_cb::quit() {
   _should_exit = true;
   app_quit();
}

bool eos_cb::is_quiting() {
   return appbase::app().is_quiting();
}

void* eos_cb::post(void* (*fn)(void *), void *params) {
   std::promise<void *> promise;
   std::future<void *> future = promise.get_future();
   appbase::app().post(appbase::priority::low, [fn, params, &promise]() {
         auto ret = fn(params);
         promise.set_value(ret);
      }
   );
   while (!should_exit()) {
      if (future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) {
         return future.get();
      }
   }
   return nullptr;
}

void eos_cb::exit() {
   _should_exit = true;
}

bool eos_cb::should_exit() {
   return _should_exit;
}

void *eos_cb::get_controller() {
   if (this->controller == nullptr) {
      this->controller = (void *)&app().get_plugin<eosio::chain_plugin>().chain();
   }
   return this->controller;
}

void eos_cb::set_controller(void *_controller) {
   this->controller = _controller;
}


void *eos_cb::get_database() {
   return (void *)&app().get_plugin<eosio::chain_plugin>().chain().db();
}

void eos_cb::set_log_level(string& logger_name, int level) {
   fc::logger::get(logger_name).set_log_level(fc::log_level(level));
}

int eos_cb::get_log_level(string& logger_name) {
   return fc::logger::get(logger_name).get_log_level();
}

string eos_cb::data_dir() {
   return appbase::app().data_dir().string();
}

string eos_cb::config_dir() {
   return appbase::app().config_dir().string();
}

extern int main_cleos( int argc, char** argv );
extern int main_keosd( int argc, char** argv );

int main_nodeos( int argc, char** argv ) {
   auto ret = eos_init(argc, argv);
   if (ret != 0) {
      return ret;
   }
   if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
      return 0;
   }
   return eos_exec();
}

int main(int argc, char** argv)
{
   if (argc >= 2 && strcmp(argv[1], "nodeos") == 0) {
      return main_nodeos(argc-1, &argv[1]);
   }

   if (argc >= 2 && strcmp(argv[1], "cleos") == 0) {
      return main_cleos(argc-1, &argv[1]);
   }

   if (argc >= 2 && strcmp(argv[1], "keosd") == 0) {
      return main_keosd(argc-1, &argv[1]);
   }

   eos_cb *cb = new eos_cb();
   ipyeos_init_chain(cb);
   return start_python(argc, argv);
}

int try_exec(std::function<int(int argc, char** argv)> fn, int argc, char** argv)
{
   try {
      return fn(argc, argv);
   } catch( const extract_genesis_state_exception& e ) {
      return EXTRACTED_GENESIS;
   } catch( const fixed_reversible_db_exception& e ) {
      return FIXED_REVERSIBLE;
   } catch( const node_management_success& e ) {
      return NODE_MANAGEMENT_SUCCESS;
   } catch (const std_exception_wrapper& e) {
      if( e.code() == fc::std_exception_code ) {
         if( e.top_message().find( "atabase dirty flag set" ) != std::string::npos ) {
            elog( "database dirty flag set (likely due to unclean shutdown): replay required" );
            return DATABASE_DIRTY;
         }
      }
      elog( "${e}", ("e", e.to_detail_string()));
      return OTHER_FAIL;
   } catch( const boost::interprocess::bad_alloc& e ) {
      elog("bad alloc");
      return BAD_ALLOC;
   } catch( const boost::exception& e ) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
      return OTHER_FAIL;
   } catch( const std::runtime_error& e ) {
      if( std::string(e.what()).find("atabase dirty flag set") != std::string::npos ) {
         elog( "database dirty flag set (likely due to unclean shutdown): replay required" );
         return DATABASE_DIRTY;
      } else {
         elog( "${e}", ("e",e.what()));
      }
      return OTHER_FAIL;
   } catch( const std::exception& e ) {
      elog("${e}", ("e",e.what()));
      return OTHER_FAIL;
   } catch( ... ) {
      elog("unknown exception");
      return OTHER_FAIL;
   }

   ilog("${name} successfully exiting", ("name", nodeos::config::node_executable_name));
   return SUCCESS;
}

template<typename... plugins>
int _eos_init(int argc, char** argv)
{
   uint32_t short_hash = 0;
   fc::from_hex(eosio::version::version_hash(), (char*)&short_hash, sizeof(short_hash));

   appbase::app().set_version(htonl(short_hash));
   appbase::app().set_version_string(eosio::version::version_client());
   appbase::app().set_full_version_string(eosio::version::version_full());

   auto root = fc::app_path();
   appbase::app().set_default_data_dir(root / "eosio" / nodeos::config::node_executable_name / "data" );
   appbase::app().set_default_config_dir(root / "eosio" / nodeos::config::node_executable_name / "config" );
   http_plugin::set_defaults({
      .default_unix_socket_path = "",
      .default_http_port = 8888,
      .server_header = nodeos::config::node_executable_name + "/" + appbase::app().version_string()
   });
   if(!appbase::app().initialize<plugins...>(argc, argv, initialize_logging)) {
      const auto& opts = appbase::app().get_options();
      if( opts.count("help") || opts.count("version") || opts.count("full-version") || opts.count("print-default-config") ) {
         return SUCCESS;
      }
      return INITIALIZE_FAIL;
   }
   if (auto resmon_plugin = appbase::app().find_plugin<resource_monitor_plugin>()) {
      resmon_plugin->monitor_directory(appbase::app().data_dir());
   } else {
      elog("resource_monitor_plugin failed to initialize");
      return INITIALIZE_FAIL;
   }
   initialize_logging();
   ilog( "${name} version ${ver} ${fv}",
         ("name", nodeos::config::node_executable_name)("ver", appbase::app().version_string())
         ("fv", appbase::app().version_string() == appbase::app().full_version_string() ? "" : appbase::app().full_version_string()) );
   ilog("${name} using configuration file ${c}", ("name", nodeos::config::node_executable_name)("c", appbase::app().full_config_file_path().string()));
   ilog("${name} data directory is ${d}", ("name", nodeos::config::node_executable_name)("d", appbase::app().data_dir().string()));
   appbase::app().startup();
   appbase::app().set_thread_priority_max();
   return SUCCESS;
}

int eos_init(int argc, char** argv) {
   auto fn = [](int argc, char** argv){
      return _eos_init<chain_plugin, net_plugin, producer_plugin, resource_monitor_plugin>(argc, argv);
   };
   return try_exec(fn, argc, argv);
}

int pyeos_init(int argc, char** argv) {
   auto fn = [](int argc, char** argv){
      return _eos_init<producer_plugin>(argc, argv);
   };
   return try_exec(fn, argc, argv);
}

int eos_exec() {
   auto fn = [](int argc, char** argv){
      appbase::app().exec();
      return SUCCESS;
   };
   return try_exec(fn, 0, (char **)nullptr);
}


int _eos_exec_once() {
   auto& exec = appbase::app().executor();
   std::exception_ptr eptr = nullptr;

   auto& io_serv{exec.get_io_service()};
   boost::asio::io_service::work work(io_serv);
   (void)work;
   bool more = true;

   // if (exec.get_priority_queue().size() == 0) {
   //    more = io_serv.poll();
   // }
   io_serv.poll(); // queue up any ready; allowing high priority item to get into the queue
   // execute the highest priority item
   more = exec.execute_highest();
   return more;
   if (!more) {
      more = io_serv.run_one();
   }

   if (more) {
      return true;
   }

   try {
      exec.clear(); // make sure the queue is empty
      appbase::app().shutdown();   // may rethrow exceptions
   } catch (...) {
      if (!eptr)
         eptr = std::current_exception();
   }

   // if we caught an exception while in the application loop, rethrow it so that main()
   // can catch it and report the error
   if (eptr)
      std::rethrow_exception(eptr);

   return false;
}

int eos_exec_once() {
   auto fn = [](int argc, char** argv){
      if (_eos_exec_once()) {
         return SUCCESS;
      }
      return OTHER_FAIL;
   };
   return try_exec(fn, 0, (char **)nullptr);
}
