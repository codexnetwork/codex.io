/**
 *  Some config data in chain state db
 *  @copyright defined in eosforce/LICENSE.txt
 */

#include <eosio/chain/config_on_chain.hpp>
#include <chainbase/chainbase.hpp>
#include <eosio/chain/contract_types.hpp>
#include <eosio/chain/apply_context.hpp>

namespace eosio { namespace chain {

// get_num_config_on_chain return -1 if no found
int64_t get_num_config_on_chain( const chainbase::database& db, const name& typ, const int64_t default_value/* = -1*/ ) {
   const auto cfg_itr = db.find<config_data_object, by_name>(typ);
   if( cfg_itr == nullptr ) {
      //ilog("no found num cfg ${n}", ( "n", typ ));
      return default_value;
   }
   return cfg_itr->num;
}

void set_num_config_on_chain( chainbase::database& db, const name& typ, const int64_t num ) {
   auto itr = db.find<config_data_object, by_name>(typ);
   if( itr == nullptr ) {
      //ilog("set num config ${t} to ${v}", ( "n", typ )("v", num));
      db.create<config_data_object>([&]( auto& c ) {
         c.typ = typ;
         c.num = num;
      });
   } else {
      db.modify<config_data_object>(*itr, [&]( auto& c ) {
         c.num = num;
      });
   }
}

void set_config_on_chain( chainbase::database& db, const setconfig &cfg ) {
   auto itr = db.find<config_data_object, by_name>(cfg.typ);
   if( itr == nullptr ) {
      ilog("set num config ${n} to ${v}", ( "n", cfg.typ )("v", cfg));
      db.create<config_data_object>([&]( auto& c ) {
         c.typ = cfg.typ;
         c.num = cfg.num;
         c.key = cfg.key;
         c.fee = cfg.fee;
      });
   } else {
      db.modify<config_data_object>(*itr, [&]( auto& c ) {
         c.num = cfg.num;
         c.key = cfg.key;
         c.fee = cfg.fee;
      });
   }
}

bool is_func_has_open( const controller& ctl, const name &func_typ, const int64_t default_open_block) {
      const auto head_num = static_cast<int64_t>( ctl.head_block_num() );
      const auto open_num = get_num_config_on_chain( ctl.db(), func_typ );

   return (head_num >= 0) && ((open_num >= 0 && head_num >= open_num)
                              || (open_num == -1 && default_open_block != 0 && head_num >= default_open_block));
}

// is_func_open_in_curr_block if a func is open in curr block
bool is_func_open_in_curr_block( const controller& ctl, const name &func_typ, const int64_t default_open_block /* =0 */  ) {
   const auto head_num = static_cast<int64_t>( ctl.head_block_num() );
   const auto open_num = get_num_config_on_chain( ctl.db(), func_typ );
   if( open_num < 0 ) {
      // no cfg
      return (default_open_block > 0) && (head_num == default_open_block); // if head_num < 0 , default_open_block != head_num
   }
   if( head_num < 0 ){
      return false;
   }
   return head_num == open_num;
}

void set_list_config_on_chain(controller& ctl,const setconfig &cfg) {
   // if else
   list_type set_type = list_type::list_type_count;
   if (cfg.typ == config::list_typ::list_actor_black) {
      set_type = list_type::actor_blacklist_type;
   }
   else if(cfg.typ == config::list_typ::list_contract_black) {
      set_type = list_type::contract_blacklist_type;
   }
   else if(cfg.typ == config::list_typ::list_resource_grey) {
      set_type = list_type::resource_greylist_type;
   }
   else {
      return ;
   }

  // if (set_type == list_type::list_type_count) return ;

   list_action_type action = list_action_type::list_action_type_count;
   if (cfg.num == 1) {
      action = list_action_type::insert_type;
   }
   else if (cfg.num == 2) {
      action = list_action_type::remove_type;
   }
   else {
      return ;
   }

//   if(action == list_action_type::list_action_type_count) return ;

   if (set_type == list_type::list_type_count) return ;
   
   std::vector<account_name> name_list;
   name_list.push_back(cfg.key);
   
   ctl.set_name_list(set_type,action,name_list);
}

void set_guaranteed_minimum_config(controller& ctl,const setconfig &cfg) {
   gmr_type set_gmr = gmr_type::gmr_type_count;
   if(cfg.typ == config::gmr_typ::gmr_cpu) {
      set_gmr = gmr_type::cpu_us_type;
   }
   else if(cfg.typ == config::gmr_typ::gmr_ram) {
      set_gmr = gmr_type::ram_byte_type;
   }
   else if(cfg.typ == config::gmr_typ::gmr_net) {
      set_gmr = gmr_type::net_byte_type;
   }
   else {
      return ;
   }

   ctl.set_gmr_config(set_gmr,cfg.num);
}

} }  /// eosio::chain