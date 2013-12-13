#!/usr/bin/python

import sys
import time

from rpc import call

PLAYING = 1

while True :
    # clear the line
    sys.stdout.write("\r%s\r" % (" " * 79))

    status = call('player_status', {})

    if status :
        # print the current status
        if status["state"] == PLAYING :
            sys.stdout.write('%s - %02d:%02d' % (status["current"]["name"], status["position"] / 60, status["position"] % 60))
        else :
            sys.stdout.write('not playing')
    else :
        sys.stdout.write('player not available')

    # flush the output
    sys.stdout.flush()

    time.sleep(1)
