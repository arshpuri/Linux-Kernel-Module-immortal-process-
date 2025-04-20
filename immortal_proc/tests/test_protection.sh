#!/bin/bash

# Load module if not loaded
if ! lsmod | grep -q immortal_proc; then
    echo "Loading module..."
    sudo insmod src/immortal_proc.ko
fi

# Start a test process
echo "Starting test process..."
sleep 600 &
TEST_PID=$!

echo "Test process PID: $TEST_PID"

# Add process to protected list
echo "Protecting process..."
echo "+$TEST_PID" > /proc/immortal_proc

# Try to kill it with different signals
echo "Testing SIGTERM protection..."
kill -TERM $TEST_PID
sleep 1

if ps -p $TEST_PID > /dev/null; then
    echo "SIGTERM protection working!"
else
    echo "SIGTERM protection failed!"
    exit 1
fi

echo "Testing SIGKILL protection..."
kill -KILL $TEST_PID
sleep 1

if ps -p $TEST_PID > /dev/null; then
    echo "SIGKILL protection working!"
else
    echo "SIGKILL protection failed!"
    exit 1
fi

# Remove protection
echo "Removing protection..."
echo "-$TEST_PID" > /proc/immortal_proc

# Now kill should work
echo "Testing kill after protection removal..."
kill -KILL $TEST_PID
sleep 1

if ! ps -p $TEST_PID > /dev/null; then
    echo "Protection removal working!"
else
    echo "Protection removal failed!"
    kill -KILL $TEST_PID  # Try one more time
    exit 1
fi

echo "All tests passed!"
