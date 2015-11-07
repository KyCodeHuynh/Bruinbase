#!/bin/bash

printf "Testing: LOAD\n\n"
test3=$(./bruinbase << EOF
LOAD NewMovie FROM 'movie.del')

diff=$(diff movie.tbl newmovie.tbl)
if [ "$diff" != "" ]
    then
    echo "movie.tbl and newmovie.tbl differ"
    exit 1
else
    exit 0
fi
