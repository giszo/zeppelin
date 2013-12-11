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
    f = urlopen('http://localhost:8080', json.dumps(req).encode('utf-8'))
    print(f.read())

def lib_scan() :
    call('library_scan', {})

def lib_list() :
    call('library_list_files', {})

def lib_artists() :
    call('library_get_artists', {})

def lib_albums() :
    call('library_get_albums', {})

def lib_albums_by_artist(id) :
    call('library_get_albums_by_artist', {'artist_id' : id})

def queue_list() :
    call('player_queue_get', {})

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
elif cmd == "lib_scan" :
    lib_scan()
elif cmd == "lib_list" :
    lib_list()
elif cmd == "lib_artists" :
    lib_artists()
elif cmd == "lib_albums" :
    lib_albums()
elif cmd == "lib_albums_by_artist" :
    if len(sys.argv) >= 3 :
        lib_albums_by_artist(int(sys.argv[2]))
elif cmd == "queue_list" :
    queue_list()
