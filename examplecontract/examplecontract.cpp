#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;

class examplecontract : public contract {
    using contract::contract;

    public:
        examplecontract( account_name self ) :
            contract(self),
            _statuses(_self, _self){}


        // @abi action
        void wipeall(name sender)
        {
            //find the correct asset
            require_auth( _self );
            
            auto pos = _statuses.begin();
            if (pos == _statuses.end()) { return; }

            do {
                auto iter = pos++;
                if (pos == _statuses.end())break;
                _statuses.erase(pos);
                pos = iter;
            } while(pos != _statuses.end()) ;
        }
        

        // @abi action
        void erase( name sender){
            require_auth(sender);

            auto iter = _statuses.find(sender);
            if(iter != _statuses.end()) {
	            _statuses.erase(iter);
            }
        }


        // @abi action
        void get( name sender){
	    eosio::print("wuxingyihehehehehehe---------------------\r\n");
	    eosio::print(eosio::name{sender});
	    eosio::print("\r\n");
	    eosio::print("wuxingyihehehehehehe---------------------\r\n");
            require_auth(sender);

            auto iter = _statuses.find(sender);
            if(iter != _statuses.end()) {
	            eosio::print(iter->status);
            }
        }

        // @abi action
        void test( name sender, string status ){
	    eosio::print(eosio::name{sender});
	    eosio::print("\r\n");
	    eosio::print(status.c_str());
            require_auth(sender);

            auto iter = _statuses.find(sender);
            if(iter == _statuses.end()) _statuses.emplace( sender, [&]( auto& row) {
                row.sender = sender;
                row.status = status;
            });
            else _statuses.modify( iter, 0, [&]( auto& row) {
               row.status = status;
            });
        }

        // @abi action
        void transfer( account_name from,  
                      account_name to,  
                      asset        quantity,  
                      string       memo ) {
            eosio_assert(0, "i don't want you money" );  
        }
    private:

        // @abi table
        struct statuses {
            name sender;
            string status;

            name primary_key() const { return sender; }
            EOSLIB_SERIALIZE( statuses, (sender)(status) )
        };

        multi_index<N(statuses), statuses> _statuses;


};

EOSIO_ABI( examplecontract, (test)(wipeall) (get)(erase)(transfer))
