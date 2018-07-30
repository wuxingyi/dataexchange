#include "dataexchange.hpp"
#include "base58.hpp"

using namespace std;
void dataexchange::removemarket(account_name owner, uint64_t marketid){
    //only the market owner can create a market
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(hasorder_bymarketid(marketid) == false, "market can't be removed because it still has opening orders");
    
    _markets.erase(iter);
}


void dataexchange::createmarket(account_name owner, uint64_t type, string desp){
    //only the contract owner can create a market
    require_auth(_self);

    eosio_assert(desp.length() < 30, "market description should be less than 30 characters");
    eosio_assert(hasmarket_byaccountname(owner) == false, "an account can only create only one market now");
    eosio_assert((type > typestart && type < typeend), "out of market type");

    uint64_t newid = 0;
    if (_availableid.exists()) {
        auto iditem = _availableid.get();
        newid = ++iditem.availmarketid;
        _availableid.set(iditem, _self);
    } else {
        _availableid.set(availableid(), _self);
    }

    _markets.emplace( _self, [&]( auto& row) {
        row.marketid = newid;
        row.mowner = owner;
        row.mtype = type;
        row.mdesp = desp;
    });
}

void dataexchange::suspendmkt(account_name owner, uint64_t marketid){
    //only the market owner can suspend a market
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(iter->issuspended == false, "market should be work now");

    ordertable orders(_self, owner);
    auto orderiter = orders.begin();
    while(true) {
        if (orderiter != orders.end()) {
             orders.modify( orderiter, 0, [&]( auto& order) {
                order.issuspended = true;
             });
            orderiter++;
        } else {
            break;
        }
    }
    _markets.modify( iter, 0, [&]( auto& mkt) {
       mkt.issuspended = true;
    });
}

void dataexchange::resumemkt(account_name owner, uint64_t marketid){
    //only the market owner can resume a market
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(iter->issuspended == true, "market should be suspened");

    ordertable orders(_self, owner);
    auto orderiter = orders.begin();
    while(true) {
        if (orderiter != orders.end()) {
             orders.modify( orderiter, 0, [&]( auto& order) {
                order.issuspended = true;
             });
            orderiter++;
        } else {
            break;
        }
    }
    _markets.modify( iter, 0, [&]( auto& mkt) {
       mkt.issuspended = false;
    });
}


void dataexchange::createorder(account_name seller, uint64_t marketid, asset& price) {
    require_auth(seller);

    eosio_assert( price.is_valid(), "invalid price" );
    eosio_assert( price.amount > 0, "price must be positive price" );
    eosio_assert(hasmareket_byid(marketid) == true, "no such market");

    auto iditem = _availableid.get();
    auto newid = ++iditem.availorderid;
    _availableid.set(availableid(iditem.availmarketid, newid, iditem.availdealid), _self);

    auto miter = _markets.find(marketid);
    eosio_assert(miter->mowner != seller, "please don't sell on your own market");
    eosio_assert(miter->issuspended != true, "this market has already suspened, can't create orders");
    ordertable orders(_self, miter->mowner); 

    // we can only put it to the contract owner scope
    orders.emplace(seller, [&]( auto& order) {
        order.orderid = newid;
        order.seller = seller;
        order.marketid = marketid;
        order.price = price;
    });

   //reg seller to accounts table 
   auto itr = _accounts.find(seller);
   if( itr == _accounts.end() ) {
      itr = _accounts.emplace(_self, [&](auto& acnt){
         acnt.owner = seller;
      });
   }
}

void dataexchange::suspendorder(account_name seller, account_name owner, uint64_t orderid) {
    require_auth(seller);

    ordertable orders(_self, owner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->seller == seller, "order doesn't belong to you");
    eosio_assert(iter->issuspended == false, "order should be work");
    orders.modify( iter, 0, [&]( auto& order) {
       order.issuspended = true;
    });
}

void dataexchange::resumeorder(account_name seller, account_name owner, uint64_t orderid) {
    require_auth(seller);

    ordertable orders(_self, owner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->seller == seller, "order doesn't belong to you");
    eosio_assert(iter->issuspended == true, "order should be suspened");
    orders.modify( iter, 0, [&]( auto& order) {
       order.issuspended = false;
    });
}

void dataexchange::cancelorder(account_name seller, account_name owner, uint64_t orderid) {
    require_auth(seller);

    ordertable orders(_self, owner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->seller == seller, "order doesn't belong to you");
    orders.erase(iter);
}

void dataexchange::canceldeal(account_name buyer, account_name owner, uint64_t dealid) {
    require_auth(buyer);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->orderstate == orderstate_waitinghash || dealiter->orderstate == orderstate_waitingauthorize, "order state is not orderstate_waitinghash");
    eosio_assert(dealiter->buyer == buyer, "not belong to this buyer");
    _deals.erase(dealiter);

    // refund buyer's tokens
    auto buyeritr = _accounts.find(buyer);
    eosio_assert(buyeritr != _accounts.end() , "buyer should have have account");
    _accounts.modify( buyeritr, 0, [&]( auto& acnt ) {
       acnt.asset_balance += dealiter->price;
    });
}

void dataexchange::erasedeal(uint64_t dealid) {
    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->orderstate == orderstate_finished, "order state is not orderstate_waitinghash");
    _deals.erase(dealiter);
}

//owner is the market owner, not the seller, seller is stored in struct order.
void dataexchange::makedeal(account_name buyer, account_name owner, uint64_t orderid) {
    require_auth(buyer);

    ordertable orders(_self, owner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->issuspended != true , "can not make deals because the order is suspended");

    // buyer costs tokens
    auto buyeritr = _accounts.find(buyer);
    eosio_assert(buyeritr != _accounts.end() , "buyer should have deposit token");
    _accounts.modify( buyeritr, 0, [&]( auto& acnt ) {
       acnt.asset_balance -= iter->price;
    });

    auto iditem = _availableid.get();
    auto newid = ++iditem.availdealid;
    _availableid.set(availableid(iditem.availmarketid, iditem.availorderid, newid), _self);

    //use self scope to make it simple for memory deleting.
    //it's not good for the seller to erase such entry because he may not see the datahash if the entry is deleted by seller too quickly.
    _deals.emplace(_self, [&](auto& deal) { 
        deal.dealid = newid;
        deal.marketowner = owner;
        deal.orderid = orderid;
        deal.datahash = "";
        deal.orderstate = orderstate_waitingauthorize;
        deal.buyer = buyer;
        deal.seller = iter->seller;
        deal.price = iter->price;
    });
}

void dataexchange::authorize(account_name seller, uint64_t dealid) {
    require_auth(seller);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->orderstate == orderstate_waitingauthorize, "order state is not orderstate_waitingauthorize");
    eosio_assert(dealiter->seller == seller, "this deal doesnot belong to you");
    _deals.modify( dealiter, 0, [&]( auto& deal) {
       deal.orderstate = orderstate_waitinghash;
    });
}


//datahash is generated using the buyers public key encrypted user's data.
//uploadhash is called by datasource(aka market owner).
void dataexchange::uploadhash(account_name marketowner, uint64_t dealid, string datahash) {
    require_auth(marketowner);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->orderstate == orderstate_waitinghash, "order state is not orderstate_waitinghash");
    eosio_assert(dealiter->marketowner == marketowner, "not correct market owner");
    _deals.modify( dealiter, 0, [&]( auto& deal) {
       deal.datahash = datahash;
       deal.orderstate = orderstate_finished;
    });

    // add token to seller's account
    auto selleriter = _accounts.find(dealiter->seller);
    if( selleriter == _accounts.end() ) {
        selleriter = _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = dealiter->seller;
        });
    }

    auto sellertoken = asset(uint64_t(0.9 * dealiter->price.amount));
    auto sourcetoken = asset(uint64_t(0.1 * dealiter->price.amount));

    _accounts.modify( selleriter, 0, [&]( auto& acnt ) {
       acnt.asset_balance += sellertoken;
    });

    // add token to data source account
    auto sourceitr = _accounts.find(dealiter->marketowner);
    if( sourceitr == _accounts.end() ) {
        sourceitr = _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = dealiter->marketowner;
        });
    }

    _accounts.modify( sourceitr, 0, [&]( auto& acnt ) {
       acnt.asset_balance += sourcetoken;
    });
}

void dataexchange::deposit(account_name from, asset& quantity ) {
   require_auth( from);
   
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

   //make sure contract xingyitoken have been deployed to blockchain to make it runnable
   //xingyitoken is our own token, its symbol is SYS
   action(
      permission_level{ from, N(active) },
      N(xingyitoken), N(transfer),
      std::make_tuple(from, _self, quantity, std::string("deposit token"))
   ).send();
}

void dataexchange::withdraw(account_name owner, asset& quantity ) {
   require_auth( owner );

   eosio_assert( quantity.is_valid(), "invalid quantity" );
   eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

   auto itr = _accounts.find( owner );
   eosio_assert(itr != _accounts.end(), "account has no fund, can't withdraw");

   _accounts.modify( itr, 0, [&]( auto& acnt ) {
      eosio_assert( acnt.asset_balance >= quantity, "insufficient balance" );
      acnt.asset_balance -= quantity;
   });

   //make sure contract xingyitoken have been deployed to blockchain to make it runnable
   //xingyitoken is our own token, its symblo is SYS
   action(
      permission_level{ _self, N(active) },
      N(xingyitoken), N(transfer),
      std::make_tuple(_self, owner, quantity, std::string("withdraw token"))
   ).send();

   // erase account when no more fund to free memory 
   if( itr->asset_balance.amount == 0 && itr->pkey.length() == 0) {
      _accounts.erase(itr);
   }
}

void dataexchange::regpkey(account_name owner, string pkey) {
   require_auth( owner );

   pkey.erase(pkey.begin(), find_if(pkey.begin(), pkey.end(), [](int ch) {
       return !isspace(ch);
   }));
   pkey.erase(find_if(pkey.rbegin(), pkey.rend(), [](int ch) {
       return !isspace(ch);
   }).base(), pkey.end());

   eosio_assert(pkey.length() == 53, "Length of public key should be 53");
   string pubkey_prefix("EOS");
   auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), pkey.begin());
   eosio_assert(result.first == pubkey_prefix.end(), "Public key should be prefix with EOS");

   auto base58substr = pkey.substr(pubkey_prefix.length());
   vector<unsigned char> vch;
   //(fixme)decode_base58 can be very time-consuming, must remove it in the future.
   eosio_assert(decode_base58(base58substr, vch), "Decode public failed");
   eosio_assert(vch.size() == 37, "Invalid public key: invalid base58 length");

   array<unsigned char,33> pubkey_data;
   copy_n(vch.begin(), 33, pubkey_data.begin());

   checksum160 check_pubkey;
   ripemd160(reinterpret_cast<char *>(pubkey_data.data()), 33, &check_pubkey);
   eosio_assert(memcmp(&check_pubkey.hash, &vch.end()[-4], 4) == 0, "Invalid public key: invalid checksum");

   auto itr = _accounts.find( owner );
   if( itr == _accounts.end() ) {
      itr = _accounts.emplace(_self, [&](auto& acnt){
         acnt.owner = owner;
      });
   }

   _accounts.modify( itr, 0, [&]( auto& acnt ) {
      acnt.pkey = pkey;
   });
}

void dataexchange::deregpkey(account_name owner) {
   require_auth( owner );

   auto itr = _accounts.find( owner );
   eosio_assert(itr != _accounts.end(), "account not registered yet");

   if (itr->asset_balance.amount > 0) {
       _accounts.modify( itr, 0, [&]( auto& acnt ) {
          acnt.pkey = "";
       });
   } else {
         _accounts.erase(itr);
   }
}

