#!/bin/bash

cleos wallet unlock --password=PW5KXQqYAfSGsFkb1wdB9hhxcrjt2XbbncDJA3u6JAiPe4jZsfubh
if [[ $# -ne 1 ]]; then
    echo "USAGE: build.sh <Contract Name> from within the directory"
    exit 1
fi

CONTRACT=$1

eosiocpp -o ${CONTRACT}.wast ${CONTRACT}.cpp && eosiocpp -g ${CONTRACT}.abi ${CONTRACT}.cpp 
