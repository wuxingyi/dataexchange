#!/usr/bin/python
import pycurl
import urllib
import StringIO
import json

import pycurl
import urllib
import StringIO
import json

url='http://127.0.0.1:8888/v1/chain/get_table_rows'
def getdeal():
    c = pycurl.Curl()
    postdic={"code": "dex","scope":"dex", "table":"deals", "json": "true"}
    poststr= json.dumps(postdic)
    c.setopt(pycurl.URL, url)
    c.setopt(c.POSTFIELDS, poststr)
    storage = StringIO.StringIO()
    c.setopt(c.WRITEFUNCTION, storage.write)
    c.perform()
    c.close()
    contents = storage.getvalue()
    s=json.loads(contents)
    alldeals = s['rows']
    for deal in alldeals:
        if deal['dealid'] == 2:
            if deal["ordertype"] == 1:
                print "buyer is %s, seller is %s, data source is %s " % (deal["taker"], deal["maker"], deal["marketowner"])
            elif deal["ordertype"] == 2:
                print "buyer is %s, seller is %s, data source is %s " % (deal["maker"], deal["taker"], deal["marketowner"])
            break;

def getonedeal(dealid):
    c = pycurl.Curl()
    postdic={"code": "dex","scope":"dex", "table":"deals", "json": "true", "lower_bound": dealid, "upper_bound": dealid+1}
    poststr= json.dumps(postdic)
    c.setopt(pycurl.URL, url)
    c.setopt(c.POSTFIELDS, poststr)
    storage = StringIO.StringIO()
    c.setopt(c.WRITEFUNCTION, storage.write)
    c.perform()
    c.close()
    contents = storage.getvalue()
    s=json.loads(contents)
    alldeals = s['rows']
    if len(alldeals) < 1:
        print("no such deal")
        exit(-1)
    deal = alldeals[0]
    if deal["ordertype"] == 1:
        print "buyer is %s, seller is %s, data source is %s " % (deal["taker"], deal["maker"], deal["marketowner"])
    elif deal["ordertype"] == 2:
        print "buyer is %s, seller is %s, data source is %s " % (deal["maker"], deal["taker"], deal["marketowner"])

if __name__ == "__main__":
    getdeal()
    getonedeal(2)
