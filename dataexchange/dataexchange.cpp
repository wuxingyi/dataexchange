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
        void removemarket(uint64_t id){
            require_auth(_self);

            auto iter = _markets.find(id);
            eosio_assert(iter != _markets.end(), "marketname not have been created yet");

            _markets.erase(iter);
        }


        // @abi action
        void createmarket(uint64_t marketname){
            //only the owner can create a market
            require_auth(_self);
            //eosio_assert(marketname.length() < 30, "marketname should be less than 30 characters");
            eosio_assert(has_market(marketname) == false, "marketname already exists");

            auto newid  = _markets.available_primary_key();
            _markets.emplace( _self, [&]( auto& row) {
                row.marketid = newid;
                row.marketname = marketname;
            });
        }

    private:
        // @abi table datamarkets i64
        struct datamarkets {
            uint64_t marketid; 
            uint64_t marketname;

            uint64_t by_marketname() const { return marketname; }
            uint64_t primary_key() const { return marketid; }
            EOSLIB_SERIALIZE( datamarkets, (marketid)(marketname))
        };


        bool has_market( uint64_t market)const {
           auto idx = _markets.template get_index<N(marketname)>();
           auto itr = idx.find(market);
           return itr != idx.end();
        }

        multi_index <N(datamarkets), datamarkets, 
                    indexed_by< N(marketname), const_mem_fun<datamarkets, uint64_t, &datamarkets::by_marketname> >
        > _markets;
};

EOSIO_ABI( dataexchange, (createmarket)(removemarket))
