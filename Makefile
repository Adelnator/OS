cat:	cat.c
	c99 -Wall -o cat cat.c -L ../lib -lhelpers -Wl,-rpath=../lib -I ../lib
