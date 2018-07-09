#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;

class dataexchange : public contract {
    using contract::contract;

    public:
        dataexchange( account_name self ) :
            contract(self),
            _markets(_self, _self){}

        // @abi action
        void removemarket( name creator){
            require_auth(creator);

            auto iter = _markets.find(creator);
            if(iter != _markets.end()) {
                _markets.erase(iter);
            }else {
                eosio_assert(false, "you have no market created");
            }
        }


        // @abi action
        void createmarket( name creator, string marketname){
	        eosio::print(eosio::name{creator});
	        eosio::print("\r\n");
	        eosio::print(marketname.c_str());
            require_auth(creator);

            auto iter = _markets.find(creator);
            if(iter == _markets.end()) _markets.emplace( creator, [&]( auto& row) {
                row.creator = creator;
                row.marketname = marketname;
            });
            else _markets.modify( iter, 0, [&]( auto& row) {
               row.marketname = marketname;
            });
        }

    private:
        // @abi table
        struct datamarkets {
            name creator;
            string marketname;

            name primary_key() const { return creator; }
            EOSLIB_SERIALIZE( datamarkets, (creator)(marketname) )
        };
        multi_index<N(datamarkets), datamarkets> _markets;
};

EOSIO_ABI( dataexchange, (createmarket)(removemarket))
