#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs
from cookies_util import get_cookies, set_cookie_header

query = os.environ.get("QUERY_STRING", "")
params = parse_qs(query)
name = params.get("name", [""])[0]
reset = params.get("reset")

#gather headers
headers = []

#set cookie if user submitted name or reset cookie if requested
if name:
    headers.append(set_cookie_header("username", name, path="/", max_age=3600)) #1 hour cookie
if reset:
    headers.append(set_cookie_header("username", "", path="/", max_age=0)) #clear cookie

headers.append("Content-Type: text/html")

#output all headers
for header in headers:
    print(header)
print() #end of headers, this sends \r\n\r\n

cookies = get_cookies()
stored_name = cookies.get("username", "")

print("""
<html>
<head>
    <style>
        body {
            background-color: #fcd8f7;
            font-family: Arial, sans-serif;
            padding-top: 100px;
        }
        input[type="text"] {
            padding: 10px;
            font-size: 16px;
        }
        input[type="submit"] {
            padding: 10px 20px;
            font-size: 16px;
            background-color: #8c52ff;
            border: none;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <h1>CGI Python says hi</h1>
""")

#greeting logic
if name:
    print(f"<h2>Hello, {name}!</h2>")
elif stored_name:
    print(f"<h2>Welcome back, {stored_name}!</h2>")
else:
    print("<h2>First time here? Please enter your name:</h2>")

#if name wasn't submitted yet:
if not name and not stored_name:
    print("""
    <div style="margin-top: 20px;">
        <form method="GET" action="/cgi-bin/hello.py">
            <input type="text" name="name" placeholder="Your name here">
            <input type="submit" value="Submit">
        </form>
    </div>
    """)

print("""
<div style="margin-top: 20px;">
    <a href="/cgi-bin/hello.py?reset=1">🧹 Clear Cookie</a>
</div>
<div style="margin-top: 20px;">
    <a href="/form.html">🔙 Back to main page</a>
</div>
</body>
</html>
""")


