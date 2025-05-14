#!/bin/bash

echo "🚀 Launching 5 clients simultaneously..."

for i in {1..5}
do
	test/testClient & #run in background
	pids+=($!)
done

#wait for all checks and statuses
for i in "${!pids[@]}"; do
	pid=${pids[$i]}
	wait $pid
	status=$?
	if [ $status -ne 0 ]; then
		echo "❌ Client $(i + 1) (PID $pid) failed with exit code $status"
	fi
done

echo ""
echo "Running edge case tests"
echo "==========================="
echo -e "\n▶️ testDisconnectNoFileSize"
echo "ours:" 
./test/testDisconnectNoFileSize
echo "expected:"
echo "🚫 Disconnecting before sending file size..."

echo "==========================="
echo -e "\n▶️ testDisconnectMidSend"
echo "ours:"
./test/testDisconnectMidSend
echo "expected:"
echo "🚫 Disconnecting halfway through file...";

echo "==========================="
echo -e "\n▶️ testWrongLengthFile"
echo "ours:"
./test/testWrongLengthFile
echo "expected:"
echo "💣 Sent bad file size...";

echo ""
echo "==========================="
echo "✅ All tests finished."