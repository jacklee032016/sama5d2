from flask import Flask
from flask import current_app
from flask_restful import Resource

class VideoSdp(Resource):
    def get(self):
        sdpStr='v=0\no=- 37 42 IN IP4 192.168.168.101\ns=\nt=0 0\n'
        response = current_app.make_response(rv=sdpStr)
        response.headers['content-type'] = 'application/octet-stream'
        return response

class AudioSdp(Resource):
    def get(self):
        sdpStr='v=0\no=- 37 42 IN IP4 192.168.168.101\ns=\nt=100 100\n'
        response = current_app.make_response(rv=sdpStr)
        response.headers['content-type'] = 'application/octet-stream'
        return response

class AncSdp(Resource):
    def get(self):
        sdpStr='v=0\no=- 37 42 IN IP4 192.168.168.101\ns=\nt=200 200\n'
        response = current_app.make_response(rv=sdpStr)
        response.headers['content-type'] = 'application/octet-stream'
        return response
    