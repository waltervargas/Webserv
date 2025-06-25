#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

query = os.environ.get("QUERY_STRING", "")
params = parse_qs(query)
name = params.get("name", [""])[0]

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")
print("<h1>Enter Your Name</h1>")
print("""
    <form method="GET" action="/cgi-bin/hello.py">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name">
        <input type="submit" value="Submit">
    </form>
""")

if name:
    print(f"<h2>Hello, {name}!</h2>")

print("</body></html>")
