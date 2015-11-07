#!/bin/bash
 
# Tests Bruinbash in a programmatic fashion 
# Quick primer on Bash scripts: http://learnxinyminutes.com/docs/bash/

# Variables are referenced using names and dynamically typed
printf "Testing: SELECT\n\n"
printf "Test 1\n"
test1=$(./bruinbase << ''
SELECT * FROM Movie WHERE key > 1000 AND key < 1010 
QUIT)
printf "$test1\n\n"

# Use $() to substitute commands' outputs within commands
# Backticks also do this, but cannot be nested.

printf "Test 2\n"
test2=$(./bruinbase << '' 
SELECT COUNT(*) FROM Movie
QUIT)
printf "$test2\n\n"


printf "Testing: LOAD\n\n"
printf "Test 3\n"
test3=$(./bruinbase << ''
LOAD Movie FROM 'movie.del')
printf "$test3\n\n"

printf "Test 4\n"
test4=$(./bruinbase << ''
LOAD Movie FROM 'movie.del' WITH INDEX)
printf "$test4\n\n"

