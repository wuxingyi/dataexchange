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
cleos push action dddd cancelorder '["eosio","bdibxtljzc",0]' -p eosio
```
## list opening orders in a market
```
➜  dataexchange git:(master) ✗ cleos get table dddd asdf askingorders -l -1
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
anyone can remove finished deal data by calling ```erasedeal``` to reduce memory usage, this is very important for a dapp with much active users.   
