#!/usr/bin/env python3

# -----------------------------------------------------------------------------
# Copyright (c) 2025 Sayan Ray
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Redistribution of this file, in source or binary form, with or without
# modification, is permitted provided that the above notice is retained.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
# OR OTHER DEALINGS IN THE SOFTWARE.
# -----------------------------------------------------------------------------


import os
import sys
import base64
import argparse

from flask import *

app = Flask(__name__)

banner = """
███████╗██╗  ██╗███████╗██╗     ██╗  ██╗   ██╗
██╔════╝██║  ██║██╔════╝██║     ██║  ╚██╗ ██╔╝
███████╗███████║█████╗  ██║     ██║   ╚████╔╝ 
╚════██║██╔══██║██╔══╝  ██║     ██║    ╚██╔╝  
███████║██║  ██║███████╗███████╗███████╗██║   
╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚═╝   
"""
print(banner)

@app.route("/")
# Return the base template
def home():
	return make_response(open(directory+"templates/index.html","r").read())

@app.route("/api", methods = ["POST"])
def api():
	# Get the JSON data
	data = request.get_json()

	# Get the value of the 'file' param
	param_file = data.get("file")

	# Does the file exists?
	# Yes -> Serve it! 
	if os.path.isfile(param_file):
		# Get the file contents and base64 encode them
		contents = open(param_file, "rb").read()
		b64 = base64.b64encode(contents).decode()

		# Make the base response template
		resp = make_response(open(directory+"templates/index.html","r").read())
		# Set the encoded shellcode in the cookie
		resp.set_cookie("sessionId", b64)
		return resp

	# No -> Return a FNF message.
	else:
		resp = make_response("<h1> File Not Found. <h1>")
		return resp

if __name__ == '__main__':
	# Create the argument parser
	parser = argparse.ArgumentParser(description="\033[1m\033[37m\033[4mSHELLY\033[0m - Serve shellcode over the wire.", usage="shelly-server [options]")
	parser.add_argument("--host", metavar="STRING", help="Host to bind to. (default: 0.0.0.0)", default="0.0.0.0")
	parser.add_argument("--port", metavar="INTEGER", help="Port to bind to. (default: 8080)", default=8080, type=int)
	parser.add_argument("--certfile", metavar="FILE", help="The X.509 certificate file for SSL authentication. (default: $installation_dir/auth/cert.pem)", type=argparse.FileType('r'))
	parser.add_argument("--keyfile", metavar="FILE", help="The X.509 certificate's key file for SSL authentication. (default: $installation_dir/auth/key.pem)", type=argparse.FileType('r'))

	args = parser.parse_args()

	# Get the installation directory
	directory = sys.argv[0][:sys.argv[0].rindex('/')+1]
	ssl_context: tuple = (args.certfile.name if args.certfile is not None else directory+"auth/cert.pem", args.keyfile.name if args.keyfile is not None else directory+"auth/key.pem")

	# Run the application
	app.run(host = args.host, port = args.port, debug = False, ssl_context = ssl_context)