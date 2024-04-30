#!/bin/bash

# Iterate over commits in the master branch
git rev-list --reverse master | while read commit; do
    # Check out the commit
    git checkout $commit &> /dev/null

    # Check for binary files in the current commit
    find . -type f -not -path "./.git/*" | xargs file --mime-type | grep "application"
done | sort | uniq -c # sort and unique file names

# Go back to the original branch
git checkout master &> /dev/null
