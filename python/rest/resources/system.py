
from comm.cmdIf import CmdSocket
from . import MuxResource
from utils import settings

class System(MuxResource):
    def __init__(self, **kwargs):
        super(System, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_SYSTEM
    
    def get(self, *args, **kwargs):        
        return super(System, self).get(*args, **kwargs)
    
        
    def post(self, *args, **kwargs):
        return super(System, self).post(*args, **kwargs)


    def delete(self, *args, **kwargs):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204

class Rs232(MuxResource):
    def __init__(self, **kwargs):
        super(Rs232, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_RS232
    
    def get(self, *args, **kwargs):        
        return super(Rs232, self).get(*args, **kwargs)
    
        
    def post(self, *args, **kwargs):
        return super(Rs232, self).post(*args, **kwargs)


    def delete(self, *args, **kwargs):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204

class IR(MuxResource):
    def __init__(self, **kwargs):
        super(IR, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_IR
    
    def get(self, *args, **kwargs):        
        return super(IR, self).get(*args, **kwargs)
    
        
    def post(self, *args, **kwargs):
        return super(IR, self).post(*args, **kwargs)


    def delete(self, *args, **kwargs):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204


class Security(MuxResource):
    def __init__(self, **kwargs):
        super(Security, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_SECURITYS
    
    def get(self, *args, **kwargs):        
        return super(Security, self).get(*args, **kwargs)
    
        
    def post(self, *args, **kwargs):
        return super(Security, self).post(*args, **kwargs)


    def delete(self, *args, **kwargs):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204


class Others(MuxResource):
    def __init__(self, **kwargs):
        super(Others, self).__init__()
        self.req[settings.SERVICE_DATA_FIELD_URI] = settings.SERVICE_URI_OETHERS
    
    def get(self, *args, **kwargs):        
        return super(Others, self).get(*args, **kwargs)
    
        
    def post(self, *args, **kwargs):
        return super(Others, self).post(*args, **kwargs)


    def delete(self, *args, **kwargs):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204
    