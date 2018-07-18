import string
import sys
import time
import os
import random
import json
import commands


new_words = []
def make_new_words(count):
    for i in range(count):
        w = ''.join(random.choice(string.ascii_letters).lower() for m in range(10))
        new_words.append(w)


def createAccounts():
    for i in new_words:
        r = os.system('cleos create account eosio %s EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV' % i)

def Create100Markets():
    for i in new_words:
        s={'owner': 'asdf', 'type': 2, 'desp':'taobao.com'}
        s['owner'] = i
        s2 = json.dumps(s)
        ss = "cleos push action dddd createmarket " +  "' " + s2 + " '" + " -p dddd" 
        os.system(ss)



if __name__ == "__main__":
    make_new_words(100)
    createAccounts()
    Create100Markets()
