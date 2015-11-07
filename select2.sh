#!/bin/bash

printf "===Testing===: SELECT COUNT\n\n"
test2=$(./bruinbase << EOF
SELECT COUNT(*) FROM Movie
QUIT
EOF)
printf "$test2\n\n"