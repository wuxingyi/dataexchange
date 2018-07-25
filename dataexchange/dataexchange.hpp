#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/crypto.h>
#include <cstring>

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

    //@abi action
    void removemarket(account_name owner, uint64_t marketid);
    //@abi action
    void createmarket(account_name owner, uint64_t type, string desp);
    //@abi action
    void createorder(account_name seller, uint64_t marketid, asset& price);
    //@abi action
    void cancelorder(account_name seller, account_name owner, uint64_t orderid);
    //@abi action
    void buyercancel(account_name buyer, account_name owner, uint64_t orderid);
    //@abi action
    void eraseorder(account_name seller, account_name owner, uint64_t orderid);
    //@abi action
    void tryfillorder(account_name buyer, account_name owner, uint64_t orderid);
    //@abi action
    void finishorder(account_name seller, account_name owner, uint64_t orderid, string datahash);
    //@abi action
    void deposit( account_name from, asset& quantity );
    //@abi action
    void withdraw( account_name owner, asset& quantity );
    //@abi action
    void regpkey( account_name owner, string pkey) ;
    //@abi action
    void deregpkey( account_name owner);
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

    static const uint64_t orderstate_start = 0;
    static const uint64_t orderstate_waitinghash = 1;
    static const uint64_t orderstate_finished = 2;
    static const uint64_t orderstate_canceled = 3;
    static const uint64_t orderstate_end = 4;

    //@abi table askingorders i64
    struct askingorder {
        uint64_t orderid;     //orderid is the primary key for quick erase of pening asking orders
        uint64_t marketid; 

        //account of the seller
        account_name seller;
        //the buyer try to fill this order
        account_name buyer;
        asset price;
        uint64_t orderstate;
        string datahash;

        uint64_t primary_key() const { return orderid; }
        uint64_t by_seller() const { return seller; }
        uint64_t by_marketid() const { return marketid; }
        EOSLIB_SERIALIZE( askingorder, (orderid)(marketid)(seller)(buyer)(price)(orderstate)(datahash))
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
       string       pkey;

       uint64_t primary_key()const { return owner; }

       EOSLIB_SERIALIZE( account, (owner)(asset_balance)(pkey))
    };

    multi_index< N(accounts), account> _accounts;
};
EOSIO_ABI( dataexchange, (createmarket)(removemarket)(createorder)(cancelorder)(buyercancel)(tryfillorder)(eraseorder)(finishorder)(deposit)(withdraw)(regpkey)(deregpkey))