#!/usr/bin/python3

import urllib.request
import sys

print("<html><body><h1>req2.py</h1></body></html>")

# print("now I will send", file=sys.stderr)

with urllib.request.urlopen("http://localhost:8080/cgi-bin/hello.py") as response:
    print("I sent", file=sys.stderr)
    data = response.read().decode()
    print(data)
