#!/bin/bash
 
# Tests Bruinbash in a programmatic fashion 
# Quick primer on Bash scripts: http://learnxinyminutes.com/docs/bash/

# Variables are referenced using names and dynamically typed
printf "Test 1\n"
test1=$(./bruinbase << EOF
SELECT * FROM Movie WHERE key > 1000 AND key < 1010 
QUIT)

# Use $() to substitute commands' outputs within commands
# Backticks also do this, but cannot be nested.
response1=$(printf "$test1")

# Variables' *values* are referenced by putting '$' first
answer1=$(printf "Bruinbase> 1008 'Death Machine'
1002 'Deadly Voyage'
1004 'Dear God'
1003 'Deal of a Lifetime'
1009 'Death to Smoochy'
Bruinbase>")

echo $response1 > ./tests/response.txt
echo $answer1 > ./tests/answer.txt
echo $(diff -u ./tests/response.txt ./tests/answer.txt)

# Debugging only
#printf "Response 1: $response1\n\n"
#printf "Answer 1: $answer1\n"

# See 'man test' for the comparison operators
# '=' is used for strings, '-eq' for numbers, etc.

# Because $response1 is potentially empty, we 
# we must quote it so that empty results come out 
# as [ "" -ne $answer1 ] to avoid syntax errors 
if [ "$response1" != "$answer1" ]
then
    echo "Test 1 failed."
fi

test2=$(./bruinbase << EOF 
SELECT COUNT(*) FROM Movie
QUIT)
response2=$(printf "\n$test1")

