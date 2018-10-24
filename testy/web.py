#!/usr/bin/env python

from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, unquote, parse_qs
from fake_useragent import UserAgent
import requests

# HTTPRequestHandler class
class testHTTPServer_RequestHandler(BaseHTTPRequestHandler):

    # GET
    def do_GET(self):
        # Send response status code
        parsed = urlparse(self.path)
        params = parse_qs(parsed.query)


        self.send_response(200)

        # Send headers
        self.send_header('Content-type','image/jpg')
        self.end_headers()

        # Send message back to client
        if(len(params) > 0):
            file = self.get_res(unquote(params['url'][0]))
            # Write content as utf-8 data
            self.wfile.write(file)
        return

    def do_POST(self):
        self.send_response(200)
        self.data_string = self.rfile.read(int(self.headers['Content-Length']))
        self.send_header('Content-type','text/html')
        self.end_headers()
        # Send message back to client
        message = self.post_api(self.data_string);
        # Write content as utf-8 data
        self.wfile.write(bytes(message, "utf8"))

    def post_api(self, payload):
        ua = UserAgent()
        headers = {
            'User-Agent': ua.random
        }
        r = requests.post("https://api.e-hentai.org/api.php", data=payload, headers=headers)
        return r.text;

    def get_res(self, url):
        ua = UserAgent()
        headers = {
            'User-Agent': ua.random
        }
        r = requests.get(url, allow_redirects=True, headers=headers)
        return r.content


def run():
    print('starting server...')

    # Server settings
    # Choose port 8080, for port 80, which is normally used for a http server, you need root access
    server_address = ('0.0.0.0', 5000)
    httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
    print('running server...')
    httpd.serve_forever()


run()
