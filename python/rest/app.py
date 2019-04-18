#!/usr/bin/env python

from flask import Flask
from flask_restful import Api

from resources.root import Root, RestError
from resources.system import System
from resources.medias import Video
from resources.medias import Audio
from resources.medias import Anc
from resources.sdp import VideoSdp
from resources.sdp import AudioSdp
from resources.sdp import AncSdp

# import sys
# sys.setdefaultencoding('utf-8')

import os
os.putenv('LANG', 'en_US.UTF-8')
os.putenv('LC_ALL', 'en_US.UTF-8')


app = Flask(__name__)
api = Api(app)

@api.representation('application/octet-stream')
def binary(data, code, headers=None):
    resp = api.make_response(data, code)
    resp.headers.extend(headers or {})
    return resp


api.add_resource(Root, '/')
api.add_resource(System, '/system') #, '/system/<str:id>')
api.add_resource(Video, '/video') #, '/video/<str:id>')
api.add_resource(VideoSdp, '/video/sdp')# , '/audio/<str:id>')

api.add_resource(Audio, '/audio')# , '/audio/<str:id>')
api.add_resource(AudioSdp, '/audio/sdp')# , '/audio/<str:id>')

api.add_resource(Anc, '/anc')# , '/audio/<str:id>')
api.add_resource(AncSdp, '/anc/sdp')# , '/audio/<str:id>')

# api.add_resource(RestError, '/error')


if __name__ == '__main__':
    #from flup.server.fcgi import WSGIServer
    # WSGIServer(app).run()
    app.run(debug=True, host="0.0.0.0")

