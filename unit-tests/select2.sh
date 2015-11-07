#!/bin/bash

printf "===Testing===: SELECT COUNT\n\n"
test2=$(./bruinbase << ''
SELECT COUNT(*) FROM Movie
QUIT)
printf "$test2\n\n"