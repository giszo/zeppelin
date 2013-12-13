#!/usr/bin/python

import sys
import time

from rpc import call

PLAYING = 1

def get_queue() :
    data = call('player_queue_get', {})
    queue = {}

    return {f["id"] : f for f in data}

def format_secs(s) :
    return "%02d:%02d" % (s / 60, s % 60)

while True :
    # clear the line
    sys.stdout.write("\r%s\r" % (" " * 79))

    queue = get_queue()
    status = call('player_status', {})

    if status :
        # print the current status
        if status["state"] == PLAYING :
            file = queue[status["current"]]
            sys.stdout.write('%s - %s/%s' % (file["name"], format_secs(status["position"]), format_secs(file["length"])))
        else :
            sys.stdout.write('not playing')
    else :
        sys.stdout.write('player not available')

    # flush the output
    sys.stdout.flush()

    time.sleep(1)
