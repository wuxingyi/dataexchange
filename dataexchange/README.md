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

## verify the results
```
cleos get table dddd dddd datamarkets -l -1
```
