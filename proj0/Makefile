all: simpleCSVsorter.c mergesort.o
	gcc -o simpleCSVsorter simpleCSVsorter.c mergesort.o

mergesort.o: mergesort.c
	gcc -c mergesort.c

clean:
	rm -f simpleCSVsorter
	rm -f mergesort.o

