#!/usr/bin/python3

from html import escape

def application(environ, start_response):

    envvar = ['%s=%s' % (key, value) for key, value in sorted(environ.items())]

    body = ('<html><body><h2>hello world (python3 fcgi)</h2><pre>' + escape('\n'.join(envvar)) + '</pre></body></html>').encode()
    status = '200 OK'
    headers = [('Content-Type', 'text/html; charset=utf-8'), ('Content-Length', str(len(body)))]
    start_response(status, headers)

    return [body]

if __name__ == '__main__':
    from flup.server.fcgi import WSGIServer
    WSGIServer(application).run()
    