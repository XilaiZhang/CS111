#!/bin/bash -e
python3 lab3b.py P3B-test_"$1".csv > my"$1".out
diff my"$1".out P3B-test_"$1".err > temp.txt

if [ ! -s temp.txt ] #https://www.tldp.org/LDP/abs/html/fto.html
then
    echo "test $1 passed, no need to output"
    rm temp.txt

else
    echo mine
    cat my"$1".out
    echo
    echo their
    cat P3B-test_"$1".err
    echo
    echo diff
    diff my"$1".out P3B-test_"$1".err
    echo enddiff
fi
