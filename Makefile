.PHONY: all run clean

all: run

test1: test.cc include/active_map.h
	g++-7 -I include -pedantic -Wall -o test1 -std=c++17 test.cc

test2: test.cc other/active_map.h
	g++-7 -I other -pedantic -Wall -o test2 -std=c++17 test.cc

run: test1 test2
	@./test1 && echo "PASS"
	@./test2 && echo "PASS"

clean:
	rm -f ./test1
	rm -f ./test2
