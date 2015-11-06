#!/bin/bash
 
# Tests Bruinbash in a programmatic fashion 
# Quick primer on Bash scripts: http://learnxinyminutes.com/docs/bash/

# Variables are referenced using names and dynamically typed
printf "Test 1\n"
test1=$(./bruinbase << EOF
SELECT * FROM Movie WHERE key > 1000 AND key < 1010 
QUIT)
printf "$test1\n\n"

# Use $() to substitute commands' outputs within commands
# Backticks also do this, but cannot be nested.

printf "Test 2\n"
test2=$(./bruinbase << EOF 
SELECT COUNT(*) FROM Movie
QUIT)
printf "$test2\n\n"

