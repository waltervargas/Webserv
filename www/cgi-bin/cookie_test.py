import sys
import os

cookie = os.environ.get("HTTP_COOKIE", "")
query = os.environ.get("QUERY_STRING", "")
headers = []

if "reset=1" in query:
    headers.append("Set-Cookie: visited=; Path=/; Max-Age=0")  # clear cookie
elif "visited=yes" not in cookie:
    headers.append("Set-Cookie: visited=yes; Path=/; Max-Age=3600")
headers.append("Content-Type: text/html; charset=utf-8")

# Debug print to stderr (safe)
print("DEBUG (headers):", repr(headers), file=sys.stderr)

# Correct headers with exact CRLF and blank line
for h in headers:
    sys.stdout.write(h + "\r\n")
sys.stdout.write("\r\n")  # End of headers (CRLF-only line)

# Body
sys.stdout.write(f"""<html>
  <body style="background-color: #fcd8f7; font-family: Arial, sans-serif; padding-top: 100px;">
    <h1>{'Welcome back!' if 'visited=yes' in cookie else 'Setting cookie!'}</h1>
      <div style="margin-top: 20px;">
        <form action="/cgi-bin/cookie_test.py" method="get">
          <input type="hidden" name="reset" value="1">
          <button type="submit">🧹 Clear Cookie</button>
        </form>
      </div>
      <div style="margin-top: 20px;">
        <form action="/form.html" method="get">
          <button type="submit">🔙 Back to Forms</button>
        </form>
      </div>
  </body>
</html>""")
