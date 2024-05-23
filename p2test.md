# Write in the front
When compiling on ED, a warning "implicit declaration of function 'ftruncate'" will occur.
But this warning doesn't affect the operation. I assume it's because c2x can't access ED's library so it can't detect the existence of 'ftruncate'

The test may takes a long time. Because I need to "sleep 1" every time a command is inputted. Otherwise, there may be situations that "CONNECT" was not executed fully, "FETCH" has already been inputted.

# Test 1 - Unit Test: CONNECT
Test if a node can connect to another node
- Testing Command: CONNECT

- Input: Written in tests/part2/test1/node1.in
- Expected: Written in tests/part2/test1/node1.exp

# Test 2 - Unit Test: DISCONNECT
Test if a node can disconnect from another node
- Testing Command: CONNECT, DISCONNECT

In this testcase, if still use .in file to input commands, there may be situations that even command1 was not fully executed, command2 has already been inputted.
So I switch to use pipe file to input commands.
- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test2/node1.exp

# Test 3 - Unit Test: PEERS
Test if a node can print the list of connected peers
- Testing Command: CONNECT, DISCONNECT, PEERS

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test3/node1.exp

# Test 4 - Unit Test: ADDPACKAGE
Test if a node can add a package into management
- Testing Command: ADDPACKAGE, PACKAGES

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test4/node1.exp

# Test 5 - Unit Test: REMPACKAGE
Test if a node can remove a package from management
- Testing Command: ADDPACKAGE, REMPACKAGE, PACKAGES

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test5/node1.exp

# Test 6 - Edge Case: No package (ADD)
Test if a node can handle the situation that the package in ADDPACKAGE \<package\> doesn't exist
- Testing Command: ADDPACKAGE, PACKAGES

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test6/node1.exp

# Test 7 - Edge Case: No package (REM)
Test if a node can handle the situation that the package in REMPACKAGE \<package\> doesn't exist
- Testing Command: REMPACKAGE, PACKAGES

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test7/node1.exp

# Test 8 - Unit Test: FETCH
Test if a node can fetch data from a peers. 
To ensure the data is fetched successfully, an expected .data file is put in tests/part2/test8 folder and it will be compared with the result .data file
- Testing Command: ADDPACKAGE, CONNECT, FETCH

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test8/node1.exp, and in tests/part2/test8/file1.data (expected .data file)

# Test 9 - Edge Case: Connect Failure
Test if a node can detect a connection failure. It should output "Unable to connect to request peer" when such a failure occurs.
- Testing Command: CONNECT, QUIT

- Input: Written in p2test.sh, the commands are written into the pipe file when running
- Expected: Written in tests/part2/test9/node1.exp

