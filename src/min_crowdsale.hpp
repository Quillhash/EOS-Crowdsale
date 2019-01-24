#pragma once

#include <string>
#include <vector>

#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/system.h>
#include <eosiolib/time.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/asset.hpp>

CONTRACT min_crowdsale : public eosio::contract
{
  public:
    // constructor
    min_crowdsale(eosio::name self, eosio::name code, eosio::datastream<const char *> ds);

    // destructor
    ~min_crowdsale();

    ACTION init(eosio::name issuer, eosio::time_point_sec start, eosio::time_point_sec finish); // initialize the crowdsale

    ACTION invest(eosio::name investor, eosio::asset quantity); // transfer crowdsale tokens to the investor

    // ACTION setstart(); // start crowdsale

    // ACTION setfinish(); // stop a crowdsale

    // ACTION withdraw(); // transfer tokens from the contract account to the issuer

  private:
    // type for defining state
    struct state_t
    {
        eosio::name issuer;
        uint64_t total_eoses;
        uint64_t total_tokens;
        eosio::time_point_sec start;
        eosio::time_point_sec finish;

        // utility method for converting this object to string
        std::string toString()
        {
            std::string str = "ISSUER " + this->issuer.to_string() +
                              "\nEOSES " + std::to_string(this->total_eoses) +
                              "\nTOKENS " + std::to_string(this->total_tokens) +
                              "\nSTART " + std::to_string(this->start.utc_seconds) +
                              "\nFINISH " + std::to_string(this->finish.utc_seconds);

            return str;
        }
    };

    // table for holding investors information
    TABLE deposit_t
    {
        eosio::name account;
        uint64_t eoses;
        uint64_t tokens;

        uint64_t primary_key() const { return account.value; }
    };

    // persists the state of the aplication in a singleton. Only one instance will be strored in the RAM for this application
    eosio::singleton<"state"_n, state_t> state_singleton;

    // store investors and balances with contributions in the RAM
    eosio::multi_index<"deposit"_n, deposit_t> deposits;

    // hold present state of the application
    state_t state;

    // /**
    //  * Extended Assets store assets and owner account information along with it.
    //  * These two variables are used to keep eos and token balances of the contract.
    //  *
    //  */
    // eosio::extended_asset asset_eos; // hold eos
    // eosio::extended_asset asset_tkn; // hold native tokens

    // issuer account
    // eosio::name issuer;

    // private function to call issue action from inside the contract
    void inline_issue(eosio::name to, eosio::asset quantity, std::string memo) const
    {
        // define the type for storing issue information
        struct issue
        {
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        // create an instance of the action sender and call send function on it
        eosio::action issue_action = eosio::action(
            eosio::permission_level(this->state.issuer, "active"_n),
            "quillhash111"_n, // name of the contract
            "issue"_n,
            issue{to, quantity, memo});

        issue_action.send();
    }

    // private function to handle token transafers
    void inline_transfer(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(get_self(), "active"_n),
            "quillhash111"_n, // name of the contract
            "transfer"_n,
            transfer{from, to, quantity, memo});

        transfer_action.send();
    }


    // a utility function to return default parameters for the state of the crowdsale
    state_t default_parameters() const
    {
        state_t ret;
        ret.total_eoses = 0;
        ret.total_tokens = 0;
        ret.start = eosio::time_point_sec(0);
        ret.finish = eosio::time_point_sec(0);

        return ret;
    }
};