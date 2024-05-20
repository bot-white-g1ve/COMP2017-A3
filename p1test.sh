# Please refer to descriptions in test_report.md

# Compile the test program
make pkgtest

#####

# Test 1
test_count="test1"

# run the program, redirect the output
./pkgtest tests/data/file1.bpkg -load_bpkg > tests/part1/"$test_count"/file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt" # delete temp files created when program is running
fi

# compare with expected output
if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then 
    # pass
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 2
test_count="test2"

cd tests/data
../../pkgtest file1.bpkg -file_check > ../part1/test2/file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 3
test_count="test3"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file1.bpkg -file_check > file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
if [ -f "file1.data" ]; then
    rm "file1.data"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 4
test_count="test4"

cp tests/data/file2.data tests/part1/"$test_count"/file2.data

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file2.bpkg -file_check > file2.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
if [ -f "file2.data" ]; then
    rm "file2.data"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file2.out tests/part1/"$test_count"/file2.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 5
test_count="test5"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file1.bpkg -all_hashes > file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 6
test_count="test6"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file1.bpkg -all_hashes_com ../../data/file1.data > file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

# Test 7
test_count="test7"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file1.bpkg -all_chunks_com ../../data/file1.data > file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 8
test_count="test8"

cd tests/data
../../pkgtest file1.bpkg -min_hashes > ../part1/test8/file1.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../..

if diff -u tests/part1/"$test_count"/file1.out tests/part1/"$test_count"/file1.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 9
test_count="test9"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file3.bpkg -all_chunks_com ../../data/file1.data > file3.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file3.out tests/part1/"$test_count"/file3.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 10
test_count="test10"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file1.bpkg -hashes_of 560d5c7b9e8466b246b774d640d45e548ddc2368411d81a1286ccbb18e87bd30 > file1.out1
../../../pkgtest ../../data/file1.bpkg -hashes_of d652327ff2885ed2b6c5a6958f910d8f85104e5fd314681762071d22348c55b4 > file1.out2
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file1.out1 tests/part1/"$test_count"/file1.exp1 > /dev/null; then
    # If the first testcase is correct, test the second one
    if diff -u tests/part1/"$test_count"/file1.out2 tests/part1/"$test_count"/file1.exp2 > /dev/null; then
        echo "pass $test_count"
    else
        echo "fail $test_count"
    fi
else
    echo "fail $test_count"
fi

#####

# Test 11
test_count="test11"

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file4.bpkg -load_bpkg > file4.out
# if [ -f "debug_0.txt" ]; then
#     rm "debug_0.txt"
# fi
cd ../../..

if diff -u tests/part1/"$test_count"/file4.out tests/part1/"$test_count"/file4.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

# Test 12
test_count="test12"

cp tests/data/file5.data tests/part1/"$test_count"/file5.data

cd tests/part1/"$test_count"
../../../pkgtest ../../data/file5.bpkg -chunk_check > file5.out
if [ -f "debug_0.txt" ]; then
    rm "debug_0.txt"
fi
if [ -f "file5.data" ]; then
    rm "file5.data"
fi
cd ../../..

if diff -u tests/part1/"$test_count"/file5.out tests/part1/"$test_count"/file5.exp > /dev/null; then
    echo "pass $test_count"
else
    echo "fail $test_count"
fi

#####

rm pkgtest
