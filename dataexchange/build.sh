#!/bin/bash

cleos wallet unlock --password=PW5KXQqYAfSGsFkb1wdB9hhxcrjt2XbbncDJA3u6JAiPe4jZsfubh
if [[ $# -ne 2 ]]; then
    echo "USAGE: build.sh <ACCOUNT NAME> <Contract Name> from within the directory"
    exit 1
fi

ACCOUNT=$1
CONTRACT=$2

eosiocpp -o ${CONTRACT}.wast ${CONTRACT}.cpp && eosiocpp -g ${CONTRACT}.abi ${CONTRACT}.cpp 