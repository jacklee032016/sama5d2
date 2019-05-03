from flask_restful import Resource, abort, reqparse

from functools import wraps

from comm.cmdIf import CmdSocket
from utils import settings
# import json

def authenticate(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        if not getattr(func, 'authenticated', True):
            return func(*args, **kwargs)

        parser = reqparse.RequestParser()
        parser.add_argument(settings.SERVICE_DATA_FIELD_USERNAME,
                            # required=True,
                            help="username cannot be blank!") 
        parser.add_argument(settings.SERVICE_DATA_FIELD_PASSWORD, 
                            # required=True, 
                            help="passwd cannot be blank!")

        parser.add_argument(settings.SERVICE_DATA_FIELD_DATA, 
                            # required=True, 
                            type=dict, action='append', # must parsed as dict, otherwise it is parsed as a string, eg, one value. JL. 05.01,2019
                            help="data is optional array!")

        args = parser.parse_args()
        
        # use authen in C controller, so more flexible
        # if (args['username'] is None or args['passwd'] is None):
        #    abort(401, message="no username and passwd is provided")
        
        kwargs[settings.SERVICE_DATA_FIELD_USERNAME] = args.get(settings.SERVICE_DATA_FIELD_USERNAME, None)
        kwargs[settings.SERVICE_DATA_FIELD_PASSWORD] = args.get(settings.SERVICE_DATA_FIELD_PASSWORD, None)
        kwargs[settings.SERVICE_DATA_FIELD_DATA] = args.get(settings.SERVICE_DATA_FIELD_DATA, None)
        #acct = basic_authentication()  # custom account lookup function

        # if acct:
        return func(*args, **kwargs)

        # abort(401)
    return wrapper



class MuxResource(Resource):
    method_decorators = [authenticate]
    
    def __init__(self, **kwargs):
        # smart_engine is a black box dependency
        # self.smart_engine = kwargs['smart_engine']
        # self.debug = kwargs.get("debug", False)
        self.req = {};
        self.req[settings.SERVICE_DATA_FIELD_METHOD] = settings.REQUEST_METHOD_GET 
        
        self.conn = CmdSocket(debug=True, **kwargs)


    def get(self, *args, **kwargs):

        #args = parser.parse_args()
        #if (args['username'] is None or args['passwd'] is None):
        #    abort(401, message="no username and passwd is provided")
        
        self.req[settings.SERVICE_DATA_FIELD_USERNAME] = kwargs.get(settings.SERVICE_DATA_FIELD_USERNAME, None)
        self.req[settings.SERVICE_DATA_FIELD_PASSWORD] = kwargs.get(settings.SERVICE_DATA_FIELD_PASSWORD, None)
        # reqRoot['username'] = args['username']
        
        ret = self.conn.connect(cmd=self.req)

        return ret


    def post(self, *args, **kwargs):
        self.req[settings.SERVICE_DATA_FIELD_USERNAME] = kwargs.get(settings.SERVICE_DATA_FIELD_USERNAME, None)
        self.req[settings.SERVICE_DATA_FIELD_PASSWORD] = kwargs.get(settings.SERVICE_DATA_FIELD_PASSWORD, None)
        self.req[settings.SERVICE_DATA_FIELD_METHOD] = settings.REQUEST_METHOD_POST
        
        # self.req[settings.SERVICE_DATA_FIELD_DATA] = kwargs.get(settings.SERVICE_DATA_FIELD_DATA, None)
        data =kwargs.get(settings.SERVICE_DATA_FIELD_DATA, None)  
        if data is not None:
            print(data)
            self.req[settings.SERVICE_DATA_FIELD_DATA] = []
            self.req[settings.SERVICE_DATA_FIELD_DATA].insert(0, data) 

        ret = self.conn.connect(cmd=self.req)
        return ret, 201
