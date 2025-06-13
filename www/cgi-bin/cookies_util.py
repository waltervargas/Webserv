import os

def get_cookies():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    cookies = {}
    for item in cookie_header.split(";"):
        if "=" in item:
            key, value = item.split("=", 1)
#            cookies[key] = value
            cookies[key.strip()] = value.strip()
    return cookies

def set_cookie_header(key, value, path="/", max_age=None):
    cookie = f"Set-Cookie: {key}={value}; Path={path}"
    if max_age is not None:
        cookie += f"; Max-Age={max_age}"
    return cookie