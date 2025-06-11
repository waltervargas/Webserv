#!/bin/bash

PORT=8081

echo "🔍 Testing HTTP status codes..."

#200 OK
echo -e "\\n▶️ 200 OK"
curl -i http://localhost:$PORT/

#201 Created
echo -e "\\n▶️ 201 created (via POST)"
curl -i -X POST -F "file=@test.txt" http://localhost:$PORT/upload/

#202 No content
echo -e "\\n▶️ 204 No Content (DELETE)"
curl -i -X DELETE http://localhost:$PORT/deletable.txt
#ensure this is available in www folder for testing: 
#echo "Delete me" > www/deletable.txt


#301 Moved Permanently
echo -e "\\n▶️ 301 Moved Permanently"
curl -i http://localhost:$PORT/old

#302 Found
echo -e "\\n▶️ 302 Found"
curl -i http://localhost:$PORT/temp

#400 Bad Request
echo -e "\\n▶️ 400 Bad Request"
printf 'GET BAD_REQUEST\\r\\n\\r\\n' | nc localhost $PORT

#401 Unauthorized
echo -e "\\n▶️ 401 Unauthorized"
curl -i http://localhost:$PORT/protected

#403 Forbidden
echo -e "\\n▶️ 403 Forbidden"
curl -i http://localhost:$PORT/forbidden

#404 Not Found
echo -e "\\n▶️ 404 Not Found"
curl -i http://localhost:$PORT/missing.html

#413 Payload too large
#dd copies and converts raw data
#if=/dev/zero special file that outputs an infinite number of zero bytes/null chars
#of=big.txt outputs the file to big.txt
#bs = block size
#count = how many blocks to write, I limited it to 2
echo -e "\\n▶️ 413 Payload too large"
dd if=/dev/zero of=big.txt bs=1024 count=2
curl -i -X POST -F "file=@big.txt" http://localhost:$PORT/upload
rm -f big.txt

#500 Internal Server Error
echo -e "\\n▶️ 500 Internal Server ERror"
curl -i http://localhost:$PORT/crash

#501 Not implemented
echo -e "\\n▶️ 501 Not implemented"
printf 'BLAH / HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n' | nc localhost $PORT

#502 Bad Gateway
echo -e "\\n▶️ 502 Bad Gateway"
curl -i http://localhost:$PORT/crash.py
#ensure this is saved in cgi-bin:
#!/usr/bin/env python3
#import sys
#sys.exit(1)  # simulate a crash

#503 Service Unavailable
echo -e "\\n▶️ 503 Service Unavailable"
curl -i http://localhost:$PORT/busy

echo -e "\\n✅ Done testing all listed status codes."