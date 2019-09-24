/**
 *  Some config data in chain state db
 *  @copyright defined in eosforce/LICENSE.txt
 */

#ifndef EOSIO_CONFIG_ON_CHAIN_HPP
#define EOSIO_CONFIG_ON_CHAIN_HPP

#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/multi_index_includes.hpp>
#include <eosio/chain/contract_types.hpp>
#include <eosio/chain/apply_context.hpp>

namespace eosio { namespace chain {

namespace config{
   namespace res_typ{
      static const auto cpu_per_fee          = N(res.cpufee);
      static const auto net_per_fee          = N(res.netfee);
      static const auto free_ram_per_account = N(res.freeram);
      static const auto ram_rent_b_per_eos   = N(res.ramrent);
   };

   namespace func_typ{
      static const auto chain_maintain_stat = N(f.maintain);
   };

   namespace list_typ{
      static const auto list_actor_black = N(l.actblack);
      static const auto list_contract_black = N(l.crtblack);
      static const auto list_resource_grey = N(l.resgrey);
   };

   namespace gmr_typ{
      static const auto gmr_cpu = N(gmr.cpu);
      static const auto gmr_ram = N(gmr.ram);
      static const auto gmr_net = N(gmr.net);
   }

};

// some spec type for fast
class config_data_object : public chainbase::object<config_data_object_type, config_data_object> {
   OBJECT_CTOR(config_data_object);

   id_type id;
   account_name typ;
   int64_t num = 0;
   account_name key; // TBD By FanYang will use for cfg future
   asset fee; // TBD By FanYang will use for cfg future
};

struct by_name;
using config_data_object_index = chainbase::shared_multi_index_container<
      config_data_object,
      indexed_by<
            ordered_unique< tag<by_id>,
                  BOOST_MULTI_INDEX_MEMBER(config_data_object, config_data_object::id_type, id)
            >,
            ordered_unique< tag<by_name>,
                  BOOST_MULTI_INDEX_MEMBER(config_data_object, name, typ)
            >
      >
>;

// get_num_config_on_chain return -1 if no found
int64_t get_num_config_on_chain( const chainbase::database& db, const name& typ, const int64_t default_value = -1 );

// set_num_config_on_chain if is -1 err
void set_num_config_on_chain( chainbase::database& db, const name& typ, const int64_t num );

// set_config_on_chain set chain cfg
void set_config_on_chain( chainbase::database& db, const setconfig &cfg );

// is_func_has_open is a func is open
bool is_func_has_open( const controller& ctl, const name &func_typ, const int64_t default_open_block = 0);

// is_func_open_in_curr_block if a func is open in curr block
bool is_func_open_in_curr_block( const controller& ctl, const name &func_typ, const int64_t default_open_block = 0 );

//set list config
void set_list_config_on_chain(controller& ctl,const setconfig &cfg);

void set_guaranteed_minimum_config(controller& ctl,const setconfig &cfg);

} } /// namespace eosio::chain

FC_REFLECT(eosio::chain::config_data_object, (id)(typ)(num)(key)(fee))
CHAINBASE_SET_INDEX_TYPE(eosio::chain::config_data_object, eosio::chain::config_data_object_index)


#endif //EOSIO_CONFIG_ON_CHAIN_HPP
