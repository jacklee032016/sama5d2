
from flask_restful import Resource

videoCfg = {
    'ip': '239.0.0.100',
    'port': 3600,
    'fps': 59,
}

audioCfg = {
    'ip': '239.0.0.100',
    'port': 3620,
    'sample': 48000,
}

ancCfg = {
    'ip': '239.0.0.100',
    'port': 3640,
}


class Video(Resource):
    def get(self):
        return videoCfg
    
    def post(self):
        return videoCfg, 201

class Audio(Resource):
    def get(self):
        return audioCfg
    
    def post(self):
        return audioCfg, 201

class Anc(Resource):
    def get(self):
        return ancCfg
    
    def post(self):
        return ancCfg, 201
