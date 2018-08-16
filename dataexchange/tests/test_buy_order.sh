container=df762864e6f8
alias cleos="docker exec $container /opt/eosio/bin/cleos -u http://127.0.0.1:8888 --wallet-url http://127.0.0.1:8900"

# 1: restart daemons, build/deploy contract and create accounts"
function step_1() {
    echo "STEP 1: restart daemons, build/deploy contract and create accounts, issue tokens"
    docker exec $container bash -c 'killall -9 nodeos keosd mongod'
    docker exec $container bash -c 'rm -rf /dataexchange/local/mongodbdb/db/*'
    docker exec $container bash -c 'rm -rf /dataexchange/local/share/eosio/nodeos/data /dataexchange/local/share/eosio/nodeos/*.log'
    docker exec $container bash -c 'mongod &>> /dataexchange/local/share/eosio/nodeos/mongod.log & '
    docker exec $container bash -c 'keosd --http-server-address=127.0.0.1:8900 &>> keosd.log &'
    docker exec $container bash -c 'cd /dataexchange/dataexchange/dataexchange/contracts/dataexchange ; sh build.sh dataexchange'
    # we sleep twice because staring mongod is time-consuming, cost nearly 20 seconds.
    sleep 10
    docker exec $container bash -c 'nodeos -e -p eosio --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --contracts-console --http-server-address 0.0.0.0:8888 --plugin eosio::mongo_db_plugin --mongodb-uri mongodb://127.0.0.1:27017/dataexchange &>> /dataexchange/local/share/eosio/nodeos/nodeos.log &'
    sleep 5
    cleos wallet unlock --password=PW5KXQqYAfSGsFkb1wdB9hhxcrjt2XbbncDJA3u6JAiPe4jZsfubh
    cleos create account eosio dex EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos set contract dex /dataexchange/dataexchange/dataexchange/contracts/dataexchange
    cleos create account eosio datasource1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio datasource2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio seller1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio seller2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio seller3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio buyer1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos create account eosio buyer2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos set account permission buyer1 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"dex","permission":"eosio.code"},"weight":1}]}' owner -p buyer1
    cleos set account permission buyer2 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"dex","permission":"eosio.code"},"weight":1}]}' owner -p buyer2
    cleos set account permission seller1 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"dex","permission":"eosio.code"},"weight":1}]}' owner -p seller1
    cleos set account permission seller2 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"dex","permission":"eosio.code"},"weight":1}]}' owner -p seller2
    cleos set account permission dex active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"dex","permission":"eosio.code"},"weight":1}]}' owner -p dex

    cleos create account eosio xingyitoken EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
    cleos set contract xingyitoken /contracts/eosio.token
    cleos push action xingyitoken create '[ "xingyitoken", "1000000000.0000 SYS", 0, 0, 0]' -p xingyitoken
    cleos push action xingyitoken issue '[ "buyer1", "10000000.0000 SYS", "buyer1" ]' -p xingyitoken
    cleos push action xingyitoken issue '[ "buyer2", "10000000.0000 SYS", "buyer1" ]' -p xingyitoken
    cleos get currency balance xingyitoken buyer1
    cleos get currency balance xingyitoken buyer2
}

# 2.two buyers all deposit their COIN to the contract
function step_2() {
    echo "STEP 2: deposit some coins to contract"
    # deposit twice for buyer1 because sometimes this action may failed by throwing a exception"transaction takes to long""
    cleos push action dex deposit '[ "buyer1", "1000.0000 SYS" ]' -p buyer1
    cleos push action dex deposit '[ "buyer1", "1000.0000 SYS" ]' -p buyer1
    cleos push action dex deposit '[ "buyer2", "1000.0000 SYS" ]' -p buyer2
    cleos get currency balance xingyitoken buyer1
    cleos get currency balance xingyitoken buyer2
    cleos get currency balance xingyitoken dex
}

# 3.create market
function step_3() {
    echo "STEP 3: create market"
    cleos push action dex createmarket ' {"owner": "datasource1", "desp": "datasource1", "type": 2} ' -p dex
    cleos push action dex createmarket ' {"owner": "datasource2", "desp": "datasource2", "type": 2} ' -p dex
}

# 4.buyer1 buyer2 all create biding order
function step_4() {
    echo "STEP 4: buyer1 buyer2 create biding order"
    cleos push action dex createorder ' {"orderowner": "buyer1", "ordertype": 2, "marketid": 0, "price": "10.0000 SYS"} ' -p buyer1 
    cleos push action dex createorder ' {"orderowner": "buyer2", "ordertype": 2, "marketid": 0, "price": "20.0000 SYS"} ' -p buyer2
    cleos get table dex datasource1 marketorders
}

# 5 try make deals
function step_5() {
    echo "STEP 5: make deal"
    cleos push action dex makedeal ' {"taker": "seller1", "marketowner": "datasource1", "orderid": 1} ' -p seller1
    cleos push action dex makedeal ' {"taker": "seller2", "marketowner": "datasource1", "orderid": 2} ' -p seller2
    cleos get table dex datasource1 marketorders
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 6 authorize the deal
function step_6() {
    echo "STEP 6: authorize the deal deal"
    cleos push action dex authorize ' {"maker": "buyer1", "dealid": 1} ' -p buyer1
    cleos push action dex authorize ' {"maker": "buyer2", "dealid": 2} ' -p buyer2
    cleos get table dex datasource1 marketorders
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 7 test suspendorder and resumeorder
function step_7() {
    echo "STEP 7: test suspendorder and resumeorder"
    cleos push action dex suspendorder '{"orderowner": "buyer1", "marketowner":"datasource1", "orderid": 1} ' -p buyer1
    echo "abi makedeal SHOULD FAIL: order is suspended"
    cleos push action dex makedeal ' {"taker": "seller1", "marketowner": "datasource1", "orderid": 1} ' -p seller1
    cleos push action dex resumeorder '{"orderowner": "buyer1", "marketowner":"datasource1", "orderid": 1} ' -p buyer1
    echo "abi makedeal SHOULD SUCCESS: order is resumed"
    cleos push action dex makedeal ' {"taker": "seller1", "marketowner": "datasource1", "orderid": 1} ' -p seller1
    cleos get table dex datasource1 marketorders
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 8 seller and datasource negociate secret key
function step_8() {
    echo "STEP 8: seller and datasource negociate secret key"
    # p=ffffffffffffffc5, g=5, A=98128406754500292, a=8610715357444524147, B=12526033386409324239, b=15949919193124526316, s=7617149221468993112
    cleos push action dex uploadpuba ' ["seller1", 1, 98128406754500292] ' -p seller1
    cleos push action dex uploadpuba ' ["seller2", 2, 98128406754500292] ' -p seller2
    cleos push action dex uploadpubb ' ["datasource1", 1, 12526033386409324239] ' -p datasource1
    cleos push action dex uploadpubb ' ["datasource1", 2, 12526033386409324239] ' -p datasource1
    cleos get table dex datasource1 marketorders
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 9.seller upload datahash(calling uploadhash)
function step_9() {
    echo "STEP 9: send datahash"
    cleos push action dex uploadhash '["datasource1",  1, "asdfasdf"]' -p datasource1
    cleos push action dex uploadhash '["datasource1",  2, "asdfasf" ]' -p datasource1
    cleos push action dex uploadhash '["seller1",  1, "www"]' -p seller1
    cleos push action dex uploadhash '["seller2",  2, "test" ]' -p seller2

    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 10.buyer confirm the datahash(calling comfirmhash)
function step_10() {
    echo "STEP 10: buyer comfirm the datahash"
    # p=ffffffffffffffc5, g=5, A=98128406754500292, a=8610715357444524147, B=12526033386409324239, b=15949919193124526316, s=7617149221468993112
    cleos push action dex confirmhash '[ "buyer1", 1]' -p buyer1 
    cleos push action dex confirmhash '[ "buyer2", 2]' -p buyer2
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 11.seller upload private a
function step_11() {
    echo "STEP 11: seller upload private a"
    # p=ffffffffffffffc5, g=5, A=98128406754500292, a=8610715357444524147, B=12526033386409324239, b=15949919193124526316, s=7617149221468993112
    # put a wrong one
    cleos push action dex uploadpria '[ "seller1", 1, 9 ]' -p seller1
    # put a correct one
    cleos push action dex uploadpria '[ "seller2", 2, 8610715357444524147 ]' -p seller2
    cleos get table dex dex accounts
    cleos get table dex dex deals
}

# 12.datasource upload private b
function step_12() {
    echo "STEP 12: datasource upload private b"
    # p=23, g=5, A=8, a=6, B=19, b=15, s=2
    cleos push action dex uploadprib '[0, 1, 15949919193124526316 ]' -p datasource1 
    cleos push action dex uploadprib '[0, 2, 15949919193124526316 ]' -p datasource1 
    cleos get table dex dex accounts
    cleos get table dex dex deals
}


# 13.sellers withdraw their tokens
function step_13() {
    echo "STEP 13: sellers withdraw their tokens"
    echo "abi withdraw SHOULD FAIL: no tokens earned because of wrong secret"
    cleos push action dex withdraw '[ "seller1", "9.0000 SYS" ]' -p seller1
    cleos push action dex withdraw '[ "seller2", "18.0000 SYS" ]' -p seller2
    cleos get currency balance xingyitoken seller1
    cleos get currency balance xingyitoken seller2
    cleos get table dex dex accounts
}

# 14.datasource withdraw token
function step_14() {
    echo "STEP 14: datasource withdraw their tokens"
    cleos push action dex withdraw '[ "datasource1", "2.0000 SYS" ]' -p datasource1
    cleos get currency balance xingyitoken datasource1
    cleos get table dex dex accounts
}


# 15.sellers erase finished deal to reduce memory usage
function step_15() {
    echo "STEP 15: sellers erase deal to reduce memory usage"
    cleos push action dex erasedeal '[1]' -p seller1
    cleos push action dex erasedeal '[2]' -p seller2
    cleos get table dex datasource1 marketorders
}

# 16.buyer cancel deal 
function step_16() {
    echo "STEP 16: buyer cancel deal and get refund"
    cleos push action dex makedeal ' {"taker": "seller1", "marketowner": "datasource1", "orderid": 1} ' -p seller1
    cleos get table dex dex accounts
    cleos push action dex canceldeal ' ["seller1", "datasource1", 4]' -p seller1
    cleos get table dex dex accounts
}

# 17.test market suspend、resume、remove and deal expireation
function step_17() {
    echo "STEP 17: test market suspend, resume and remove and deal expiration"
    cleos push action dex createorder ' {"orderowner": "buyer1", "ordertype": 2, "marketid": 1, "price": "10.0000 SYS"} ' -p buyer1
    cleos push action dex suspendmkt ' {"owner": "datasource2", "marketid": 1} ' -p datasource2
    cleos push action dex removeorder ' {"orderowner": "buyer1", "marketowner":"datasource2", "orderid": 3} ' -p buyer1

    echo "abi creatorder SHOULD FAIL: creating order on a supsended market"
    cleos push action dex createorder ' {"orderowner": "buyer1", "ordertype": 2, "marketid": 1, "price": "10.0000 SYS"} ' -p buyer1
    cleos push action dex resumemkt ' {"owner": "datasource2", "marketid": 1} ' -p datasource2
    #should suscess
    cleos push action dex createorder ' {"orderowner": "buyer1", "ordertype": 2, "marketid": 1, "price": "10.0000 SYS"} ' -p buyer1
    cleos push action dex suspendmkt ' {"owner": "datasource2", "marketid": 1} ' -p datasource2
    cleos push action dex suspendmkt ' {"owner": "datasource1", "marketid": 0} ' -p datasource1
    sleep 6
    cleos push action dex removemarket ' {"owner": "datasource2", "marketid": 1} ' -p datasource2

    echo "abi removemarket SHOULD FAIL: exist a inflight deal"
    cleos push action dex removemarket ' {"owner": "datasource1", "marketid": 0} ' -p datasource1

    sleep 5
    echo "abi authorize SHOULD FAIL: because it's expired"
    cleos push action dex authorize ' {"maker": "buyer1", "dealid": 3} ' -p buyer1
    cleos push action dex erasedeal '[3]' -p buyer1
    cleos push action dex removemarket ' {"owner": "datasource1", "marketid": 0} ' -p datasource1
    cleos get table dex dex datamarkets -l -1
}

if [[ $# -ne 1 ]]; then 
    echo "usage: ./tokentest.sh step_number"
    exit -1
fi

for i in `seq 1 $1`
do 
   step_$i
done
