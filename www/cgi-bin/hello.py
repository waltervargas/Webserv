#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

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
