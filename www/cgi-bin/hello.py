#!/usr/bin/env python3

import os
from PIL import Image
from urllib.parse import parse_qs



#get the abosolute path to www/favicon.ico relative to this script
www_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
favicon_path = os.path.join(www_dir, "favicon.ico")

#check if favicon already exists, browsers automatically send a request
#to /favicon.ico when loading a site, we need a basic .ico (.ico is legacy default
#behavior of browsers that suppor backward compatibility)
if not os.path.exists("../favicon.ico"):
    img = Image.new("RGBA", (32, 32), (252, 216, 247, 255))
    # Open your PNG or create a blank one
    img.save(favicon_path, format="ICO")

query = os.environ.get("QUERY_STRING", "")
params = parse_qs(query)
name = params.get("name", [""])[0]

print("Content-Type: text/html\r\n\r\n")

print("""
<html>
<head>
    <style>
        body {
            background-color: #fcd8f7;
            font-family: Arial, sans-serif;
            padding-top: 100px;
        }
    </style>
</head>
<h1>CGI Python says hi</h1>
<h2>Enter Your Name</h2>
    <form method="GET" action="/cgi-bin/hello.py">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name">
        <input type="submit" value="Submit">
    </form>
""")

if name:
    print(f"<h2>Hello, {name}!</h2>")

print("</body></html>")
