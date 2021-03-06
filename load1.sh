#!/bin/bash

printf "\n==Testing: LOAD==\n"
test3=$(./bruinbase << END
LOAD NewMovie FROM 'movie.del'
END)

diff=$(diff movie.tbl newmovie.tbl)
if [ "$diff" != "" ]
then
    echo "movie.tbl and newmovie.tbl differ"
    exit 1
else
    exit 0
fi
