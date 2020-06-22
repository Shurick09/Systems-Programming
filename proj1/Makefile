all: scannerCSVsorter.c mergesort.o
	gcc -g -o sorter -Werror -fsanitize=address scannerCSVsorter.c mergesort.o

mergesort.o: mergesort.c
	gcc -c mergesort.c

clean:
	rm -f sorter
	rm -f mergesort.o
