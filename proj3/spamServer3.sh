#!/bin/bash

./generator > generator3.txt

cat generator3.txt | while read x; do echo "$x"; sleep 2.5; done |./bankingClient python.cs.rutgers.edu 45632
