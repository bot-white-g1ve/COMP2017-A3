# Structure
All the testcases are put in tests folder

naming: p1t1 means part 1 testcase 1

p1test.sh and p2test.sh will automatically run all the tests

# p1t1
Parse valid .bpkg file, ensure all the expected hashes read from the file is correct

Related function: bpkg_get_all_hashes

# p1t2
Parse valid .bpkg file, ensure the program can retrieve all the chunk hashes rooted at a specific node

Related function: bpkg_get_all_chunk_hashes_from_hash

# p1t3
Parse valid .bpkg file and read from .data file, ensure the program can verify if every chunk is completed, or if some chunks are incompleted

Related function: bpkg_get_completed_chunks

# p1t5
Parse valid .bpkg file and determine if .data file exists or not. If exists, print out. If not, print out and create

Related function: bpkg_file_check