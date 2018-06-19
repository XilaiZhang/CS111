#!/bin/bash
for i in {1..22}
do 
	echo test number $i
	bash tester.sh $i
	echo
	echo

done
