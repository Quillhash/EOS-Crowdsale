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
    create_account("token_host", master, account_name="quillhash111")

    COMMENT('''
    Build and deploy token contract:
    ''')

    # creating token contract
    token_contract = Contract(token_host, TOKEN_CONTRACT_WORKSPACE)
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

    token_host.push_action(
        "create",
        {
            "issuer": token_host,
            "maximum_supply": "1000000000 EOS"
        },
        [token_host]
    )


    ########################################################################################################
    COMMENT('''
    Create QUILL tokens 
    ''')

    token_host.push_action(
        "create",
        {
            "issuer": issuer,
            "maximum_supply": "1000000000 QUILL"
        },
        [token_host]
    )


    ########################################################################################################
    COMMENT('''
    Issue EOS tokens to the sub accounts 
    ''')

    # give tokens to alice
    token_host.push_action(
        "issue",
        {
            "to": alice,
            "quantity": "1000000 EOS",
            "memo": "issued tokens to alice"
        },
        [token_host]
    )

    # give tokens to carol
    token_host.push_action(
        "issue",
        {
            "to": carol,
            "quantity": "1000000 EOS",
            "memo": "issued tokens to carol"
        },
        [token_host]
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

    # deposit some funds
    host.push_action(
        "invest",
        {
            "investor": alice,
            "quantity": "10000 EOS"
        },
        permission=(alice, Permission.ACTIVE)
    )



    # assert("host" in DEBUG())

    stop()


if __name__ == "__main__":
    test()
