#!/usr/bin/python3

import cgitb

cgitb.enable()

def clear_cookie():
    print("Set-Cookie: id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/")
    print("Status: 302 Found")
    print("Location: /login")
    print("Content-Type: text/html\r\n")

clear_cookie()

