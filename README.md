# Organisation of the software
All the codes (.c files) are put in folder "src".

All the head files (.h files) are put in folder "include".

All the test related files are put in folder "tests"

folder "high_performance" is a benchmark where multiple threads are used to build a merkle tree in parallel. So that the running time is reduced.

folder "resources" is inherited from the scaffold. I didn't delete them as I am afraid auto tests on ed may need the full scaffold to execute.

file "README.md", "p1test.md" and "p2test.md" are description about the software, the part1 test and the part2 test.

file "config.cfg" and "config2.cfg" are used for part2 testing.

file "find_binary.sh" is used to find binaries.

file "p1test.sh" and "p2test.sh" are test scripts for part1 and part2

# Binaries in the repo
There are binaries in "tests" folder and "resources" folder.

The binaries in "tests" folder are .data files. These are used specially for testing, and there is no way to remove them without influencing the auto-test.

The binaries in "resources" folder are inherited from the scaffold. As mentioned in the first part "Organisation of the software", I don't delete them because I am afraid they may influence the ed auto-test.

These binaries are permitted according the ed post and the coordinator.

# Structure of "src"
As the requirement for part1 and part2 are different. Some of the codes are used for part1, while some others are used for part2.

.c files used in part1:
src/pkgmain.c, src/chk/pkgchk.c, src/utils/queue.c, src/utils/str.c, src/tree/merkletree.c, src/debug/debug.c, src/crypt/sha256.c

.c files used in part2:
src/btide.c, src/debug/debug.c, src/config.c, src/peer.c, src/package.c, src/bpkg.c, src/utils/queue.c, src/utils/str.c, src/tree/merkletree.c, src/crypt/sha256.c, src/utils/linked_list.c

# Structure of "tests"
For details of tests, please refer to "p1test.md" and "p2test.md"

The testing data for part1 is put in folder "tests/data"

As in part2, we have a server-client structure and they hold different data, the testing data for part2 is put in folder "tests/server_data" and "tests/client_data"

The .in files and .exp files are put in "tests/part1" and "tests/part2". They are divided and allocated to different testcases.
