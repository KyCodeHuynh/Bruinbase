#!/bin/bash

printf "==Testing==: SELECT COUNT\n\n"
test2=$(./bruinbase << END
SELECT COUNT(*) FROM IndexMovie
QUIT
END)
printf "$test2\n\n"