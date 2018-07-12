# how to run
## build and deploy
build the cpp code and deploy it to blockchain:  
```
sh build.sh data dataexchange
```
## create markets by script
```
python testCreate1000Markets.py
```

## verify the results
```
cleos get table data data datamarkets -l -1
```
