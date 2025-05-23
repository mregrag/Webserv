#!/usr/bin/env python3
import cgi
import os
import sys

# Where to store uploaded files
UPLOAD_DIR = '/home/mregrag/Desktop/Web/www/uploads/'

print("Content-Type: text/html\n\n")

form = cgi.FieldStorage()
fileitem = form['file']

if fileitem.filename:
    # Prevent directory traversal
    fn = os.path.basename(fileitem.filename)
    with open(os.path.join(UPLOAD_DIR, fn), 'wb') as f:
        f.write(fileitem.file.read())
    print(f"<h1>File {fn} uploaded successfully!</h1>")
else:
    print("<h1>No file was uploaded</h1>")
