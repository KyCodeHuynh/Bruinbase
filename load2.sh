#!/bin/bash 

# Cannot LOAD into the same table twice, or else output table will
# will have two instances of each loaded movie, 
# since we can only append, and not edit/delete/prevent duplicates.
# Instead, we load into a different file. Later, we'll see if 
# SELECT's on the indexed table are faster.
printf "Testing: LOAD WITH INDEX\n\n"
test4=$(./bruinbase << ''
LOAD IndexMovie FROM 'movie.del' WITH INDEX)

diff=$(diff movie.tbl indexmovie.tbl)
if [ "$diff" != "" ]
    then
    echo "movie.tbl and indexmovie.tbl differ"
    exit 1
else
    exit 0
fi
