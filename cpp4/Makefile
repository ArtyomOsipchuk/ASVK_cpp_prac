all: run

run:
	g++ -g -O0 main.cpp -o main
	./main 0 0

main:
	g++ -g -O0 main.cpp -o main

animate:
	g++ -g -O0 animate.cpp -o animate -lncurses
	./animate series_1_run_1_sol.txt

# ----------- Параметры экспериментов ---------
PROGRAM = ./main
SERIES = 0 1 2 3 4 5 6 7 8 9
RUNS = 0 1 2 3 4
RESULT_CSV = results.csv

# ----------- Исследование --------------------
research: $(RESULT_CSV)

$(RESULT_CSV):
	@echo "Series,Run,Time,Best" > $@
	@for S in $(SERIES); do \
	  for R in $(RUNS); do \
	    OUTPUT=$$($(PROGRAM) $$S $$R 2>&1); \
	    TIME=$$(echo "$$OUTPUT" | awk '{print $$1}'); \
	    BEST=$$(echo "$$OUTPUT" | awk '{print $$2}'); \
	    echo "$$S,$$R,$$TIME,$$BEST" >> $@; \
	  done; \
	done
	@echo "Исследование завершено"