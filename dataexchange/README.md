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
