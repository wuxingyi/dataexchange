alias cleos='docker exec 9a6b940339c6 /opt/eosio/bin/cleos -u http://localhost:8888 --wallet-url http://localhost:8900'

# 0.create accounts
cleos wallet unlock --password=PW5JjCX7zZr35TcdUzCqzJAcb4cQcvKTTEutHCs9yoNLwiLaLKmNA
docker exec 9a6b940339c6 bash -c 'cd /dataexchange/dataexchange/dataexchange ; sh build.sh dex dataexchange'
cleos create account eosio datasource1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
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


# 1.xingyitoken is used for token transfer
cleos create account eosio xingyitoken EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos set contract xingyitoken /contracts/eosio.token
cleos push action xingyitoken create '[ "xingyitoken", "1000000000.0000 SYS", 0, 0, 0]' -p xingyitoken
cleos push action xingyitoken issue '[ "buyer1", "10000000.0000 SYS", "buyer1" ]' -p xingyitoken
cleos push action xingyitoken issue '[ "buyer2", "10000000.0000 SYS", "buyer1" ]' -p xingyitoken
cleos get currency balance xingyitoken buyer1
cleos get currency balance xingyitoken buyer2

# 2.two buyers all deposit their COIN to the contract
cleos push action dex deposit '[ "buyer1", "100.0000 SYS" ]' -p buyer1
cleos push action dex deposit '[ "buyer2", "100.0000 SYS" ]' -p buyer2
cleos get currency balance xingyitoken buyer1
cleos get currency balance xingyitoken buyer2
cleos get currency balance xingyitoken dex

# 3.create market
cleos push action dex createmarket ' {"owner": "datasource1", "desp": "datasource", "type": 2} ' -p dex

# 4.seller1 and seller2 both create asking order
cleos push action dex createorder ' {"seller": "seller1", "marketid": 0, "price": "1.0000 SYS", "dataforsell": "aasdf"} ' -p seller1
cleos push action dex createorder ' {"seller": "seller2", "marketid": 0, "price": "2.0000 SYS", "dataforsell": "aasdf"} ' -p seller2
cleos push action dex createorder ' {"seller": "seller3", "marketid": 0, "price": "3.0000 SYS", "dataforsell": "aasdf"} ' -p seller3
cleos get table dex datasource1 askingorders


# 5.fill the asking orders
cleos push action dex fillorder ' {"buyer": "buyer1", "owner": "datasource1", "orderid": 1} ' -p buyer1
cleos push action dex fillorder ' {"buyer": "buyer1", "owner": "datasource1", "orderid": 2} ' -p buyer1
cleos push action dex fillorder ' {"buyer": "buyer2", "owner": "datasource1", "orderid": 3} ' -p buyer2
cleos get table dex datasource1 askingorders
cleos get table dex dex accounts


# 6.sellers withdraw their tokens
cleos push action dex withdraw '[ "seller1", "1.0000 SYS" ]' -p seller1
cleos push action dex withdraw '[ "seller2", "2.0000 SYS" ]' -p seller2
cleos push action dex withdraw '[ "seller3", "3.0000 SYS" ]' -p seller3
cleos get currency balance xingyitoken seller1
cleos get currency balance xingyitoken seller2
cleos get currency balance xingyitoken seller3
cleos get table dex dex accounts

# 7.buyers withdraw their tokens
#each buyer buy data for 3 SYS, so left only 97 each
cleos push action dex withdraw '[ "buyer1", "97.0000 SYS" ]' -p buyer1
cleos push action dex withdraw '[ "buyer2", "97.0000 SYS" ]' -p buyer2
# now there are no account stored, all memory freed
cleos get table dex dex accounts

# 8.sellers reg and dereg keys
cleos push action dex regpkey '[ "seller1", "asdfasdfasdfasdfasdfa" ]' -p seller1
cleos push action dex regpkey '[ "seller2", "eoseoseoseoseoseoseos" ]' -p seller2
cleos push action dex deposit '[ "seller1", "1.0000 SYS" ]' -p seller1
cleos push action dex deposit '[ "seller2", "1.0000 SYS" ]' -p seller2
cleos get table dex dex accounts
cleos push action dex deregpkey '[ "seller1" ]' -p seller1
cleos push action dex deregpkey '[ "seller2" ]' -p seller2
cleos push action dex withdraw '[ "seller1", "1.0000 SYS" ]' -p seller1
cleos push action dex withdraw '[ "seller2", "1.0000 SYS" ]' -p seller2
cleos get table dex dex accounts