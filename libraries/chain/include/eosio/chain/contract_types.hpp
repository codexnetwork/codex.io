#pragma once

#include <eosio/chain/authority.hpp>
#include <eosio/chain/chain_config.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/asset.hpp>

namespace eosio { namespace chain {

using action_name    = eosio::chain::action_name;

struct newaccount {
   account_name                     creator;
   account_name                     name;
   authority                        owner;
   authority                        active;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(newaccount);
   }
};

struct setconfig {
   account_name  typ;
   int64_t       num = 0;
   account_name  key; // TBD By FanYang will use for cfg future
   asset         fee; // TBD By FanYang will use for cfg future

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return config::action::setconfig_name;
   }
};

struct setcode {
   account_name                     account;
   uint8_t                          vmtype = 0;
   uint8_t                          vmversion = 0;
   bytes                            code;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(setcode);
   }
};

struct setfee{
   account_name  account;
   action_name   action;
   asset         fee;

   uint32_t      cpu_limit = 0;
   uint32_t      net_limit = 0;
   uint32_t      ram_limit = 0;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return config::action::setfee_name;
   }
};

struct setabi {
   account_name                     account;
   bytes                            abi;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(setabi);
   }
};


struct updateauth {
   account_name                      account;
   permission_name                   permission;
   permission_name                   parent;
   authority                         auth;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(updateauth);
   }
};

struct deleteauth {
   deleteauth() = default;
   deleteauth(const account_name& account, const permission_name& permission)
   :account(account), permission(permission)
   {}

   account_name                      account;
   permission_name                   permission;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(deleteauth);
   }
};

struct linkauth {
   linkauth() = default;
   linkauth(const account_name& account, const account_name& code, const action_name& type, const permission_name& requirement)
   :account(account), code(code), type(type), requirement(requirement)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;
   permission_name                   requirement;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(linkauth);
   }
};

struct unlinkauth {
   unlinkauth() = default;
   unlinkauth(const account_name& account, const account_name& code, const action_name& type)
   :account(account), code(code), type(type)
   {}

   account_name                      account;
   account_name                      code;
   action_name                       type;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(unlinkauth);
   }
};

struct canceldelay {
   permission_level      canceling_auth;
   transaction_id_type   trx_id;

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(canceldelay);
   }
};

struct transfer_fee {
   account_name  payer;
   asset         quantity;

   static account_name get_account() {
      return config::token_account_name;
   }

   static action_name get_name() {
      return config::action::fee_name;
   }
};

struct onerror {
   uint128_t      sender_id;
   bytes          sent_trx;

   onerror( uint128_t sid, const char* data, size_t len )
   :sender_id(sid),sent_trx(data,data+len){}

   static account_name get_account() {
      return config::system_account_name;
   }

   static action_name get_name() {
      return N(onerror);
   }
};

} } /// namespace eosio::chain

FC_REFLECT( eosio::chain::newaccount                       , (creator)(name)(owner)(active) )
FC_REFLECT( eosio::chain::setconfig                        , (typ)(num)(key)(fee) )
FC_REFLECT( eosio::chain::setcode                          , (account)(vmtype)(vmversion)(code) )
FC_REFLECT( eosio::chain::setfee                           , (account)(action)(fee)(cpu_limit)(net_limit)(ram_limit) )
FC_REFLECT( eosio::chain::setabi                           , (account)(abi) )
FC_REFLECT( eosio::chain::updateauth                       , (account)(permission)(parent)(auth) )
FC_REFLECT( eosio::chain::deleteauth                       , (account)(permission) )
FC_REFLECT( eosio::chain::linkauth                         , (account)(code)(type)(requirement) )
FC_REFLECT( eosio::chain::unlinkauth                       , (account)(code)(type) )
FC_REFLECT( eosio::chain::canceldelay                      , (canceling_auth)(trx_id) )
FC_REFLECT( eosio::chain::transfer_fee                     , (payer)(quantity) )
FC_REFLECT( eosio::chain::onerror                          , (sender_id)(sent_trx) )
