#!/bin/bash

./generator > generator2.txt

cat generator2.txt | while read x; do echo "$x"; sleep 2.5; done |./bankingClient python.cs.rutgers.edu 45632
