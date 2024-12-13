#!/bin/bash

for F in *.c
do 
	echo "compiling: $(echo $F | cut -d'.' -f1)"
	bash ../../../comp/comp_c $(echo $F | cut -d'.' -f1)
done
