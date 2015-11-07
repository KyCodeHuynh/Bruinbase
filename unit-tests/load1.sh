#!/bin/bash

printf "Testing: LOAD\n\n"
test3=$(./bruinbase << EOF
LOAD NewMovie FROM 'movie.del')

echo "Before diff"
$diff=$(diff movie.tbl newmovie.tbl)
echo "After diff"   
if [ -n $diff ]
    then
    exit 1
fi
