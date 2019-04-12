
import abc
import socket
import logging
import struct
import time
import sys
import json

#from cmds import DeviceCtrl
#from . import CommandCodec
from . import MSGLEN

from utils import ColorMsg
from utils import settings

LOGGER = logging.getLogger(__name__)

class CmdSocket(object):

    __metaclass__ = abc.ABCMeta
    
    def __init__(self, *args, **kwargs):
        self.server = kwargs.get("ip", "/tmp/unixsocketServer")
        """ self.cmd is json format, or any python object """
        self.cmd = kwargs.get("cmd", "get")
        self.debug = kwargs.get("debug", False)
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        ColorMsg.debug_msg('connect to %s\n' % (self.server), self.debug)
        try:
            self.sock.connect(self.server)
        except socket.error as msg:
            ColorMsg.error_msg("connect failed")
            ColorMsg.error_msg(msg)
            sys.exit(1)

    def run(self, *args, **kwargs):
        #self.status = self.load_data()
        self.send()
        data = self.receive()
        return data

    def __repr__(self):
        return self.__class__.__name__

    def send(self ):
        #self.cmd = cmd['cmd']
        # self.sendPacket(CommandCodec.encode(cmd, debug=self.debug))
        """ change to bytes object """
        cmdStr = json.dumps(self.cmd).encode('ascii')
        """ change to binary buffer to send out in socket """
        cmdBuf = struct.pack( "%ds" % len(cmdStr), cmdStr )
        # self.sendPacket()
        self.sendPacket(cmdBuf)


    def sendPacket(self, data):
        """
        """
        ColorMsg.debug_msg('send packet: %s to %s\n' % (data, self.server), self.debug)
        self.start_time = time.time()
        try:
            sent = self.sock.sendall(data)
            return sent
        except socket.error as e:
            usedTime = (time.time() - self.start_time) * 1000
            ColorMsg.error_msg("\tAfter %s ms, send timeout on socket "% (usedTime))
            self.sock.close()
            #ColorMsg.error_msg('closing UNIX socket when sendall data: %s' %(msg))
            #self.sock.close()
            return 0


    def receive(self):
        """
        receive from socket
        :return: node dict list
        """
        nodes = []
        lenPacket = MSGLEN

        while 1:#bytes_recd < MSGLEN:
            datas = self.receiveRaw()  # min(MSGLEN - bytes_recd, 2048))
            if datas is None:
                break;
                # return datas

            # ColorMsg.error_msg("chunk %d: %s" % (len(datas), self.cmd))
            # for chunk in datas:
            #    bytes_recd = 0
            #    chunks = b''  # bytearray()#[]
            #    if chunk == b'':
            #        raise RuntimeError("socket connection broken")
            #    if bytes_recd == 0 and ( (chunk is not None) and (len(chunk) > 4) ):
            #        unpacker = struct.Struct("<HH")
            #        tag, lenPacket = unpacker.unpack(chunk[0:4])
            #        lenPacket = socket.ntohs(lenPacket)

            #    chunks = b"".join([chunks, chunk])  # (chunk)#.decode('utf-8'))
            #    bytes_recd = bytes_recd + len(chunk)
                # ColorMsg.debug_msg('read length :%d, IP Command length :%d' % (bytes_recd, lenPacket), self.debug)
            #    if bytes_recd > lenPacket + 4:
            #        ColorMsg.error_msg("bytes_read %d larger than %s" % (bytes_recd, lenPacket+4))
            #        break

                # ColorMsg.debug_msg("receive data:%s" % (chunks), self.debug)
            #    replyJson = CommandCodec.decode(chunks, debug=self.debug)
            #    if replyJson is not None:
            #        nodes.append(replyJson)

            #self.sock.close()
            return Datas

    def receiveRaw(self):
        """
        : default for Unix socket and TCP socket
        """
        datas = []
        while 1:
            try:
                # return self.sock.recv(4092)
                ColorMsg.debug_msg('recv from %s\n' % (self.server), self.debug)
                data = self.sock.recv(4096)
                usedTime = (time.time() - self.start_time) * 1000
                #ColorMsg.debug_msg("After %s ms, receive %d bytes data from %s data:%s " % (usedTime, len(data), node, data), self.debug)
                ColorMsg.success_msg("\tAfter %s ms, receive %d bytes data from UNIX socket %s" % (usedTime, len(data), data) )
                datas.append(data)
                if self.server != "<broadcast>":
                    return datas
            except socket.timeout as e:
                usedTime = (time.time() - self.start_time) * 1000
                ColorMsg.error_msg("\tAfter %s ms, read timeout on socket "% (usedTime))
                self.sock.close()
                if self.server != "<broadcast>":
                    return None
                return datas
            # sys.exit(1)
            # logging.error("Read timeout on socket '{}': {}".format(e))
