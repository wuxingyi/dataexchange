#!/bin/bash

cleos wallet unlock --password=PW5JjCX7zZr35TcdUzCqzJAcb4cQcvKTTEutHCs9yoNLwiLaLKmNA
if [[ $# -ne 2 ]]; then
    echo "USAGE: build.sh <ACCOUNT NAME> <Contract Name> from within the directory"
    exit 1
fi

ACCOUNT=$1
CONTRACT=$2

eosiocpp -o ${CONTRACT}.wast ${CONTRACT}.cpp &&
eosiocpp -g ${CONTRACT}.abi ${CONTRACT}.cpp &&
cleos create account eosio ${ACCOUNT} EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos set contract ${ACCOUNT} ../${CONTRACT}
