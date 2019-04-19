
import abc
import socket
import logging
import struct
import time

from cmds import DeviceCtrl
from . import CommandCodec
from . import MSGLEN

from utils import ColorMsg
from utils import settings

LOGGER = logging.getLogger(__name__)

class IpCmdSocket(object):
    """
    JSON API socket
    Open one socket for multiple commands

    """

    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def load_data(self):
        """
        Abstract interface which must be implemented by child class
        :return: boolean indicates OK or fail
        """
        return

    def __init__(self, devCtrl, *args, **kwargs):
        self.dev = devCtrl
        self.server = kwargs.get("ip", "<broadcast>")
        self.port = kwargs.get("port", 3600)
        self.debug = kwargs.get("debug", False)
        #self.simGateway = kwargs.get("simGateway", None)

        self.peer = (self.server, self.port)

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # Set a timeout so the socket does not block indefinitely when trying to receive data.
        self.sock.settimeout(settings.IP_CMD_FIND_TIMEOUT)

        ColorMsg.debug_msg( '%s, %s:%s, gateway:%s'%(self.__class__.__name__, self.server, self.port, self.dev.simGateway), self.debug)

        if self.server == "<broadcast>":
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

        if self.dev.simGateway is not None:
            self.sock.bind((self.dev.simGateway, 3600))

    def send(self, cmd):
        self.cmd = cmd['cmd']
        self.sendPacket(CommandCodec.encode(cmd, debug=self.debug))

    def sendPacket(self, data):
        """
        """
        ColorMsg.debug_msg('send packet: %s to %s\n' % (data, self.peer), self.debug)
        self.start_time = time.time()
        sent = self.sock.sendto(data, self.peer)
        return sent

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
                return nodes

            # ColorMsg.error_msg("chunk %d: %s" % (len(datas), self.cmd))
            for chunk in datas:
                bytes_recd = 0
                chunks = b''  # bytearray()#[]
                if chunk == b'':
                    raise RuntimeError("socket connection broken")
                if bytes_recd == 0 and ( (chunk is not None) and (len(chunk) > 4) ):
                    unpacker = struct.Struct("<HH")
                    tag, lenPacket = unpacker.unpack(chunk[0:4])
                    lenPacket = socket.ntohs(lenPacket)

                chunks = b"".join([chunks, chunk])  # (chunk)#.decode('utf-8'))
                bytes_recd = bytes_recd + len(chunk)
                # ColorMsg.debug_msg('read length :%d, IP Command length :%d' % (bytes_recd, lenPacket), self.debug)
                if bytes_recd > lenPacket + 4:
                    ColorMsg.error_msg("bytes_read %d larger than %s" % (bytes_recd, lenPacket+4))
                    break

                # ColorMsg.debug_msg("receive data:%s" % (chunks), self.debug)
                replyJson = CommandCodec.decode(chunks, debug=self.debug)
                if replyJson is not None:
                    nodes.append(replyJson)

            #self.sock.close()
            return nodes

    def receiveRaw(self):
        """
        : default for Unix socket and TCP socket
        """
        datas = []
        while 1:
            try:
                # return self.sock.recv(4092)
                data, node = self.sock.recvfrom(4096)
                usedTime = (time.time() - self.start_time) * 1000
                ColorMsg.debug_msg("After %s ms, receive %d bytes data from %s data:%s " % (usedTime, len(data), node, data), self.debug)
                ColorMsg.success_msg("\tAfter %s ms, receive %d bytes data from %s" % (usedTime, len(data), node) )
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

    def run(self, *args, **kwargs):
        """ entry point for task/thread.
        If it is called by thread sprawn interface, it is asnchronous;
        If it is called like a function, it is synchronous
        """
        self.status = self.load_data()

    def __repr__(self):
        return self.__class__.__name__
