#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include<eosiolib/singleton.hpp>
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
        _markets(_self, _self),
        _accounts(_self, _self),
        _availableid(_self, _self),
        _deals(_self, _self){}

    //@abi action
    void removemarket(account_name owner, uint64_t marketid);
    //@abi action
    void createmarket(account_name owner, uint64_t type, string desp);
    //@abi action
    void createorder(account_name orderowner, uint64_t ordertype, uint64_t marketid, asset& price);
    //@abi action
    void removeorder(account_name orderowner, account_name marketowner, uint64_t orderid);
    //@abi action
    void canceldeal(account_name canceler, account_name owner, uint64_t dealid);
    //@abi action
    void erasedeal(uint64_t dealid);
    //@abi action
    void makedeal(account_name taker, account_name marketowner, uint64_t orderid);
    //@abi action
    void uploadhash(uint64_t marketid, uint64_t dealid, string datahash);
    //@abi action
    void deposit( account_name from, asset& quantity );
    //@abi action
    void withdraw( account_name owner, asset& quantity );
    //@abi action
    void regpkey( account_name owner, string pkey) ;
    //@abi action
    void deregpkey( account_name owner);
    //@abi action
    void authorize(account_name maker, uint64_t dealid);
    //@abi action
    void suspendorder(account_name orderowner, account_name marketowner, uint64_t orderid);
    //@abi action
    void resumeorder(account_name orderowner, account_name marketowner, uint64_t orderid);
    //@abi action
    void suspendmkt(account_name owner, uint64_t marketid);
    //@abi action
    void resumemkt(account_name owner, uint64_t marketid);

private:
    static const uint64_t typestart = 0;
    static const uint64_t authorities = 1;
    static const uint64_t shopping = 2;
    static const uint64_t travelling = 3;
    static const uint64_t sellouts = 4;        
    static const uint64_t typeend = 5;


    static const uint64_t market_min_suspendtoremoveal_interval = 5;
    static const uint64_t deal_expire_interval = 10;

    struct availableid {
        uint64_t availmarketid; 
        uint64_t availorderid; 
        uint64_t availdealid; 
        availableid():availmarketid(0),availorderid(0),availdealid(0){}
        availableid(uint64_t mid, uint64_t oid, uint64_t did):availmarketid(mid),availorderid(oid),availdealid(did){}
        EOSLIB_SERIALIZE( availableid, (availmarketid)(availorderid)(availdealid))
    };

    singleton<N(availableid), availableid> _availableid;

    struct marketstats {
        //nr means number
        uint64_t totalopenorders_nr;
        uint64_t suspendedorders_nr;
        uint64_t finisheddeals_nr;
        uint64_t ongoingdeals_nr;
        EOSLIB_SERIALIZE( marketstats, (totalopenorders_nr)(suspendedorders_nr)(finisheddeals_nr)(ongoingdeals_nr))
    };


    //@abi table datamarkets i64
    struct datamarket {
        uint64_t marketid; 
        uint64_t mtype;
        string mdesp;
        account_name mowner;
        bool issuspended;
        bool isremoved;
        time_point_sec minremovaltime;
        marketstats mstats;

        uint64_t primary_key() const { return marketid; }
        uint64_t by_mtype() const { return mtype; }
        uint64_t by_mowner() const { return mowner; }
        EOSLIB_SERIALIZE( datamarket, (marketid)(mtype)(mdesp)(mowner)(issuspended)(isremoved)(minremovaltime)(mstats))
    };

    bool hasmareket_byid(uint64_t id)const {
       auto iter = _markets.find(id);
       if (iter == _markets.end()) return false;
       return true;
    }

    bool cancreate( account_name accnt)const {
       auto idx = _markets.template get_index<N(mowner)>();
       auto itr = idx.find(accnt);
       if (itr == idx.end()) return true;
       return itr->isremoved;
    }

    multi_index <N(datamarkets), datamarket, 
                indexed_by< N(mtype), const_mem_fun<datamarket, uint64_t, &datamarket::by_mtype> >,
                indexed_by< N(mowner), const_mem_fun<datamarket, uint64_t, &datamarket::by_mowner> >
    > _markets;

    static const uint64_t dealstate_start = 0;
    static const uint64_t dealstate_waitingauthorize = 1;
    static const uint64_t dealstate_waitinghash = 2;
    static const uint64_t dealstate_finished = 3;
    static const uint64_t dealstate_canceled = 4;
    static const uint64_t dealstate_expired = 5;
    static const uint64_t dealstate_end = 6;

    static const uint64_t ordertype_start = 0;
    static const uint64_t ordertype_ask = 1;
    static const uint64_t ordertype_bid = 2;
    static const uint64_t ordertype_end = 3;

    //@abi table deals i64
    struct deal {
        uint64_t dealid;
        uint64_t orderid;
        uint64_t marketid;
        account_name marketowner;
        account_name maker;
        account_name taker;
        uint64_t ordertype;
        uint64_t dealstate;
        string datahash;
        asset price;
        time_point_sec expiretime;

        uint64_t primary_key() const { return dealid; }
        EOSLIB_SERIALIZE( deal, (dealid)(orderid)(marketid)(marketowner)(maker)(taker)(dealstate)(ordertype)(datahash)(price)(expiretime))
    }; 
    multi_index< N(deals), deal> _deals;

    //@abi table askingorders i64
    struct askingorder {
        uint64_t orderid;     //orderid is the primary key for quick erase of pening asking orders
        uint64_t marketid; 
        account_name orderowner;
        uint64_t order_type; 
        asset price;
        bool issuspended; 

        uint64_t primary_key() const { return orderid; }
        uint64_t by_orderowner() const { return orderowner; }
        uint64_t by_marketid() const { return marketid; }
        EOSLIB_SERIALIZE( askingorder, (orderid)(marketid)(orderowner)(order_type)(price)(issuspended))
    };

    bool hasorder_byorderowner( account_name marketowner, account_name _orderowner)const {
       askingordertable orders(_self, marketowner); 
       auto idx = orders.template get_index<N(orderowner)>();
       auto itr = idx.find(_orderowner);
       return itr != idx.end();
    }

    typedef multi_index <N(askingorders), askingorder ,
                 indexed_by< N(orderowner), const_mem_fun<askingorder, uint64_t, &askingorder::by_orderowner> >,
                 indexed_by< N(marketid), const_mem_fun<askingorder, uint64_t, &askingorder::by_marketid> >
    > askingordertable;


    //@abi table accounts i64
    //this table is used to record the tokens per eos account
    //those who have any token deposited to contract will got a entry in this table
    //and got erased after all fund withdrawed to reduce memory usage.
    struct account {
       account_name owner;
       asset        asset_balance;
       string       pkey;
       uint64_t     finished_deals;
       uint64_t     outgoingbuy_deals;
       uint64_t     outgoingsell_deals;
       uint64_t     expired_deals;

       uint64_t primary_key()const { return owner; }

       EOSLIB_SERIALIZE( account, (owner)(asset_balance)(pkey)(finished_deals)(outgoingbuy_deals)(outgoingsell_deals)(expired_deals))
    };

    multi_index< N(accounts), account> _accounts;
};
EOSIO_ABI( dataexchange, (createmarket)(removemarket)(createorder)(removeorder)(canceldeal)(makedeal)(erasedeal)(uploadhash)(deposit)(withdraw)(regpkey)(deregpkey)
           (authorize)(suspendorder)(resumeorder)(suspendmkt)(resumemkt)
         )