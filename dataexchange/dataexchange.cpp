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
            _accounts(_self, _self){}

        // @abi action
        void removemarket(account_name owner, uint64_t marketid){
            //only the market owner can create a market
            require_auth(owner);

            auto iter = _markets.find(marketid);
            eosio_assert(iter != _markets.end(), "market not have been created yet");
            eosio_assert(iter->mowner == owner , "have no permission to this market");
            eosio_assert(hasorder_bymarketid(marketid) == false, "market can't be removed because it still has opening orders");
            
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
        void createorder(account_name seller, uint64_t marketid, const asset& price, string dataforsell) {
            require_auth(seller);

            eosio_assert(hasmareket_byid(marketid) == true, "no such market");

            auto iter = _availableid.begin();
            eosio_assert(iter != _availableid.end(), "availableid should have been initialized");

            uint64_t id = 0;
            id = iter->availorderid + 1;
            _availableid.modify( iter, 0, [&]( auto& row) {
                row.availorderid = id;
            });

            auto miter = _markets.find(marketid);
            eosio_assert(miter->mowner != seller, "please don't sell on your own market");
            ordertable orders(_self, miter->mowner); 

            // we can only put it to the contract owner scope
            orders.emplace(seller, [&]( auto& order) {
                order.orderid = id;
                order.seller = seller;
                order.marketid = marketid;
                order.price = price;
                order.dataforsell = dataforsell;
            });
        }

        // @abi action
        void cancelorder(account_name seller, account_name owner, uint64_t orderid) {
            require_auth(seller);

            ordertable orders(_self, owner);
            auto iter = orders.find(orderid);

            eosio_assert(iter != orders.end() , "no such order");
            eosio_assert(iter->seller == seller, "order doesn't belong to you");
            orders.erase(iter);
        }

        // @abi action
        //owner is the market owner, not the seller, seller is stored in struct order.
        void fillorder(account_name buyer, account_name owner, uint64_t orderid) {
            require_auth(buyer);

            ordertable orders(_self, owner);
            auto iter = orders.find(orderid);

            eosio_assert(iter != orders.end() , "no such order");


            // buyer costs tokens
            auto buyeritr = _accounts.find(buyer);
            eosio_assert(buyeritr != _accounts.end() , "buyer should have deposit token");
            _accounts.modify( buyeritr, 0, [&]( auto& acnt ) {
               acnt.asset_balance -= iter->price;
            });

            auto selleriter = _accounts.find(iter->seller);
            if( selleriter == _accounts.end() ) {
                _accounts.emplace(buyer, [&](auto& acnt){
                  acnt.owner = iter->seller;
               });
            }

            selleriter = _accounts.find(iter->seller);

            // add token to seller's account
            _accounts.modify( selleriter, 0, [&]( auto& acnt ) {
               acnt.asset_balance += iter->price;
            });

            orders.erase(iter);
        }

        //@abi action
        void deposit( const account_name from, const asset& quantity ) {
           
           eosio_assert( quantity.is_valid(), "invalid quantity" );
           eosio_assert( quantity.amount > 0, "must deposit positive quantity" );

           auto itr = _accounts.find(from);
           if( itr == _accounts.end() ) {
              itr = _accounts.emplace(_self, [&](auto& acnt){
                 acnt.owner = from;
              });
           }

           _accounts.modify( itr, 0, [&]( auto& acnt ) {
               acnt.asset_balance += quantity;
           });

           action(
              permission_level{ from, N(active) },
              N(xingyitoken), N(transfer),
              std::make_tuple(from, _self, quantity, std::string("deposit token"))
           ).send();
        }

        //@abi action
        void withdraw( const account_name owner, const asset& quantity ) {
           require_auth( owner );

           eosio_assert( quantity.is_valid(), "invalid quantity" );
           eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

           auto itr = _accounts.find( owner );
           eosio_assert(itr != _accounts.end(), "account has no fund, can't withdraw");

           _accounts.modify( itr, 0, [&]( auto& acnt ) {
              eosio_assert( acnt.asset_balance >= quantity, "insufficient balance" );
              acnt.asset_balance -= quantity;
           });

           action(
              permission_level{ _self, N(active) },
              N(xingyitoken), N(transfer),
              std::make_tuple(_self, owner, quantity, std::string("withdraw token"))
           ).send();

           // erase account when no more fund to free memory 
           if( itr->asset_balance.amount == 0) {
              _accounts.erase(itr);
           }
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
            asset price;
            string dataforsell; //(fixme) need change

            uint64_t primary_key() const { return orderid; }
            uint64_t by_seller() const { return seller; }
            uint64_t by_marketid() const { return marketid; }
            EOSLIB_SERIALIZE( askingorder, (orderid)(marketid)(seller)(price)(dataforsell))
        };

        bool hasorder_bymarketid( account_name id)const {
           ordertable orders(_self, id); 
           auto idx = orders.template get_index<N(marketid)>();
           auto itr = idx.find(id);
           return itr != idx.end();
        }

        typedef multi_index <N(askingorders), askingorder ,
                     indexed_by< N(seller), const_mem_fun<askingorder, uint64_t, &askingorder::by_seller> >,
                     indexed_by< N(marketid), const_mem_fun<askingorder, uint64_t, &askingorder::by_marketid> >
        > ordertable;


        //@abi table accounts i64
        //this table is used to record the tokens per eos account
        //those who have any token deposited to contract will got a entry in this table
        //and got erased after all fund withdrawed to reduce memory usage.
        struct account {
           account_name owner;
           asset        asset_balance;

           uint64_t primary_key()const { return owner; }

           EOSLIB_SERIALIZE( account, (owner)(asset_balance))
        };

        multi_index< N(accounts), account> _accounts;
};

EOSIO_ABI( dataexchange, (createmarket)(removemarket)(createorder)(cancelorder)(fillorder)(deposit)(withdraw))
