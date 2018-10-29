#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/crypto.h>

#define EOS_SYMBOL symbol(symbol_code("EOS"), 4)
#define TOP_SYMBOL symbol(symbol_code("TOP"), 4)

using namespace eosio;
using namespace std;

//using eosio::asset;
//using std::string;

CONTRACT beteos : public eosio::contract
{
  public:
    using contract::contract;
    beteos(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds),
                                                                    _bets(receiver, receiver.value), _currentbets(receiver, receiver.value) {}

    struct [[eosio::table]] bet
    {
        uint64_t key;
        uint64_t level;
        name player1;
        name player2;
        uint64_t invest;
        name winner;
        int result;
        uint64_t primary_key() const { return key; }
        uint64_t by_level() const { return level; }
        EOSLIB_SERIALIZE(bet, (key)(level)(player1)(player2)(invest)(winner)(result))
    };

    struct [[eosio::table]] cbet
    {
        uint64_t key;
        uint64_t level;
        name player1;
        name player2;
        uint64_t invest;

        uint64_t primary_key() const { return key; }
        uint64_t by_level() const { return level; }
        EOSLIB_SERIALIZE(cbet, (key)(level)(player1)(player2)(invest))
    };

    struct rd
    {
        name p1;
        name p2;
        uint64_t t;
        name last;
        uint64_t s;
    };

	//true 1 
    ACTION debug(name user)
    {
        print("Hello, ", name{user});
    }
	//debug 

    ACTION acivegame(bool act)
    {
        active = act;
        seed = 1;
        print(active, seed);
    }

    auto rand(const name &player1, name &player2, name &lp, uint64_t s)
    {
        rd r = {player1, player2, current_time(), lp, s};
        capi_checksum256 result;
        sha256((char *)&r, sizeof(r) * 2, &result);
        int res = result.hash[1] < result.hash[0] ? 1 : 2;
        return res;
    }

    ACTION transfer(name from, name to, asset quantity, string memo)
    {
        require_auth(from);
        eosio_assert(active == true, "only active");
        eosio_assert(quantity.is_valid(), "Invalid token transfer...");
        eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS token is allowed");
        eosio_assert(quantity.amount > 0, "must buy a positive amount");
        print("\n ontransfer from:", from, " to:", to, " quantity:", quantity, " memo:", memo);
        if (to != _self)
            return;

        auto level_index = _currentbets.get_index<name("level")>();
        uint32_t findindex = quantity.amount;
        auto itr = level_index.find(findindex);
        if (itr != level_index.end())
        {
            print("\n find ! key:", itr->key, "level:", itr->level, " player1:", itr->player1, " player2", itr->player2, "invest", itr->invest);
            if (itr->player1.value == 0)
            {
                print("\n player1 is nil", itr->player1);
                level_index.modify(itr, get_self(), [&](auto &p) {
                    p.player1 = from;
                    p.invest += quantity.amount;
                });
            }
            else
            {
                print("\n player2 is nil ready to random", itr->player1, from, lastplayer, seed);
                auto res = rand(itr->player1, from, lastplayer, seed);
                name winner = res == 1 ? itr->player1 : from;
                uint64_t total = itr->invest + quantity.amount;
                uint64_t com = total * 0.01;
                uint64_t reward = total - com;
                
                // reward
                asset rewardresult(reward, quantity.symbol);
                print("\n res is :", res," invest total:", total, "winner:", winner," symbol:: ", quantity.symbol," reward:", rewardresult);

                action(
                    permission_level{get_self(), "active"_n},
                    "eosio.token"_n,
                    "transfer"_n,
                    std::make_tuple(get_self(), winner, rewardresult, std::string("reward from top eos"))
                ).send();

                // history save
                _bets.emplace(get_self(), [&](auto &p) {
                    p.key = _bets.available_primary_key();
                    p.level = itr->level;
                    p.player1 = itr->player1;
                    p.player2 = from;
                    p.invest = total;
                    p.result = res;
                    p.winner = winner;
                });

                // clean current
                name iname;
                level_index.modify(itr, get_self(), [&](auto &p) {
                    p.player1 = iname;
                    p.player2 = iname;
                    p.invest = 0;
                });

                lastplayer = winner;
                seed++;
                print("\n clean! lastpl and seed ", lastplayer, seed);
            }
        }
        else
        {
            _currentbets.emplace(get_self(), [&](auto &p) {
                p.key = _currentbets.available_primary_key();
                p.level = quantity.amount;
                p.player1 = from;
                p.invest += quantity.amount;
            });
        }
    };

  private:
    bool active;
    name lastplayer;
    uint64_t seed;
    typedef eosio::multi_index<name("bet"), bet, indexed_by<name("level"), const_mem_fun<bet, uint64_t, &bet::by_level>>> betting;
    typedef eosio::multi_index<name("cbet"), cbet, indexed_by<name("level"), const_mem_fun<cbet, uint64_t, &cbet::by_level>>> cbetting;
    betting _bets;
    cbetting _currentbets;
};

extern "C"
{
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        print(name{receiver}, name{code}, name{action});
        if (code == receiver)
        {
            switch (action)
            {
                EOSIO_DISPATCH_HELPER(beteos, (debug)(acivegame))
            }
        }
        else if (code == "eosio.token"_n.value && action == "transfer"_n.value)
        {
            execute_action(name(receiver), name(code), &beteos::transfer);
        }
    }
};
