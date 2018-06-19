# range of threads and iterations to see what it takes to cause a failure

./lab2_add --threads=2  --iterations=100    >> lab2_add.csv

./lab2_add --threads=2  --iterations=1000   >> lab2_add.csv

./lab2_add --threads=2  --iterations=10000  >> lab2_add.csv

./lab2_add --threads=2  --iterations=100000 >> lab2_add.csv

./lab2_add --threads=4  --iterations=100    >> lab2_add.csv

./lab2_add --threads=4  --iterations=1000   >> lab2_add.csv

./lab2_add --threads=4  --iterations=10000  >> lab2_add.csv

./lab2_add --threads=4  --iterations=100000 >> lab2_add.csv

./lab2_add --threads=8  --iterations=100    >> lab2_add.csv

./lab2_add --threads=8  --iterations=1000   >> lab2_add.csv

./lab2_add --threads=8  --iterations=10000  >> lab2_add.csv

./lab2_add --threads=8  --iterations=100000 >> lab2_add.csv

./lab2_add --threads=12 --iterations=100    >> lab2_add.csv

./lab2_add --threads=12 --iterations=1000   >> lab2_add.csv

./lab2_add --threads=12 --iterations=10000  >> lab2_add.csv

./lab2_add --threads=12 --iterations=100000  >> lab2_add.csv
