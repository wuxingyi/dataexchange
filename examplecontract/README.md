# DECENTRLIZAED DATA EXCHANGE: smart contracts

## how to build and run
run the build scrip like the following command line:  
```
sh build.sh example examplecontract
```

then the smart contract will be deployed on the account named ```datadmin```(account will be created if not exists).  
you can use ```test``` function to add entries to the ledger and use ```erase``` or ```wipeall``` to clear entry or all entries.  


```
sh build.sh example examplecontract
remote➜  examplecontract git:(master) ✗ sh build.sh example examplecontract
Error 3120007: Already unlocked
1553290ms thread-0   abi_generator.hpp:68          ricardian_contracts  ] Warning, no ricardian clauses found for examplecontract

1553291ms thread-0   abi_generator.hpp:75          ricardian_contracts  ] Warning, no ricardian contract found for test

1553291ms thread-0   abi_generator.hpp:75          ricardian_contracts  ] Warning, no ricardian contract found for wipeall

1553291ms thread-0   abi_generator.hpp:75          ricardian_contracts  ] Warning, no ricardian contract found for get

1553291ms thread-0   abi_generator.hpp:75          ricardian_contracts  ] Warning, no ricardian contract found for erase

1553291ms thread-0   abi_generator.hpp:75          ricardian_contracts  ] Warning, no ricardian contract found for transfer

Generated examplecontract.abi ...
executed transaction: 44fae00162a27ae50f4c19ff5453765f06d75cd580c9af0ce82526686e0c6192  200 bytes  774 us
#         eosio <= eosio::newaccount            {"creator":"eosio","name":"example","owner":{"threshold":1,"keys":[{"key":"EOS6MRyAjQq8ud7hVNYcfnVPJ...
warning: transaction executed locally, but may not be confirmed by the network yet
Reading WAST/WASM from ../examplecontract/examplecontract.wasm...
Using already assembled WASM...
Publishing contract...
executed transaction: 91a73b718fa59c83c76a17e7a7536a664b34ba4a21c9753685d0ad6e8d98e269  5840 bytes  933 us
#         eosio <= eosio::setcode               {"account":"example","vmtype":0,"vmversion":0,"code":"0061736d01000000017b1460027f7e0060057f7e7e7f7f...
#         eosio <= eosio::setabi                {"account":"example","abi":"0e656f73696f3a3a6162692f312e30000608737461747573657300020673656e64657204...
warning: transaction executed locally, but may not be confirmed by the network yet

remote➜  examplecontract git:(master) ✗ cleos push action example test '["eosio", "testasdf"]' -p eosio
executed transaction: 83a67b780b48fa9c33d82fe7e8a9c532fe4e987469361dbc0089ef99d32f4b6c  112 bytes  655 us
#       example <= example::test                {"sender":"eosio","status":"testasdf"}
warning: transaction executed locally, but may not be confirmed by the network yet

remote➜  examplecontract git:(master) ✗ cleos push action example test '["wuxingyiwar3", "testasdf"]' -p wuxingyiwar3
executed transaction: 5cf723043a0831fd2bb7549e7b3dad7bce36657364f66143aa5b262c83cc3659  112 bytes  649 us
#       example <= example::test                {"sender":"wuxingyiwar3","status":"testasdf"}

remote➜  examplecontract git:(master) ✗ cleos push action example test '["example", "testasdf"]' -p example
executed transaction: 63aeaa0a96aeedb03b1ab522e2150850953ce4c2a078ceb73234f211345b85d1  112 bytes  626 us
#       example <= example::test                {"sender":"example","status":"testasdf"}

remote➜  examplecontract git:(master) ✗ cleos get table example example statuses
{
  "rows": [{
      "sender": "eosio",
      "status": "testasdf"
    },{
      "sender": "example",
      "status": "testasdf"
    },{
      "sender": "wuxingyiwar3",
      "status": "testasdf"
    }
  ],
  "more": false
}

# erase esoio
remote➜  examplecontract git:(master) ✗ cleos push action example erase '["eosio"]' -p eosio
executed transaction: b59fb57dcdb39e0c482e5c1f7043e74896ae5ec8a78124ce7e54f1a9794105c5  104 bytes  633 us
#       example <= example::erase               {"sender":"eosio"}
warning: transaction executed locally, but may not be confirmed by the network yet
remote➜  examplecontract git:(master) ✗ cleos get table example example statuses
{
  "rows": [{
      "sender": "example",
      "status": "testasdf"
    },{
      "sender": "wuxingyiwar3",
      "status": "testasdf"
    }
  ],
  "more": false
}

```
