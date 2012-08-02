UPCC=upcc
UPCRUN=upcrun
MPIRUN=mpirun
GASNETRUN=gasnetrun_ibv
PYTHON=python
GNUPLOT=gnuplot
PYTHONPATH=../../..

default: all

all: graph.png

graph.png: py.out upc.out mpi.fast.out mpi.slow.out
	$(GNUPLOT) -persist bw.gplot

py.out: pp.py
	@echo Running Python benchmark
	$(GASNETRUN) -n 2 $(PYTHON) $^ --max-exp=22 | tee $@

upc.out: pp
	@echo Running UPC benchmark
	$(UPCRUN) -q -n 2 -shared-heap 2GB $^ | tee $@

mpi.fast.out: pp.mpi.py
	$(MPIRUN) -n 2 $(PYTHON) $^ | tee $@

mpi.slow.out: pp.mpi.py
	$(MPIRUN) -n 2 $(PYTHON) $^ --slow | tee $@

pp: pp.upc
	@echo Compiling UPC benchmark
	$(UPCC) -O -o $@ $^

.PHONY: clean clean-out

clean-out:
	rm -rf *out

clean:
	rm -rf *out pp graph.png
