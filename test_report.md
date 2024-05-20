** Testcase for Merkle Tree and .bpkg file format **

# Author
SID: 520627482

# Date
Sun 12 May 2024 22:04:41 AEDT

# AI used (yes/no)
No. All the test cases are written by myself

# Test 1 Description - Unit Test: Parse .bpkg file
Testing if the .bpkg file is parsed correctly by function 'bpkg_load'. Expectedly, 'bpkg_load' should read from .bpkg file, create a bpkg_obj object which stores all the necessary information (including a merkle tree, but it is not tested for this testcase).
- Testing point: .bpkg file parsing

- The input: a .bpkg file, an expected bpkg_obj type object
- Expected: The output of function 'bpkg_load' is the same as the expected bpkg_obj type object

Test Script: Run pkgtest with argument '-load_bpkg', redirect the output to .out file and compare with .exp

# Test 2 Description - Unit Test: Existing .data file
Testing if after parsing the .bpkg file, our program can correctly identify the existing .data file by function 'bpkg_file_check'. 'bpkg_file_check' will attempt to access the corresponding data file specified by .bpkg file, and check if "it exists" or "it is missing". It is related to .bpkg file parsing.
- Testing point: .bpkg file parsing, .bpkg file checking

- The input: a .bpkg file, a .data file
- Expected: The query should return "File Exists"

Test Script: Run pkgtest with argument '-file_check', with the .data file existing, redirect the output

# Test 3 Description - Unit Test: Create missing .data file
Testing if after parsing the .bpkg file, our program can correctly create the missing .data file with correct size by function 'bpkg_file_check'. When the data file is missing, 'bpkg_file_check' should create the .data file, and the size of such file should be consistent with the size specified in .bpkg file (filled with 0). This is also part of .bpkg parsing.
- Testing point: .bpkg file parsing, .bpkg file checking

- The input: a .bpkg file
- Expected: The query should return "File Created", and the file with correct size should be created

Test Script: Run pkgtest with argument '-file_check', without the .data file. Then delete the created .data file

# Test 4 Description - Edge Case: Size-mismatched file
Testing if after parsing the .bpkg file, our program can correctly identify if the size of .data file is inconsistent with the field in .bpkg file. If the .data file's size is inconsistent, our program should append 0 or delete contents so the size of .data file becomes consistent with the size specified in .bpkg file.
- Testing point: .bpkg file parsing, .bpkg file

- The input: a .bpkg file, a corrupted .data file
- Expected: The query should return "File Exists", and the file should be resized to align with the size in the .bpkg file

Test script: Copying the size-mismatched .data file to current testing dir. Then run pkgtest with argument '-file_check', with the size-mismatched .data file. Then delete the .data file

# Test 5 Description - End-to-End Test: Get all hashes
Testing if our program can correctly get all the expected hashes from the merkle tree and the expected hashes in the merkle tree is consistent with the hashes in .bpkg file. Our program will: 1. parse .bpkg file, 2. build a merkle tree, 3. get all the expected hashes.
- Testing point: .bpkg file, merkle tree building, merkle tree traversing

- The input: a .bpkg file, an expected .exp file
- Expected: The output of printing the query should be same as the content in .exp file

Test Script: Run pkgtest with argument '-all_hashes', redirect the output to .out file and compare with .exp

# Test 6 Description - End-to-End Test: Generate computed hashes
Testing if our program can read from the .data file, and insert the computed hashes into the merkle tree. Our program will: 1. parse .bpkg file, 2. build a merkle tree, 3. read the .data file, 4. compute the computed hashes and insert
- Testing point: merkle tree building, merkle tree computing

- The input: a .bpkg file, a .data file, an expected merkle_tree type object
- Expected: The merkle_tree output after call bpkg_load, check_chunks_completed, check_tree_completed should be the same as the expected merkle_tree

Test Script: Run pkgtest with argument '-all_hashes_com' and the .data file path, redirect the output to .out file and compare with .exp

# Test 7 Description - End-to-End Test: Get all chunks completed
Testing if our program can load .bpkg file, read from .data file and print every chunk that is completed. Our program will: 1. parse .bpkg file, 2. build a merkle tree, 3. read the .data file, 4. compute the computed hashes, 5. check if the computed hashes are consistent with the expected hashes
- Testing point: merkle tree building, merkle tree computing, comparing in merkle tree

- The input: a .bpkg file, a .data file with one chunk corrupted, an expected .exp file
- Expected: The output of printing the query should be the same as the content in .exp file

Test Script: Run pkgtest with arguments '-all_chunks_com' and the .data file path, redirect the output to .out file and compare with .exp

# Test 8 Description - Unit Test: Get min hashes completed
Testing if our program can load .bpkg file, read from .data file and print the mininum of hashes to represented the current completion state (which is the smallest set of hashes of completed branches to represent the completion state of the file)
- Testing point: merkle tree building, merkle tree computing, comparing in merkle tree, merkle tree traversing

- The input: a .bpkg file, a .data file with one chunk corrupted, an expected .exp file
- Expected: The output of printing the query should be the same as the content in .exp file

Test Script: Run pkgtest with arguments '-min_hashes', the .data file is already put in the target dir. The output will be redirected to .out file and compare with .exp

# Test 9 Description - Edge Case: Chunks with different size
Testing if our .data file is divided into chunks with different size, can our program still successfully get all chunks completed. This is related to make the merkle tree flexible, as each node can represent a chunk with different size
- Testing point: merkle tree building, merkle tree computing, comparing in merkle tree

- The input: a .data file with chunks divided into different size, a .bpkg file corresponding to the .data file, an expected .exp file
- Expected: The output of printing the query should be the same as the content in .exp file

Test Script: Run pkgtest with arguments '-add_chunks_com' and the .data file. But in .bpkg file, the size of every chunk could be different. The output will be redirected to .out file and compare with .exp

# Test 10 Description - Unit Test: Get hashes below a node
Testing if our program is able to get all hashes rooted below a specific node from a merkle tree. The node's expected hash will be given, and the program should successfully find which nodes in the merkle tree is a descendnt of such node.
- Testing point: merkle tree building, merkle tree traversing

- The input: a .bpkg file, a node's expected hash, an expected .exp file
- Expected: The output of printing the query should be the same as the content in .exp file

Test Script: Run pkgtest with arguments '-hashes_of' and the target hash. Two cases will be tested, one with the target hash being the root, another with the target hash being the last hash in the non-leaf nodes. The output will be redirected to .out file and compare with .exp

# Test 11 Description - Edge Case: Bad bpkg format
Testing if our program is able to identify the input .bpkg file with wrong format. A bad .bpkg file will be given, and the function 'bpkg_load' should return NULL
- Testing point: .bpkg file parsing
  
- The input: a .bpkg file with wrong format
- Expected: The output of printing the query should be "Unable to load pkg and tree"

Test Script: Run pkgtest with argument '-load_bpkg', the given .bpkg file doesn't contain all necessary fields. The output will be redirected to .out file and compare with .exp

# Test 12 Description - Edge Case: Incomplete .data file
Testing if our program is able to find out incomplete data chunks from .data file while parsing the .bpkg file. A .data with 3 incomplete chunks will be given. By default, our program should only return the chunks that are completed. This test is similiar to Test 7 but is a bit different
- Testing point: .bpkg file parsing, merkle tree building, merkle tree computing, comparing in merkle tree

- The input: a .bpkg file, a .data file with 3 incomplete chunks, an expected .exp file
- Expected: The output of printing the query should be the same as the content in .exp file

Test Script: Copy the incomplete .data file into the test directory. Run pkgtest with argument '-chunk_check'. The output will be redirected to .out file and compare with .exp

# Testing script description
I have compiled pkgtest in folder 'tests/bin', it includes pkgchk.c and runs its functions. Then it will print out the results which will be redirected to .out files, which will be compared against .exp files.

I have included the pkgtest.c in bin folder, but as I am only the tester, I don't know how will the programer write pkgchk.c or other c files, so they are not included. But the compiled pkgtest is included for easy of using (You won't be able to compile them without pkgchk.c or other c files). 

The testing data (.bpkg files and .data files) are put in folder 'tests/data'. I have created these files by myself for testing

# Test 1 Script
The script here is the plain code without redirecting and comparing, the complete test script is in tests/test.sh

tests/bin/pkgtest tests/data/file1.bpkg -load_bpkg

# Test 2 Script
cd tests/data
../bin/pkgtest file1.bpkg -file_check

# Test 3 Script
cd tests/test3
../bin/pkgtest ../data/file1.bpkg -file_check 

# Test 4 Script
cp tests/data/file2.data tests/test4/file2.data
cd tests/test4
../bin/pkgtest ../data/file2.bpkg -file_check 

# Test 5 Script
cd tests/test5
../bin/pkgtest ../data/file1.bpkg -all_hashes

# Test 6 Script
cd tests/test6
../bin/pkgtest ../data/file1.bpkg -all_hashes_com ../data/file1.data

# Test 7 Script
cd tests/test7
../bin/pkgtest ../data/file1.bpkg -all_chunks_com ../data/file1.data

# Test 8 Script
cd tests/data
../bin/pkgtest file1.bpkg -min_hashes

# Test 9 Script
cd tests/test9
../bin/pkgtest ../data/file3.bpkg -all_chunks_com ../data/file1.data 

# Test 10 Script
cd tests/test10
../bin/pkgtest ../data/file1.bpkg -hashes_of 4e4dcf5cb1f3cfb33e5b93f760f79fc34a5b627454081f586685b808b972107e
../bin/pkgtest ../data/file1.bpkg -hashes_of 0b3c5b3cb770cc33d192d6a3bea451eac2949278fc30b34e678a4428de2d651f

# Test 11 Script
cd tests/test11
../bin/pkgtest ../data/file4.bpkg -load_bpkg

# Test 12 Script
cp tests/data/file5.data tests/test12/file5.data
cd tests/test12
../bin/pkgtest ../data/file5.bpkg -chunk_check

# Data Files Description
file1.bpkg: Linked with file1.data, all chunks completed
file2.bpkg: Linked with file2.data, whose size doesn't match
file3.bpkg: Linked with file1.data, but the chunks have been reorganized and assigned different sizes.
file4.bpkg: Some fields missing in the package
file5.bpkg: Linked with file5.data, some chunks incomplete

# Instructions to run testing script
The testing script is put inside 'tests' folder. 

Firstly, set permission for test.sh

```bash
chmod 755 tests/test.sh
chmod 755 tests/bin/pkgtest
```

Then, run test.sh

```bash
bash test.sh
```
