#!/bin/bash

printf "===Testing===: SELECT\n\n"
test1=$(./bruinbase << ''
SELECT * FROM Movie WHERE key > 1000 AND key < 1010 
QUIT)
printf "$test1\n\n"