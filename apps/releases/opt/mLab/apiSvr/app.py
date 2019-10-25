#!/usr/bin/env python

from flask import Flask
from flask_restful import reqparse, abort, Api

from resources.root import Root, RestError
from resources.system import System, Rs232, IR, PTP, Security, Others
from resources.medias import Video, Audio, Anc, SdpConfig
from resources.sdp import VideoSdp, AudioSdp, AncSdp

from utils import settings

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


api.add_resource(Root, settings.SERVICE_URI_ROOT)
api.add_resource(System, settings.SERVICE_URI_SYSTEM)
api.add_resource(Rs232, settings.SERVICE_URI_RS232)
api.add_resource(IR, settings.SERVICE_URI_IR)
api.add_resource(PTP, settings.SERVICE_URI_PTP)
api.add_resource(Security, settings.SERVICE_URI_SECURITYS)
api.add_resource(Others, settings.SERVICE_URI_OETHERS)

api.add_resource(SdpConfig, settings.SERVICE_URI_SDP_CONFIG)

api.add_resource(Video, settings.SERVICE_URI_VIDEO)
api.add_resource(VideoSdp, settings.SERVICE_URI_SDP_VIDEO)# , '/audio/<str:id>')

api.add_resource(Audio, settings.SERVICE_URI_AUDIO)
api.add_resource(AudioSdp, settings.SERVICE_URI_SDP_AUDIO)# , '/audio/<str:id>')

api.add_resource(Anc, settings.SERVICE_URI_ANC)
api.add_resource(AncSdp, settings.SERVICE_URI_SDP_ANC)# , '/audio/<str:id>')


# api.add_resource(RestError, '/error')


if __name__ == '__main__':
    #from flup.server.fcgi import WSGIServer
    # WSGIServer(app).run()
    app.run(debug=True, host="0.0.0.0")

