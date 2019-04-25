#!/usr/bin/env python

from comm.cmdIf import CmdSocket

# request form from REST API

cmdRoot ={
    "method" : "GET",
    "uri": "/video",
    "username":"admin",
    "passwd":"admin",
    "data": [{
        "ip":"192.168.168.171",
        "port:": 4700,
        }]
    }


if __name__ == '__main__':
    test = CmdSocket(debug=True, cmd=cmdRoot)
    test.run()
