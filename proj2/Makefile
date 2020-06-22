all: multiThreadSorter_thread.c mergesort.o
	gcc -g -o multiThreadSorter -pthread -fsanitize=address multiThreadSorter_thread.c mergesort.o

mergesort.o: mergesort.c
	gcc -g -c mergesort.c

clean:
	rm -f multiThreadSorter
	rm -f mergesort.o
