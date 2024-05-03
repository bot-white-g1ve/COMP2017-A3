# prepare for testing
make pkgchecker

# p1t1 test
objs/pkgchecker resources/pkgs/file1.bpkg -all_hashes > ./tests/p1t1/file1.all_hashes
diff tests/p1t1/file1.all_hashes tests/p1t1/file1.all_hashes_exp

# p1t2 test
objs/pkgchecker resources/pkgs/file1.bpkg -hashes_of 8d26a4b8b8c566e6dd33d6ac273bba1c15c9538f053554fc0e40b7eed8b902dc > ./tests/p1t2/file1.all_chunks1
diff tests/p1t2/file1.all_chunks1 tests/p1t2/file1.all_chunks_exp1
objs/pkgchecker resources/pkgs/file1.bpkg -hashes_of 46f9db9c0951037a119545595f33cc025b6ed99a752ff15930e1a462862f30c7 > ./tests/p1t2/file1.chunks2
diff tests/p1t2/file1.chunks2 tests/p1t2/file1.chunks_exp2