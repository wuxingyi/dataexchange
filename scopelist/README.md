# list by scope: an example contract
## our goal
store data according to scope, so we can list the data by scope.  

## contract
according to the following abi:  
```
void selltomarket(account_name seller, account_name marketowner, uint64_t price) 
```
all the order's are sorted by the scope of the marketowner, so we can get all the orders from the marketowner's scope.  
you can simply get the order list by the following command line:  
```
cleos get table scopelist eosio orders
```

## run and verify
### compile and deploy the contract:
we use docker to build smart contract here, you can also build it by ```sh build.sh scopelist scopelist```:  
```
docker exec 9a6b940339c6 bash -c 'cd /dataexchange/dataexchange/scopelist ; sh build.sh scopelist scopelist'
```
### push the order to the scope:
```
for i in `seq 1 10`
do 
  cleos push action scopelist selltomarket ' {"seller": "eosio", "marketowner": "wxy", "price":2} ' -p eosio
  cleos push action scopelist selltomarket ' {"seller": "eosio", "marketowner": "asdf", "price":2} ' -p eosio
done
```
### check the result
we can see that all orders belongs to marketowner wxy and asdf:
```
➜  scopelsit git:(master) ✗ cleos get table scopelist  wxy orders -l -1
{
  "rows": [{
      "orderid": 0,
      "price": 2
    },{
      "orderid": 1,
      "price": 2
    },{
      "orderid": 2,
      "price": 2
    },{
      "orderid": 3,
      "price": 2
    },{
      "orderid": 4,
      "price": 2
    },{
      "orderid": 5,
      "price": 2
    },{
      "orderid": 6,
      "price": 2
    },{
      "orderid": 7,
      "price": 2
    },{
      "orderid": 8,
      "price": 2
    },{
      "orderid": 9,
      "price": 2
    }
  ],
  "more": false
}

➜  scopelsit git:(master) ✗ cleos get table scopelist  asdf orders -l -1
{
  "rows": [{
      "orderid": 0,
      "price": 2
    },{
      "orderid": 1,
      "price": 2
    },{
      "orderid": 2,
      "price": 2
    },{
      "orderid": 3,
      "price": 2
    },{
      "orderid": 4,
      "price": 2
    },{
      "orderid": 5,
      "price": 2
    },{
      "orderid": 6,
      "price": 2
    },{
      "orderid": 7,
      "price": 2
    },{
      "orderid": 8,
      "price": 2
    },{
      "orderid": 9,
      "price": 2
    }
  ],
  "more": false
}
```

