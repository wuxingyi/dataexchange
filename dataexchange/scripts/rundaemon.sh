nodeos -e -p eosio --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --contracts-console --http-server-address 0.0.0.0:8888
keosd --http-server-address=127.0.0.1:8900

cleos wallet unlock --password=PW5JjCX7zZr35TcdUzCqzJAcb4cQcvKTTEutHCs9yoNLwiLaLKmNA

cleos wallet import 5JhzmSuFirrQZq3YoeiExQFh5wrfZtbapUmzvR2oASfoEur1oBu
cleos wallet import 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
