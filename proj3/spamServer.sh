#!/bin/bash

./generator > generator.txt

cat generator.txt | while read x; do echo "$x"; sleep 2.5; done |./bankingClient python.cs.rutgers.edu 45632

