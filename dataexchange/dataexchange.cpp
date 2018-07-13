#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;
using eosio::indexed_by;
using eosio::const_mem_fun;

class dataexchange : public contract {
    using contract::contract;

    public:
        dataexchange( account_name self ) :
            contract(self),
            _markets(_self, _self){}

        // @abi action
        void removemarket(account_name owner, uint64_t id){
            //only the market owner can create a market
            require_auth(owner);

            auto iter = _markets.find(id);
            eosio_assert(iter != _markets.end(), "marketname not have been created yet");
            eosio_assert(iter->mowner == owner , "market does not belong to you");
            
            _markets.erase(iter);
        }


        // @abi action
        void createmarket(account_name owner, uint64_t type, string desp){
            //only the contract owner can create a market
            require_auth(_self);

            eosio_assert(desp.length() < 30, "marketname should be less than 30 characters");
            eosio_assert(hasmarket_byaccountname(owner) == false, "an account can only create only one market now");
            eosio_assert((type > typestart && type < typeend), "out of market type");

            auto newid  = _markets.available_primary_key();
            _markets.emplace( _self, [&]( auto& row) {
                row.marketid = newid;
                row.mowner = owner;
                row.mtype = type;
                row.mdesp = desp;
            });
        }

    private:
        static const uint64_t typestart = 0;
        static const uint64_t authorities = 1;
        static const uint64_t shopping = 2;
        static const uint64_t travelling = 3;
        static const uint64_t sellouts = 4;        
        static const uint64_t typeend = 5;

        //@abi table datamarkets i64
        struct datamarket {
            uint64_t marketid; 
            uint64_t mtype;
            string mdesp;
            account_name mowner;

            uint64_t primary_key() const { return marketid; }
            uint64_t by_mtype() const { return mtype; }
            uint64_t by_mowner() const { return mowner; }
            EOSLIB_SERIALIZE( datamarket, (marketid)(mtype)(mdesp)(mowner))
        };


        bool hasmarket_byaccountname( account_name accnt)const {
           auto idx = _markets.template get_index<N(mowner)>();
           auto itr = idx.find(accnt);
           return itr != idx.end();
        }

        multi_index <N(datamarkets), datamarket, 
                    indexed_by< N(mtype), const_mem_fun<datamarket, uint64_t, &datamarket::by_mtype> >,
                    indexed_by< N(mowner), const_mem_fun<datamarket, uint64_t, &datamarket::by_mowner> >
        > _markets;
};

EOSIO_ABI( dataexchange, (createmarket)(removemarket))
