#!/usr/bin/env python

import argparse
import json
import os
import subprocess
import sys
import time

args = None
logFile = None

datas = {
      'initAccounts':[],
      'initProducers':[],
      'initProducerSigKeys':[],
      'initAccountsKeys':[],
      'maxClients':0
}

unlockTimeout = 999999999

def jsonArg(a):
    return " '" + json.dumps(a) + "' "

def run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-eosforce.py: exiting because of error')
        sys.exit(1)

def retry(args):
    while True:
        print('bios-boot-eosforce.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
        else:
            break

def background(args):
    print('bios-boot-eosforce.py:', args)
    logFile.write(args + '\n')
    return subprocess.Popen(args, shell=True)

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def replaceFile(file, old, new):
    try:
        f = open(file,'r+')
        all_lines = f.readlines()
        f.seek(0)
        f.truncate()
        for line in all_lines:
            line = line.replace(old, new)
            f.write(line)
        f.close()
    except Exception,e:
        print('bios-boot-eosforce.py: replace %s frome %s to %s err by ' % (file, old, new))
        print(e)
        sys.exit(1)

def importKeys():
    keys = {}
    for a in datas["initAccountsKeys"]:
        key = a[1]
        if not key in keys:
            keys[key] = True
            run(args.cleos + 'wallet import --private-key ' + key)

def intToCurrency(i):
    return '%d.%04d %s' % (i // 10000, i % 10000, args.symbol)

def createNodeDir(nodeIndex, bpaccount, key):
    dir = args.nodes_dir + ('%02d-' % nodeIndex) + bpaccount['name'] + '/'
    run('rm -rf ' + dir)
    run('mkdir -p ' + dir)

def createNodeDirs(inits, keys):
    for i in range(0, len(inits)):
        createNodeDir(i + 1, datas["initProducers"][i], keys[i])

def startNode(nodeIndex, bpaccount, key):
    dir = args.nodes_dir + ('%02d-' % nodeIndex) + bpaccount['name'] + '/'
    otherOpts = ''.join(list(map(lambda i: '    --p2p-peer-address 127.0.0.1:' + str(9001 + i), range(nodeIndex - 1))))
    if not nodeIndex: otherOpts += (
        '    --plugin eosio::history_plugin'
        '    --plugin eosio::history_api_plugin'
    )


    print('bpaccount ', bpaccount)
    print('key ', key, ' ', key[1])

    cmd = (
        args.nodeos +
        '    --blocks-dir ' + os.path.abspath(dir) + '/blocks'
        '    --config-dir ' + os.path.abspath(dir) + '/../../config'
        '    --data-dir ' + os.path.abspath(dir) +
        '    --http-server-address 0.0.0.0:' + str(8000 + nodeIndex) +
        '    --p2p-listen-endpoint 0.0.0.0:' + str(9000 + nodeIndex) +
        '    --max-clients ' + str(datas["maxClients"]) +
        '    --p2p-max-nodes-per-host ' + str(datas["maxClients"]) +
        '    --enable-stale-production'
        '    --producer-name ' + bpaccount['name'] +
        '    --signature-provider=' + bpaccount['bpkey'] + '=KEY:' + key[1] +
        '    --contracts-console ' +
        '    --plugin eosio::http_plugin' +
        '    --plugin eosio::chain_api_plugin' +
        '    --plugin eosio::producer_plugin' +
        otherOpts)
    with open(dir + '../' + bpaccount['name'] + '.log', mode='w') as f:
        f.write(cmd + '\n\n')
    background(cmd + '    2>>' + dir + '../' + bpaccount['name'] + '.log')

def startProducers(inits, keys):
    for i in range(0, len(inits)):
        startNode(i + 1, datas["initProducers"][i], keys[i])

def listProducers():
    run(args.cleos + 'get table eosio eosio bps')

def stepKillAll():
    run('killall keosd nodeos || true')
    sleep(.5)

def stepStartWallet():
    run('rm -rf ' + os.path.abspath(args.wallet_dir))
    run('mkdir -p ' + os.path.abspath(args.wallet_dir))
    background(args.keosd + ' --unlock-timeout %d --http-server-address 0.0.0.0:6666 --wallet-dir %s' % (unlockTimeout, os.path.abspath(args.wallet_dir)))
    sleep(.4)

def stepCreateWallet():
    run('mkdir -p ' + os.path.abspath(args.wallet_dir))
    run(args.cleos + 'wallet create --file ./pw')

def stepStartProducers():
    startProducers(datas["initProducers"], datas["initProducerSigKeys"])
    sleep(7)
    stepSetFuncs()

def stepCreateNodeDirs():
    createNodeDirs(datas["initProducers"], datas["initProducerSigKeys"])
    sleep(0.5)

def stepLog():
    run('tail -n 1000 ' + args.nodes_dir + 'biosbpa.log')
    run(args.cleos + ' get info')
    print('you can use \"alias cleost=\'%s\'\" to call cleos to testnet' % args.cleos)

def stepMkConfig():
    with open(os.path.abspath(args.config_dir) + '/genesis.json') as f:
        a = json.load(f)
        datas["initAccounts"] = a['initial_account_list']
        datas["initProducers"] = a['initial_producer_list']
    with open(os.path.abspath(args.config_dir) + '/keys/sigkey.json') as f:
        a = json.load(f)
        datas["initProducerSigKeys"] = a['keymap']
    with open(os.path.abspath(args.config_dir) + '/keys/key.json') as f:
        a = json.load(f)
        datas["initAccountsKeys"] = a['keymap']
    datas["maxClients"] = len(datas["initProducers"]) + 10

def stepMakeGenesis():
    run('rm -rf ' + os.path.abspath(args.config_dir))
    run('mkdir -p ' + os.path.abspath(args.config_dir))
    run('mkdir -p ' + os.path.abspath(args.config_dir) + '/keys/' )

    run('cp ../genesis/force.token.abi ' + os.path.abspath(args.config_dir))
    run('cp ../genesis/force.token.wasm ' + os.path.abspath(args.config_dir))
    run('cp ../genesis/force.system.abi ' + os.path.abspath(args.config_dir))
    run('cp ../genesis/force.system.wasm ' + os.path.abspath(args.config_dir))
    run('cp ../genesis/force.msig.abi ' + os.path.abspath(args.config_dir))
    run('cp ../genesis/force.msig.wasm ' + os.path.abspath(args.config_dir))

    #run('cp ./genesis-data/genesis.json ' + os.path.abspath(args.config_dir))
    #replaceFile(os.path.abspath(args.config_dir) + "/genesis.json", "#CORE_SYMBOL#", args.symbol)
    #replaceFile(os.path.abspath(args.config_dir) + "/genesis.json", "#PUB#", args.pr)
    #run('cp ./genesis-data/key.json ' + os.path.abspath(args.config_dir) + '/keys/')
    #run('cp ./genesis-data/sigkey.json ' + os.path.abspath(args.config_dir) + '/keys/')
    run('''echo "## Notify Plugin
plugin = eosio::notify_plugin
# notify-filter-on = account:action
notify-filter-on = b1:
notify-filter-on = b1:transfer
notify-filter-on = eosio:delegatebw
# http endpoint for each action seen on the chain.
notify-receive-url = http://127.0.0.1:8080/notify
# Age limit in seconds for blocks to send notifications. No age limit if set to negative.
# Used to prevent old actions from trigger HTTP request while on replay (seconds)
notify-age-limit = -1
# Retry times of sending http notification if failed.
notify-retry-times = 3" > ''' + os.path.abspath(args.config_dir) + '/config.ini')
        
    run(args.root + 'build/programs/genesis/genesis')
    run('mv ./genesis.json ' + os.path.abspath(args.config_dir))
    run('mv ./activeacc.json ' + os.path.abspath(args.config_dir))
    
    run('mv ./key.json ' + os.path.abspath(args.config_dir) + '/keys/')
    run('mv ./sigkey.json ' + os.path.abspath(args.config_dir) + '/keys/')

def setFuncStartBlock(func_typ, num):
    run(args.cleos +
        'push action force setconfig ' +
        ('\'{"typ":"%s","num":%s,"key":"","fee":"%s"}\' ' % (func_typ, num, intToCurrency(0))) +
        '-p force.config' )

def setFee(account, act, fee, cpu, net, ram):
    run(args.cleos +
        'set setfee ' +
        ('%s %s ' % (account, act)) +
        '"' + intToCurrency(fee) + '" ' +
        ('%d %d %d' % (cpu, net, ram)))

def stepSetFuncs():
    # we need set some func start block num
    # setFee('eosio', 'setconfig', 100, 100000, 1000000, 1000)

    # some config to set
    print('stepSetFuncs')

def clearData():
    stepKillAll()
    run('rm -rf ' + os.path.abspath(args.config_dir))
    run('rm -rf ' + os.path.abspath(args.nodes_dir))
    run('rm -rf ' + os.path.abspath(args.wallet_dir))
    run('rm -rf ' + os.path.abspath(args.log_path))
    run('rm -rf ' + os.path.abspath('./pw'))
    run('rm -rf ' + os.path.abspath('./config.ini'))

def restart():
    stepKillAll()
    stepMkConfig()
    stepStartWallet()
    stepCreateWallet()
    importKeys()
    stepStartProducers()
    stepLog()

# =======================================================================================================================
# Command Line Arguments
parser = argparse.ArgumentParser()

commands = [
    ('k', 'kill',           stepKillAll,                False,   "Kill all nodeos and keosd processes"),
    ('c', 'clearData',      clearData,                  False,   "Clear all Data, del ./nodes and ./wallet"),
    ('r', 'restart',        restart,                    False,   "Restart all nodeos and keosd processes"),
    ('g', 'mkGenesis',      stepMakeGenesis,            True,    "Make Genesis"),
    ('m', 'mkConfig',       stepMkConfig,               True,    "Make Configs"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start keosd, create wallet, fill with keys"),
    ('W', 'createWallet',   stepCreateWallet,           True,    "Create wallet"),
    ('i', 'importKeys',     importKeys,                 True,    "importKeys"),
    ('D', 'createDirs',     stepCreateNodeDirs,         True,    "create dirs for node and log"),
    ('P', 'start-prod',     stepStartProducers,         True,    "Start producers"),
    ('l', 'log',            stepLog,                    True,    "Show tail of node's log"),
]

parser.add_argument('--root', metavar='', help="Eosforce root dir from git", default='../../')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default='build/contracts/')
parser.add_argument('--cleos', metavar='', help="Cleos command", default='build/programs/cleos/cleos --wallet-url http://127.0.0.1:6666 ')
parser.add_argument('--nodeos', metavar='', help="Path to nodeos binary", default='build/programs/nodeos/nodeos')
parser.add_argument('--nodes-dir', metavar='', help="Path to nodes directory", default='./nodes/')
parser.add_argument('--keosd', metavar='', help="Path to keosd binary", default='build/programs/keosd/keosd')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--config-dir', metavar='', help="Path to config directory", default='./config')
parser.add_argument('--symbol', metavar='', help="The core symbol", default='SYS')
parser.add_argument('--pr', metavar='', help="The Public Key Start Symbol", default='FOSC')
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")

for (flag, command, function, inAll, help) in commands:
    prefix = ''
    if inAll: prefix += '*'
    if prefix: help = '(' + prefix + ') ' + help
    if flag:
        parser.add_argument('-' + flag, '--' + command, action='store_true', help=help, dest=command)
    else:
        parser.add_argument('--' + command, action='store_true', help=help, dest=command)

args = parser.parse_args()

args.cleos += '--url http://127.0.0.1:%d ' % 8001

args.cleos = args.root + args.cleos
args.nodeos = args.root + args.nodeos
args.keosd = args.root + args.keosd
args.contracts_dir = args.root + args.contracts_dir

logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')

haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()

if not haveCommand:
    print('bios-boot-eosforce.py: Tell me what to do. -a does almost everything. -h shows options.')
