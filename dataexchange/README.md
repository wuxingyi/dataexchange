# how to run
## build and deploy
build the cpp code and deploy it to blockchain:  
```
sh build.sh dddd dataexchange
```
## create markets by script
```
python testCreate100Markets.py
```
note that only the contract owner can create a market.

## verify the results
```
cleos get table dddd dddd datamarkets -l -1
```
## remove market
removing market need both owner's permission and the marketid.
note that only the market owner can remove a market.

```
cleos push action dddd removemarket '["bdibxtljzc",104]' -p bdibxtljzc
```

## create a order with a giving marketid:
```
cleos push action dddd createorder '["eosio","bdibxtljzc", 0, 2,"jd.com"]' -p eosio
```

## remove previously created order
```
cleos push action dddd removeorder '["eosio","bdibxtljzc",0]' -p eosio
```
## list opening orders in a market
```
➜  dataexchange git:(master) ✗ cleos get table dddd asdf marketorders -l -1
{
  "rows": [{
      "orderid": 1,
      "marketid": 0,
      "seller": "asdfasdfasdf",
      "price": 1,
      "dataforsell": "aasdf"
    },{
      "orderid": 2,
      "marketid": 0,
      "seller": "asdfasdfasdf",
      "price": 1,
      "dataforsell": "aasdf"
    },{
      "orderid": 3,
      "marketid": 0,
      "seller": "asdfasdfasdf",
      "price": 1,
      "dataforsell": "aasdf"
    }
  ],
  "more": false
}
```
# order and deal design
orders can be filled multiple times by calling ```makedeal``` abi, each time ```makedeal``` is called, a new deal is created with an unique ```dealid```.  
if an order is canceled by the seller, no more deals can be made anymore, but pending deals can be continue to finish.  
anyone can remove finished deal data by calling ```erasedeal``` to reduce memory usage, this is very important for a dapp with large amount of active users.   

# bidirection order system
a user can put an order to the market, in current implemention there are two kinds of orders, which are **ask** order and **bid** order.
we can the user proposed the order as a **maker**, and the user(s) calling the ```makedeal``` abi as a **taker**.   
In a **ask** order, the maker is the seller, and the opposite side (aka **taker**) is the buyers;  
In a **bid** order, the maker is the buyer, and the opposite side (aka **taker**) is the sellers.  
In both **ask** and **bid** order, it is the **maker's** right to  ```authorize``` a deal before the data source actually upload data hash.  

# deal state machine
previously the deal is finished if the data source upload the ipfs hash to blockchain,   
but actually the buy should confirm that the hash is valid and the hash is exist in the ipfs network,  
also, the file should carefully encoded by the following formula:  

```Buyer's pubickey(seller_accountname + shared_secret(raw data))```

after the buyer download the ipfs file, he should first decode it by the private key registered to the blockchain,   
then check whether seller_accountname is the correct seller, if so, he can call confirmhash to put the state machine to the next state.

