CC=gcc
CFLAGS=-Wall -Wvla -std=c2x -g -fsanitize=address
LDFLAGS=-lm -lpthread
INCLUDE=-Iinclude

.PHONY: clean

# Required for Part 1 - Make sure it outputs a .o file
# to either objs/ or ./
# In your directory
pkgchk.o: src/chk/pkgchk.c
	if [ ! -d "objs" ]; then mkdir objs; fi
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o objs/$@


pkgchecker: src/pkgmain.c src/chk/pkgchk.c src/utils/queue.c src/utils/str.c src/tree/merkletree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o objs/$@

pkgmain: src/pkgmain.c src/chk/pkgchk.c src/utils/queue.c src/utils/str.c src/tree/merkletree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

pkgtest: src/pkgtest.c src/chk/pkgchk.c src/utils/queue.c src/utils/str.c src/tree/merkletree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

# Required for Part 2 - Make sure it outputs `btide` file
# in your directory ./
btide: src/btide.c src/debug/debug.c src/config.c src/peer.c src/package.c src/bpkg.c src/utils/queue.c src/utils/str.c src/tree/merkletree.c src/crypt/sha256.c src/utils/linked_list.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

hpcheck: src/chk/pkgchk.c high_performance/hpcheck.c src/utils/queue.c src/utils/str.c src/tree/merkletree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

hpcheck2: high_performance/bpkg2.c high_performance/hpcheck.c src/utils/queue.c src/utils/str.c high_performance/merkle_tree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

hpcheck3: high_performance/bpkg3.c high_performance/hpcheck.c src/utils/queue.c src/utils/str.c high_performance/merkle_tree.c src/debug/debug.c src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

# Alter your build for p1 tests to build unit-tests for your
# merkle tree, use pkgchk to help with what to test for
# as well as some basic functionality
p1tests:
	bash p1test.sh

# Alter your build for p2 tests to build IO tests
# for your btide client, construct .in/.out files
# and construct a script to help test your client
# You can opt to constructing a program to
# be the tests instead, however please document
# your testing methods
p2tests:
	bash p2test.sh

clean:
	rm -f objs/*
    

