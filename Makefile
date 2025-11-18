all: run

run:
	g++ -g -O0 input_gen.cpp -o gen
	./gen 100 1 20
	g++ -g -O0 main.cpp -o main
	./main 1 20 L

# ----------- Параметры экспериментов ---------
GENERATOR = ./gen
PROGRAM = ./main
JOBS_SEQ = 14000 15000 16000 17000
JOBS_PAR = 14000
MIN_TIME = 1
MAX_TIME = 100
M_SEQ = 2 8 16 20
NPROCS = 2 4 8 16 20
COOLINGS = L

RESULT_CSV = results_seq4.csv
RESULT_PAR = results_par4.csv

# ----------- Исследование последовательного алгоритма -------
sequential: $(RESULT_CSV)

$(RESULT_CSV):
	@echo "Jobs,Processors,Cooling,Run,Time,BestK2" > $@
	@for N in $(JOBS_SEQ); do \
	  $(GENERATOR) $$N $(MIN_TIME) $(MAX_TIME) > jobs.csv;\
	  for M in $(M_SEQ); do \
	    for K in $(COOLINGS); do \
	      for run in 1 2 3 4 5; do \
	        /usr/bin/time -f "$$N,$$M,$$K,$$run,%e" -a -o time.tmp -- \
	          $(PROGRAM) 1 $$M $$K > prog_out.tmp; \
	        # last line STDOUT должно быть вида "Best K2: число"\
	        best=`tail -100 prog_out.tmp | grep 'Best K2:' | awk '{print $$3}'`; \
	        line=`tail -1 time.tmp`; \
	        echo "$$line,$$best" >> $@; \
	      done; \
	    done; \
	  done; \
	done
	@rm -f time.tmp prog_out.tmp

# ----------- Исследование параллельного алгоритма -----------
parallel: $(RESULT_PAR)

$(RESULT_PAR):
	@echo "Nproc,Processors,Cooling,Run,Time,BestK2" > $@
	@for Nproc in $(NPROCS); do \
	  for run in 1; do \
	    $(GENERATOR) $(JOBS_PAR) $(MIN_TIME) $(MAX_TIME) > jobs.csv; \
	    for K in $(COOLINGS); do \
	      for M in 8; do \
	        /usr/bin/time -f "$$Nproc,$$M,$$K,$$run,%e" -a -o time.tmp -- \
	          $(PROGRAM) $$Nproc $$M $$K > prog_out.tmp; \
	        best=`tail -100 prog_out.tmp | grep 'Best K2:' | awk '{print $$3}'`; \
	        line=`tail -1 time.tmp`; \
	        echo "$$line,$$best" >> $@; \
	      done; \
	    done; \
	  done; \
	done
	@rm -f time.tmp prog_out.tmp

# ----------- Утилиты -----------

.PHONY: sequential parallel