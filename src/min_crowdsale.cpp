#include "min_crowdsale.hpp"

#include "config.h"
#include "pow10.hpp"
// #include "src/utils/str_expand.hpp"

// utility macro for converting EOS to our tokens
#define EOS2TKN(EOS) (int64_t)((EOS)*POW10(DECIMALS) * RATE / (1.0 * POW10(4) * RATE_DENOM))

#define NOW now() // rename system func that returns time in seconds

// constructor
min_crowdsale::min_crowdsale(eosio::name self, eosio::name code, eosio::datastream<const char *> ds) : eosio::contract(self, code, ds),
                                                                                                       state_singleton(this->_self, this->_self.value), // code and scope both set to the contract's account
                                                                                                       deposits(this->_self, this->_self.value),
                                                                                                       state(state_singleton.exists() ? state_singleton.get() : default_parameters())
                                                                                                    //    asset_eos( // initialize eos asset
                                                                                                    //        eosio::asset(0, eosio::string_to_symbol(4, "EOS")),
                                                                                                    //        "eosio.token"_n),
                                                                                                    //    asset_tkn(
                                                                                                    //        eosio::asset(0, eosio::string_to_symbol(DECIMALS, STR(SYMBOL))),
                                                                                                    //        eosio::string_to_name(STR(CONTRACT)))
//    issuer("quilltest111"_n)
{
    eosio::print("Inside constructor\n");
    //eosio::print(self);
    eosio::print("\n");
}

// destructor
min_crowdsale::~min_crowdsale()
{
    this->state_singleton.set(this->state, this->_self); // persist the state of the crowdsale before destroying instance

    eosio::print("Saving state to the RAM\n");
    eosio::print(this->state.toString());
}

// initialize the crowdfund
void min_crowdsale::init(eosio::name issuer, eosio::time_point_sec start, eosio::time_point_sec finish)
{
    eosio_assert(!this->state_singleton.exists(), "Already Initialzed");
    eosio_assert(start < finish, "Start must be less than finish");
    require_auth(this->_self);

    // update state
    this->state.issuer = issuer;
    this->state.start = start;
    this->state.finish = finish;
}

// invest EOS and fet tokens
void min_crowdsale::invest(eosio::name investor , eosio::asset quantity)
{
    require_auth(investor); // only caller can invest

    eosio::print("Inside the invest action\n");
    eosio::print(investor);
    eosio::print("\n");

    // check timings of the eos crowdsale
    eosio_assert(NOW >= this->state.start.utc_seconds, "Crowdsale hasn't started");
    eosio_assert(NOW <= this->state.finish.utc_seconds, "Crowdsale finished");

    // check the minimum and maximum contribution
    eosio_assert(quantity.amount >= MIN_CONTRIB, "Contribution too low");
    eosio_assert((quantity.amount <= MAX_CONTRIB) || !MAX_CONTRIB, "Contribution too high");

    // hold the reference to the investor stored in the RAM
    auto it = this->deposits.find(investor.value);

    // calculate from EOS to tokens
    int64_t tokens_to_give = EOS2TKN(quantity.amount);

    // update total eos obtained and tokens distributed
    this->state.total_eoses += quantity.amount;
    this->state.total_tokens += tokens_to_give;

    // check if the hard cap was reached
    eosio_assert(this->state.total_tokens <= HARD_CAP_TKN, "Hard cap reached");

    // if the depositor account was found, store his updated balances
    int64_t entire_eoses = quantity.amount;
    int64_t entire_tokens = tokens_to_give;
    if (it != this->deposits.end())
    {
        entire_eoses += it->eoses;
        entire_tokens += it->tokens;
    }

    // if the depositor was not found create a new entry in the database, else update his balance
    if (it == this->deposits.end())
    {
        this->deposits.emplace(this->_self, [investor, entire_eoses, entire_tokens](auto &deposit) {
            deposit.account = investor;
            deposit.eoses = entire_eoses;
            deposit.tokens = entire_tokens;
        });
    }
    else
    {
        this->deposits.modify(it, this->_self, [investor, entire_eoses, entire_tokens](auto &deposit) {
            deposit.account = investor;
            deposit.eoses = entire_eoses;
            deposit.tokens = entire_tokens;
        });
    }

    // set the amounts to transfer, then call inline issue action to update balances in the token contract
    eosio::asset amount = eosio::asset(tokens_to_give, quantity.symbol);
    // this->inline_issue(investor, amount, "Crowdsale deposit");

    this->inline_transfer(investor, get_self(), quantity, "Crowdsale deposit");
}

EOSIO_DISPATCH(min_crowdsale, (init)(invest))
// EOSIO_DISPATCH(min_crowdsale, (hi))