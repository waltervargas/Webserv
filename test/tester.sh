#!/bin/bash

echo "🚀 Launching 5 clients simultaneously..."

#launch clients in parallel
pids=()
for i in {1..5}; do
	test/testClient & #run in background
	pids+=($!)
done

#wait for all checks and statuses
client_failures=0
for i in "${!pids[@]}"; do
	pid=${pids[$i]}
	wait $pid
	status=$?
	if [ $status -ne 0 ]; then
		echo "❌ Client $(i + 1) (PID $pid) failed with exit code $status"
		((client_failures++))
	fi
done

echo ""
echo "Running edge case tests"
echo "==========================="

edge_case_failures=0
run_edge_case() {
	name="$1"
	command="$2"
	expected="$3"

	echo -e "\n▶️ $name"
	echo "ours:" 
	if eval "$command"; then
		echo "expected: "
		echo "$expected"
	else
		echo "❌ Edge case '$name' failed to execute"
		((edge_case_failures++))
	fi
}

run_edge_case "testDisconnectNoFileSize" "./test/testDisconnectNoFileSize" "🚫 Disconnecting before sending file size..."
run_edge_case "testDisconnectMidSend" "./test/testDisconnectMidSend" "🚫 Disconnecting halfway through file..."
run_edge_case "testWrongLengthFile" "./test/testWrongLengthFile" "💣 Sent bad file size..."

echo ""
echo "Running functional tests (curl-based)"
echo "==========================="

total=0
passed=0

run_test() {
	name="$1"
	command="$2"
	((total++))
	echo -n "▶️ $name... "
	if eval "$command" > /dev/null 2>&1; then
		echo "✅  PASSED"
		((passed++))
	else
		echo "❌ FAILED"
	fi
}

run_test "Get root index"     "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/ | grep -q 200"
run_test "Upload file"        "curl -s -F 'file=@test/test.txt' http://localhost:8080/upload | grep -q 'File saved'"
run_test "Static asset"       "curl -s http://localhost:8080/static/style.css | grep -q 'body'"
run_test "Error 404"          "curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/notfound | grep -q 404"
run_test "CGI Python script"  "curl -s http://localhost:8080/cgi-bin/hello.py | grep -q 'Hello, World'"

echo ""
echo "==========================="
echo "🧪 Summary"
echo "==========================="

if [ $client_failures -eq 0 ]; then
	echo "✅ All client connections succeeded."
else
	echo "❌ $client_failures client(s) failed."
fi

if [ $edge_case_failures -eq 0 ]; then
	echo "✅ All edge case tests passed."
else
	echo "❌ $edge_case_failures edge case test(s) failed."
fi

if [ $passed -eq $total ]; then
	echo "✅ All $total functional tests passed!"
else
	echo "❌ $((total - passed)) of $total tests failed."
fi