from flask_restful import Resource

rootCfg = [
    'system/',
    'video/',
    'audio/',
    'anc/',
    'rs232/',
]

class Root(Resource):
    
    def get(self):
        return rootCfg
    