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
      cookies = ""

      # Copy cookies from client to server
      if 'Cookie' in self.headers:
        cookies = self.headers.get('Cookie')

      self.send_response(200)

      # Send headers
      self.send_header('Content-type','image/jpg')

      # Send message back to client
      if(len(params) > 0):
        # Special editions for these domains
        file, cookies = self.get_res(unquote(params['url'][0]), cookies, self.headers)
        # Write content as utf-8 data

        # Copy cookie sets
        for k, v in cookies.items():
          self.send_header('Set-Cookie', "{}={}".format(k,v))

        self.end_headers()

        self.wfile.write(file)
      return

    # /?url=<formatted_url>
    def do_POST(self):
      parsed = urlparse(self.path)
      params = parse_qs(parsed.query);
      self.data_string = self.rfile.read(int(self.headers['Content-Length']))
      payload = eval(self.data_string.decode("utf-8"))


      # JSON requests don't need decoded, pass normally
      if('method' in payload):
        response, cookies = self.post(self.data_string, unquote(params['url'][0]), self.headers)
      else:
        response, cookies = self.post(payload, unquote(params['url'][0]), self.headers)

      self.send_response(200)
      self.send_header('Content-type', 'application/json')
      
      # Copy cookie sets
      for k, v in cookies.items():
        self.send_header('Set-Cookie', "{}={}".format(k,v))

      self.end_headers()

      # Write content as utf-8 data
      self.wfile.write(bytes(response.text, "utf8"))


    def post(self, payload, url, h):
      ua = UserAgent()
      headers = {
          'User-Agent': ua.random
      }
      if "Content-Type" in h:
        headers["Content-Type"] = h["Content-Type"];
      if "Accept" in h:
        headers["Accept"] = h["Accept"];
      if "Authorization" in h:
        print("Auth - {}".format(h["Authorization"]))
        headers["Authorization"] = h["Authorization"];

      s = requests.session()
      r = s.post(url=url, data=payload, headers=headers) 
      return r, s.cookies

    def get_res(self, url, cookies, h):
      ua = UserAgent()
      headers = {
          'User-Agent': ua.random
      }
      if "Content-Type" in h:
        headers["Content-Type"] = h["Content-Type"];
      if "Accept" in h:
        headers["Accept"] = h["Accept"];
      if "Authorization" in h:
        print("Auth - {}".format(h["Authorization"]))
        headers["Authorization"] = h["Authorization"];
      if len(cookies) > 0:
        headers["Cookie"] = cookies

      s = requests.session()
      r = s.get(url=url, allow_redirects=True, headers=headers)
      return r.content, s.cookies


def run():
  print('starting server...')

  # Server settings
  # Choose port 8080, for port 80, which is normally used for a http server, you need root access
  server_address = ('0.0.0.0', 5000)
  httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
  print('running server...')
  httpd.serve_forever()


run()
