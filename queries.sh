#!/bin/bash
 
 # Tests Bruinbash in a programmatic fashion 
 ./bruinbase << EOF
 SELECT * FROM Movie WHERE key > 1000 AND key < 1010 
 QUIT 