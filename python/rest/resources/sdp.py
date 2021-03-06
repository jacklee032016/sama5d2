import struct
import time
import json

from flask import Flask
from flask import current_app
# from flask_restful import Resource
from . import MuxResource
from .system import System
from .medias import Video, Audio, Anc 

# from flask.sessions import SessionInterface
from utils import ColorMsg
from utils import settings

class SdpSession(object):
    
    def __init__(self, **kwargs):
        super(SdpSession, self).__init__(**kwargs)
        
    def sessionInfo(self, *args, **kwargs):
        ts = int(time.time())
        self.sys = System(**kwargs).get(*args, **kwargs)
        if "status" in self.sys: #["ip"] is None:# when connection broken, the return is dict
            self.error = 1
            return self.sys #'{"status": 500, "detail": "Can not connect to backend server"}' 
        """
        session parameters:
        v=VERSION.
        o=USERNAME(-) SESSION_ID SESSION_VERSION IN IP4 ADDRESS;  session ID: 1|0(TX|RX)+1|0(VIDEO|AUDIO)+current_time() 
        s=  #name of session
        t= 0 0 # start and stop time
        """ 
        # str(datetime.datetime.now()).split('.')[0]
        
        """ comment for simplify testing """
        #if self.sys.get("isTx", 1) == 0:
        #    self.error = 1
        #    return json.loads('{"status": 501, "detail": "RX not support SDP service"}' )
        
        self.error = 0
        sessionStr='v=0\no=- %d 1 IN IP4 %s\ns=MuxLab %s 2110\nt=0 0\n'%(
            ts, 
            self.sys.get("ip", "0.0.0.0"), 
            self.sys.get("name", "NoName")
        )
        return sessionStr

    def mediaInfo(self, *args, **kwargs):
        return ""

class SdpResource(MuxResource, SdpSession):
    def __init__(self, **kwargs):
        super(SdpResource, self).__init__()

    def get(self, *args, **kwargs):
        sesStr = self.sessionInfo(self, *args, **kwargs)
        if self.error == 0:
            mediaStr = self.mediaInfo(self, *args, **kwargs)
        
        if self.error == 1:
            return sesStr

        ## packedCmd = struct.pack("<HH%dsI" % len(bytesCmd), 0xA0FA, socket.ntohs(len(bytesCmd) + 4), bytesCmd, socket.htonl(crc32) )
        sdpStr= sesStr+mediaStr
        response = current_app.make_response(rv=sdpStr)
        response.headers['content-type'] = 'application/octet-stream'
        return response
                  
                  
class VideoSdp(SdpResource):
    def __init__(self, **kwargs):
        super(VideoSdp, self).__init__()

    def mediaInfo(self, *args, **kwargs):
        medias = Video(**kwargs).get(*args, **kwargs)
        if medias is None:
            return ''
        """ when something is wrong, only json object with status is returned cmdIf interface """
        if "status" in medias: # when connection broken, the return is dict
            self.error = 1
            return   #'{"status": 500, "detail": "Media (video) parameter is not array"}'  
        media = medias[0]
        self.payloadType = media.get("payloadType", 96)
        
        """
        Media Parameters
            m=video 1000 RTP/AVP 96
            c=IN IP4 239.20.45.0/64
            a=rtpmap:96 raw/90000
            a=fmtp:96 sampling=YCbCr-4:2:2; width=1920; height=1080; exactframerate=24; depth=10; TCS=SDR; colorimetry=BT709; PM=2110GPM; SSN=ST2110-20:2017; TP=2110TPN;
            a=ts-refclk:ptp=IEEE1588-2008:
            a=mediaclk:direct=0
            a=mid:VID 
        """
        
        str = "m=video %d RTP/AVP %d\n"%(media.get("port", 36000), self.payloadType)
        str += "c=IN IP4 %s/64\n"%(
            media.get("ip", "0.0.0.0")
            )    ## IP/TTL
        fps = media.get("fps", "60")
        if fps == "23.98":
            fps = "24000/1001"
        if fps == "29.97":
            fps = "30000/1001"
        if fps == "59.94":
            fps = "60000/1001"
        str += "a=rtpmap:%d raw/90000\n"%(self.payloadType) 
        str += "a=fmtp:%d sampling=%s; width=%d; height=%d; exactframerate=%s; depth=%d; TCS=SDR; colorimetry=BT709; PM=2110GPM; SSN=ST2110-20:2017; TP=2110TPN;\n"%(
            self.payloadType, 
            media.get("colorSpace", "YCbCr-4:2:2"), 
            media.get("width", 1920), 
            media.get("height", 1080), 
            fps,
            media.get("depth", 16) )
        
        # mediaStr += "a=ts-refclk:ptp=IEEE1588-2008:\n"
        mac= self.sys.get("mac", "FF:FF:FF:FF:FF:FF")
        str += "a=ts-refclk:localmac=%s-%s-%s-%s-%s-%s\n"%(mac[0:2],mac[3:5],mac[6:8],mac[9:11],mac[12:14],mac[15:17])
        str += "a=mediaclk:direct=0\n"
        str += "mid:VID\n"
        return str

        

class AudioSdp(SdpResource):
    def __init__(self, **kwargs):
        super(AudioSdp, self).__init__()

    def mediaInfo(self, *args, **kwargs):
        medias = Audio(**kwargs).get(*args, **kwargs)
        # ColorMsg.error_msg("Media is %s"%medias)
        if medias is None:
            return ''
        if "status" in medias: #["ip"] is None:
            self.error = 1
            return  #'{"status": 500, "detail": "Media (audio) parameter is not array"}'  
       # if isinstance(medias, list):
        media = medias[0]
        self.payloadType = media.get("payloadType", 97)
        """
        Media Parameters
            m=audio 1000 RTP/AVP 100
            c=IN IP4 239.21.45.0/64
            a=rtpmap:100 L24/48000/16
            a=fmtp:100 channel-order=SMPTE2110.(SGRP,SGRP,SGRP,SGRP)
            a=ptime:0.125
            a=tsrefclk:ptp=IEEE1588-2008:
            a=mediaclk:direct=0
            a=mid:AUD
        """
        str = "m=audio %d RTP/AVP %d\n"%(
            media.get("port", 36010), 
            self.payloadType
        )
        str += "c=IN IP4 %s/64\n"%(media.get("ip","0.0.0.0") )    ## IP/TTL
        str += "a=rtpmap:%d L24/%s/%d\n"%(
            self.payloadType, 
            media.get("sample", "48000"), 
            media.get("channels", 16) 
        ) 

        str += "a=fmtp:%d channel-order=SMPTE2110.(SGRP,SGRP,SGRP,SGRP)\n"%(self.payloadType)
        str += "a=ptime:%s\n"%("1" if media.get("pktSize", "1ms")=="1ms" else "0.125")
        
        # mediaStr += "a=ts-refclk:ptp=IEEE1588-2008:\n"
        mac= self.sys.get("mac", "FF:FF:FF:FF:FF:FF")
        str += "a=ts-refclk:localmac=%s-%s-%s-%s-%s-%s\n"%(mac[0:2],mac[3:5],mac[6:8],mac[9:11],mac[12:14],mac[15:17])
        str += "a=mediaclk:direct=0\n"
        str += "mid:AUD\n"
        return str


class AncSdp(SdpResource):
    def __init__(self, **kwargs):
        super(AncSdp, self).__init__()

    def mediaInfo(self, *args, **kwargs):
        medias = Anc(**kwargs).get(*args, **kwargs)
        """ when something is wrong, only json object with status is returned cmdIf interface """
        if medias is None:
            return ''
        if "status" in medias: #["ip"] is None:
            self.error = 1
            return  #'{"status": 500, "detail": "Media parameter (ANC) is not array"}'  
        media = medias[0]
        
        self.payloadType = media.get("payloadType", 100)
        """
        Media Parameters
            m=video 1000 RTP/AVP 100
            c=IN IP4 239.20.45.0/64
            a=rtpmap:96 smpte291/90000
            a=a=fmtp:100 VPID_Code=%d
            a=ts-refclk:ptp=IEEE1588-2008:
            a=mediaclk:direct=0
            a=mid:VID 
        """
        str = "m=video %d RTP/AVP %d\n"%(
            media.get("port", 36020), 
            self.payloadType
        )
        str += "c=IN IP4 %s/64\n"%(media.get("ip", "0.0.0.0"))    ## IP/TTL
        str += "a=rtpmap:%d smpte291/90000\n"%(self.payloadType) 
        str += "a=fmtp:%d VPID_Code=%d\n"%(
            self.payloadType, 
            media.get("vpId", 132)
        )
        
        str += "a=mediaclk:direct=0\n"
        return str
    