#!/usr/bin/env python3
import sys

print("Content-Type: text/plain\n")

# Read and print POST data from stdin
body = sys.stdin.read()
print("Hello from CGI!")
print(body)