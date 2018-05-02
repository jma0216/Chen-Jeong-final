sh: sh.o
	g++ -o sh sh.o

sh.o: sh.cpp
	g++ -c sh.cpp -Wall -std=c++14 -g -O0 -pedantic-errors

clean:
	rm sh sh.o
