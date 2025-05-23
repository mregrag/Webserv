#!/usr/bin/python3

import urllib.request
import sys

print("<html><body><h1>Hello, World!</h1></body></html>")

# print("now I will send", file=sys.stderr)

with urllib.request.urlopen("http://localhost:7777/cgi-bin/req.py") as response:
    print("I sent", file=sys.stderr)
    data = response.read().decode()
    print(data)
