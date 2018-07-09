#!/bin/bash

cleos --wallet-url=http://127.0.0.1:8889 wallet unlock --password=PW5JDUhcbjYFmeW6yYAgpVRZZBH3jRcPr7VWAmcj5Puqaapzutjd8
if [[ $# -ne 2 ]]; then
    echo "USAGE: build.sh <ACCOUNT NAME> <Contract Name> from within the directory"
    exit 1
fi

ACCOUNT=$1
CONTRACT=$2

eosiocpp -o ${CONTRACT}.wast ${CONTRACT}.cpp &&
eosiocpp -g ${CONTRACT}.abi ${CONTRACT}.cpp &&
cleos --wallet-url=http://127.0.0.1:8889 create account eosio ${ACCOUNT} EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos --wallet-url=http://127.0.0.1:8889 set contract ${ACCOUNT} ../${CONTRACT}
