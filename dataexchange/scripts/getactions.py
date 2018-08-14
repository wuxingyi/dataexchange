#!/usr/bin/python
import pymongo
myclient = pymongo.MongoClient("mongodb://localhost:27017/")
# the db we used when start nodeos daemon
dexdb = myclient["dataexchange"]
# we crawler the "actions" collection and process the data
actions_col = dexdb["actions"]
for x in actions_col.find({"account": "dex"}):
  print(x["name"], x["data"])
