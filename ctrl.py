#!/usr/bin/python

import sys
import json

try :
    import urllib.request
    urlopen = urllib.request.urlopen
except ImportError :
    import urllib
    urlopen = urllib.urlopen

def call(method, params) :
    req = {
        "jsonrpc" : "2.0",
        "method" : method,
        "id" : 1,
        "params" : params
    }
    urlopen('http://localhost:8080', json.dumps(req).encode('utf-8'))

def queue(id) :
    call('player_queue_file', {'id' : id})

def play() :
    call('player_play', {})

def stop() :
    call('player_stop', {})

if len(sys.argv) < 2 :
    sys.exit(1)

cmd = sys.argv[1]

if cmd == "queue" :
    if len(sys.argv) >= 3 :
        queue(int(sys.argv[2]))
elif cmd == "play" :
    play()
elif cmd == "stop" :
    stop()
