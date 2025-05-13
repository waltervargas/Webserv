#!/bin/bash

echo "Running tests for the 'webserv' package"
echo "----------------------------------------"
./client_upload_images
./client_early_disconnect
./malformed_request
./concurrent_requests
echo "----------------------------------------"
echo "✅ All tests finished."