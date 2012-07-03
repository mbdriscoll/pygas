UPCC=upcc
UPCRUN=upcrun
MPIRUN=mpirun
GASNETRUN=gasnetrun_ibv
PYTHON=python
GNUPLOT=gnuplot
PYTHONPATH=../../..

default: all

all: py.out upc.out graph.png

graph.png: py.out upc.out
	$(GNUPLOT) -persist pp.gplot

py.out: pp.py
	@echo Running Python benchmark
	PYTHONPATH=$(PYTHONPATH) $(GASNETRUN) -n 2 $(PYTHON) $^ | tee $@

upc.out: pp
	@echo Running UPC benchmark
	$(UPCRUN) -q -n 2 -shared-heap 2GB $^ | tee $@

pp: pp.upc
	@echo Compiling UPC benchmark
	$(UPCC) -O -o $@ $^

.PHONY: clean

clean:
	rm -rf py.out upc.out pp graph.png
