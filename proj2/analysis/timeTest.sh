#!/bin/bash

echo "Process Sorter"
time ./sorter -c $1 -d $2 -o output >/dev/null
echo "Thread Sorter"
time ./multiThreadSorter -c $1 -d $2 -o output >/dev/null

chmod +rw output/*
rm output/*
