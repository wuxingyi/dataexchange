#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;

class scopelist: public contract {
    using contract::contract;

    public:
        scopelist( account_name self ) : contract(self){}

        // @abi action
        void selltomarket(account_name seller, account_name marketowner, uint64_t price) {
            require_auth(seller);

            //always use marketowner scope
            order_table test(_self, marketowner);

            auto priid = test.available_primary_key();

            //the seller should play it's memory usage for it's order
            test.emplace( seller, [&]( auto& row) {
                row.price = price;
                row.orderid = priid;
            });
        }

    private:

        //@abi table orders i64
        struct order {
            uint64_t orderid;     
            uint64_t price;

            uint64_t primary_key() const { return orderid; }
            EOSLIB_SERIALIZE( order, (orderid)(price))
        };


        typedef multi_index <N(orders), order> order_table;

};

EOSIO_ABI( scopelist, (selltomarket))
