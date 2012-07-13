grep G py.out > test-0.err
grep C py.out > test-1.err
python process.py | sort -n | tee results.out
gnuplot -persist stacked.gplot
