# prepare for testing
make pkgchecker

# p1t1 test
objs/pkgchecker resources/pkgs/file1.bpkg -all_hashes > ./tests/p1t1/file1.all_hashes
diff tests/p1t1/file1.all_hashes tests/p1t1/file1.all_hashes_exp

# p1t2 test
objs/pkgchecker resources/pkgs/file1.bpkg -hashes_of 4e4dcf5cb1f3cfb33e5b93f760f79fc34a5b627454081f586685b808b972107e > ./tests/p1t2/file1.all_chunks1
diff tests/p1t2/file1.all_chunks1 tests/p1t2/file1.all_chunks_exp1
objs/pkgchecker resources/pkgs/file1.bpkg -hashes_of 0b3c5b3cb770cc33d192d6a3bea451eac2949278fc30b34e678a4428de2d651f > ./tests/p1t2/file1.chunks2
diff tests/p1t2/file1.chunks2 tests/p1t2/file1.chunks_exp2

# p1t3 test
cd tests/p1t3 ; ../../objs/pkgchecker file1.bpkg -chunk_check ; cd ../.. > ./tests/p1t3/file1.completed_chunks1
diff tests/p1t3/file1.completed_chunks1 tests/p1t3/file1.completed_chunks_exp1

# p1t4 test
cd tests/p1t4 ; ../../objs/pkgchecker file1.bpkg -min_hashes ; cd ../.. > ./tests/p1t4/file1.min_completed1
diff tests/p1t4/file1.min_completed1 tests/p1t4/file1.min_completed_exp1

# p1t5 test
cd tests/p1t5 ; ../../objs/pkgchecker file1.bpkg -file_check ; cd ../.. > ./tests/p1t5/file1.status1