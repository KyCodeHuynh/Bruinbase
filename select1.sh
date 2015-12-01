#!/bin/bash

printf "==Testing==: SELECT\n\n"
test1=$(./bruinbase << END
SELECT * FROM IndexMovie WHERE key > 1000 AND key < 1010 
QUIT
END)
printf "$test1\n\n"