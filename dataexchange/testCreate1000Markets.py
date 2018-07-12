import string
import sys
import time
import os
import random
import json
import commands

def Create1000Markets():
    for i in range(1, 1000):
        s={'marketname': 100}
        s['marketname'] = i
        s2 = json.dumps(s)
        ss = "cleos --wallet-url=http://127.0.0.1:8889 push action data createmarket " +  "' " + s2 + " '" + " -p data"
        os.system(ss)



if __name__ == "__main__":
    Create1000Markets()
