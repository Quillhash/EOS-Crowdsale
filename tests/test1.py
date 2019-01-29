import sys
from eosfactory.eosf import *
verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.DEBUG])

CONTRACT_WORKSPACE = sys.path[0] + "/../"
TOKEN_CONTRACT_WORKSPACE = sys.path[0] + "/../../quilltoken/"

def test():
    SCENARIO('''
    Execute simple actions.
    ''')
    reset()
    create_master_account("master")

    #######################################################################################################
    # accounts where the smart contracts will be hosted
    create_account("host", master)
    create_account("token", master, account_name="eosio.token")

    COMMENT('''
    Build and deploy token contract:
    ''')

    # creating token contract
    token_contract = Contract(token, TOKEN_CONTRACT_WORKSPACE)
    token_contract.build(force=True)
    token_contract.deploy()

    ########################################################################################################
    COMMENT('''
    Build and deploy crowdsale contract:
    ''')

    # creating crowdsale contract
    contract = Contract(host, CONTRACT_WORKSPACE)
    contract.build(force=True)
    contract.deploy()

    ########################################################################################################
    COMMENT('''
    Create test accounts:
    ''')

    create_account("issuer", master)
    create_account("alice", master)
    create_account("carol", master)

    ########################################################################################################
    COMMENT('''
    Create EOS tokens 
    ''')

    token.push_action(
        "create",
        {
            "issuer": token,
            "maximum_supply": "1000000000.0000 SYS"
        },
        [token]
    )

    ########################################################################################################
    # COMMENT('''
    # Create QUILL tokens 
    # ''')

    # token.push_action(
    #     "create",
    #     {
    #         "issuer": issuer,
    #         "maximum_supply": "1000000000 QUILL"
    #     },
    #     [token]
    # )

    ########################################################################################################
    COMMENT('''
    Issue SYS tokens to the sub accounts 
    ''')

    # give tokens to alice
    token.push_action(
        "issue",
        {
            "to": alice,
            "quantity": "10000.0000 SYS",
            "memo": "issued tokens to alice"
        },
        [token]
    )

    # give tokens to carol
    token.push_action(
        "issue",
        {
            "to": carol,
            "quantity": "10000.0000 SYS",
            "memo": "issued tokens to carol"
        },
        [token]
    )

    ########################################################################################################
    COMMENT('''
    Initialize the crowdsale
    ''')

    host.push_action(
        "init",
        {
            "issuer": issuer,
            "start": "2006-01-01T00:00:00",
            "finish": "2020-04-20T00:00:00"
        },
        [host]
    )

    ########################################################################################################
    COMMENT('''
    Invest in the crowdsale 
    ''')

    # set eosio.code permission to the contract
    host.set_account_permission(
        Permission.ACTIVE,
        {
            "threshold": 1,
            "keys": [
                {
                    "key": host.active(),
                    "weight": 1
                }
            ],
            "accounts":
            [
                {
                    "permission":
                        {
                            "actor": host,
                            "permission": "eosio.code"
                        },
                    "weight": 1
                }
            ]
        },
        Permission.OWNER,
        (host, Permission.OWNER)
    )

    # transfer EOS tokens from alice to the host (contract) accounts
    token.push_action(
        "transfer",
        {
            "from": alice,
            "to": host,
            "quantity": "10.0000 SYS",
            "memo": "Invested 10 SYS in crowdsale"
        },
        permission=(alice, Permission.ACTIVE)
    )

    # deposit some funds
    # host.push_action(
    #     "invest",
    #     {
    #         "investor": alice,
    #         "quantity": "10000 EOS"
    #     },
    #     permission=(alice, Permission.ACTIVE)
    # )

    # assert("host" in DEBUG())

    # host.push_action(
    #     "testaction",
    #     {
    #         "sender": alice
    #     },
    #     permission=(alice, Permission.ACTIVE)
    # )

    stop()

if __name__ == "__main__":
    test()
