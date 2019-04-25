
from . import MuxResource
from utils import settings


errorDetail = {
    "code": 500,
    "error": "Internal error",
    "debug": "undefined",
    }


class Root(MuxResource):

    def __init__(self, **kwargs):
        super(Root, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_ROOT
    
    def get(self, *args, **kwargs):
        
        return super(Root, self).get(*args, **kwargs)

        #args = parser.parse_args()
        #if (args['username'] is None or args['passwd'] is None):
        #    abort(401, message="no username and passwd is provided")
        
        # self.req['username'] = kwargs.get("username", None)
        # self.req['passwd'] = kwargs.get("passwd", None)
        # reqRoot['username'] = args['username']
        
        # ret = self.conn.connect(cmd=self.req)

        # return ret

class RestError(MuxResource):
    
    def get(self, *args, **kwargs):
        return errorDetail
    