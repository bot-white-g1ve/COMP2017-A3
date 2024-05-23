#!/bin/bash

make btide

#####

# Test 1
test_count="test1"
./btide config2.cfg &
node2_PID=$!
sleep 1
./btide config.cfg < tests/part2/"$test_count"/node1.in > tests/part2/"$test_count"/node1.out

# Kill node2_PID
kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 2
test_count="test2"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "CONNECT 127.0.0.1:9001" >&3
sleep 1
echo "DISCONNECT 127.0.0.1:9001" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 3
test_count="test3"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "CONNECT 127.0.0.1:9001" >&3
sleep 1
echo "PEERS" >&3
sleep 1
echo "DISCONNECT 127.0.0.1:9001" >&3
sleep 1
echo "PEERS" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 4
test_count="test4"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "ADDPACKAGE file1.bpkg" >&3
sleep 1
echo "PACKAGES" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 5
test_count="test5"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "ADDPACKAGE file1.bpkg" >&3
sleep 1
echo "PACKAGES" >&3
sleep 1
echo "REMPACKAGE abcdfeghbf279694ddb22af800dcaad9" >&3
sleep 1
echo "PACKAGES" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 6
test_count="test6"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "ADDPACKAGE file2.bpkg" >&3
sleep 1
echo "PACKAGES" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 7
test_count="test7"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "REMPACKAGE aaaafeghbf279694ddb22af800dcaaaa" >&3
sleep 1
echo "PACKAGES" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 8
test_count="test8"
./btide config2.cfg < tests/part2/"$test_count"/node2.in &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "ADDPACKAGE file1.bpkg" >&3
sleep 1
echo "CONNECT 127.0.0.1:9001" >&3
sleep 1
echo "FETCH 127.0.0.1:9001 abcdfeghbf279694ddb22af800dcaad9e498ccb8bdf538905537f2f03ccd7965e0dac82751c8968fbb6ae6a126e905ee5 7d61368ccace13deb5f5d7d67ba1195be97507cf5e53cae167d7f4cf5aee181a" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 9
test_count="test9"
./btide config2.cfg &
node2_PID=$!

PIPE_FILE=$(mktemp -u)
mkfifo $PIPE_FILE

./btide config.cfg < $PIPE_FILE > tests/part2/"$test_count"/node1.out &

exec 3>$PIPE_FILE
echo "CONNECT 127.0.0.1:9002" >&3
sleep 1
echo "QUIT" >&3
exec 3>&- # Close the PIPE_FILE

# Remove PIPE_FILE
rm $PIPE_FILE

kill $node2_PID

sleep 1
# Remove debug files
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi
if [ -f "debug_9000.txt" ]; then
    rm "debug_9000.txt" # delete temp files created when program is running
fi
if [ -f "debug_9001.txt" ]; then
    rm "debug_9001.txt" # delete temp files created when program is running
fi

sleep 1 # PIPE need some time to output
# Compare .out with .exp
if diff -u tests/part2/"$test_count"/node1.out tests/part2/"$test_count"/node1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi
