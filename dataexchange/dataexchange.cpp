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
            _availableid(_self, _self),
            _markets(_self, _self),
            _askingorders(_self, _self){}

        // @abi action
        void removemarket(account_name owner, uint64_t marketid){
            //only the market owner can create a market
            require_auth(owner);

            auto iter = _markets.find(marketid);
            eosio_assert(iter != _markets.end(), "market not have been created yet");
            eosio_assert(iter->mowner == owner , "have no permission to this market");
            
            _markets.erase(iter);
        }


        // @abi action
        void createmarket(account_name owner, uint64_t type, string desp){
            //only the contract owner can create a market
            require_auth(_self);

            eosio_assert(desp.length() < 30, "market description should be less than 30 characters");
            eosio_assert(hasmarket_byaccountname(owner) == false, "an account can only create only one market now");
            eosio_assert((type > typestart && type < typeend), "out of market type");

            auto iter = _availableid.begin();
            uint64_t id = 0;
            if (iter == _availableid.end()) {
                _availableid.emplace( _self, [&]( auto& row) {
                    row.availmarketid = id;
                    row.availorderid = 0;
                    row.padding = 0;
                });
            } else {
                id = iter->availmarketid + 1;
                _availableid.modify( iter, 0, [&]( auto& row) {
                    row.availmarketid = id;
                });
            }

            _markets.emplace( _self, [&]( auto& row) {
                row.marketid = id;
                row.mowner = owner;
                row.mtype = type;
                row.mdesp = desp;
            });
        }

        // @abi action
        void createorder(account_name seller, uint64_t marketid, uint64_t price, string dataforsell) {
            require_auth(seller);

            eosio_assert(hasmareket_byid(marketid) == true, "no such market");

            auto iter = _availableid.begin();
            eosio_assert(iter != _availableid.end(), "availableid should have been initialized");
            uint64_t id = 0;
            id = iter->availorderid + 1;
            _availableid.modify( iter, 0, [&]( auto& row) {
                row.availorderid = id;
            });

            // we can only put it to the contract owner  scope
            _askingorders.emplace( _self, [&]( auto& order) {
                order.orderid = id;
                order.seller = seller;
                order.marketid = marketid;
                order.price = price;
                order.dataforsell = dataforsell;
            });
        }

        void cancelorder(account_name seller, uint64_t orderid) {
            require_auth(seller);
            auto iter = _askingorders.find(orderid);

            eosio_assert(iter != _askingorders.end() , "no such order");
            eosio_assert(iter->seller == seller, "order doesn't belong to you");
            _askingorders.erase(iter);
        }

    private:
        static const uint64_t typestart = 0;
        static const uint64_t authorities = 1;
        static const uint64_t shopping = 2;
        static const uint64_t travelling = 3;
        static const uint64_t sellouts = 4;        
        static const uint64_t typeend = 5;


        //@abi table availableid i64
        struct availableid {
            uint64_t padding; //(fixme) this is just a simple workaround because modify primary key is not allowed.
            uint64_t availmarketid; 
            uint64_t availorderid; 
            uint64_t primary_key() const { return padding; }
            EOSLIB_SERIALIZE( availableid, (padding)(availmarketid)(availorderid))
        };

        multi_index <N(availableid), availableid> _availableid;

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

        bool hasmareket_byid(uint64_t id)const {
           auto iter = _markets.find(id);
           if (iter == _markets.end()) return false;
           return true;
        }

        bool hasmarket_byaccountname( account_name accnt)const {
           auto idx = _markets.template get_index<N(mowner)>();
           auto itr = idx.find(accnt);
           return itr != idx.end();
        }

        multi_index <N(datamarkets), datamarket, 
                    indexed_by< N(mtype), const_mem_fun<datamarket, uint64_t, &datamarket::by_mtype> >,
                    indexed_by< N(mowner), const_mem_fun<datamarket, uint64_t, &datamarket::by_mowner> >
        > _markets;

        //@abi table askingorders i64
        struct askingorder {
            uint64_t orderid;     //orderid is the primary key for quick erase of pening asking orders
            uint64_t marketid; 

            //account of the seller
            account_name seller;
            uint64_t price;     //(fixme) use asset type
            string dataforsell; //(fixme) need change

            uint64_t primary_key() const { return orderid; }
            uint64_t by_seller() const { return seller; }
            EOSLIB_SERIALIZE( askingorder, (orderid)(marketid)(seller)(price)(dataforsell))
        };


        multi_index <N(askingorders), askingorder ,
                     indexed_by< N(seller), const_mem_fun<askingorder, uint64_t, &askingorder::by_seller> >
        > _askingorders;
};

EOSIO_ABI( dataexchange, (createmarket)(removemarket)(createorder)(cancelorder))
