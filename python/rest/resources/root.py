from flask_restful import Resource

rootCfg = [
    'system/',
    'video/',
    'audio/',
    'anc/',
    'rs232/',
]

errorDetail = {
    "code": 500,
    "error": "Internal error",
    "debug": "undefined",
    }

class Root(Resource):
    
    def get(self):
        return rootCfg

class RestError(Resource):
    
    def get(self):
        return errorDetail
    