
from comm.cmdIf import CmdSocket
from . import MuxResource
from utils import settings


class Video(MuxResource):
    def __init__(self, **kwargs):
        super(Video, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_VIDEO
    
    def get(self, *args, **kwargs):        
        return super(Video, self).get(*args, **kwargs)
    
    def post(self, *args, **kwargs):
        return super(Video, self).post(*args, **kwargs)


class Audio(MuxResource):
    def __init__(self, **kwargs):
        super(Audio, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_AUDIO
    
    def get(self, *args, **kwargs):        
        return super(Audio, self).get(*args, **kwargs)
    
    def post(self, *args, **kwargs):
        return super(Audio, self).post(*args, **kwargs)



class Anc(MuxResource):
    def __init__(self, **kwargs):
        super(Anc, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_ANC
    
    def get(self, *args, **kwargs):        
        return super(Anc, self).get(*args, **kwargs)
    
    def post(self, *args, **kwargs):
        return super(Anc, self).post(*args, **kwargs)



class SdpConfig(MuxResource):
    def __init__(self, **kwargs):
        super(SdpConfig, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_SDP_CONFIG
    
    def get(self, *args, **kwargs):        
        return super(SdpConfig, self).get(*args, **kwargs)
    
    def post(self, *args, **kwargs):
        return super(SdpConfig, self).post(*args, **kwargs)

