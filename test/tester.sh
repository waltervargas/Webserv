#!/bin/bash

echo "🚀 Launching 5 clients simultaneously..."

for i in {1..5}
do
	test/testClient & #run in background
	pid=$!
	wait $pid
	status=$?
	if [ $status -ne 0 ]; then
		echo "❌ Client $i (PID $pid) failed with exit code $status"
	fi
done

echo ""
echo "Running edge case tests"
echo "▶️ testDisconnectNoFileSize"
./test/testDisconnectNoFileSize

echo "▶️ testDisconnectMidSend"
./test/testDisconnectMidSend

echo "▶️ testWrongLengthFile"
./test/testWrongLengthFile

echo "✅ All tests finished."