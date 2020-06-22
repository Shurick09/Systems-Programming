#!/bin/bash

./generator > generator4.txt

cat generator4.txt | while read x; do echo "$x"; sleep 2.5; done |./bankingClient python.cs.rutgers.edu 45632
