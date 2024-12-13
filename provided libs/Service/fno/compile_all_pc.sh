#!/bin/bash

for F in *.pc
do 
	echo "compiling: $(echo $F | cut -d'.' -f1)"
	bash ../../../comp/comp_pro_c $(echo $F | cut -d'.' -f1)
done
