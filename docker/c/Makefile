ccrun: containerize.o
	gcc -g -Wall main.c containerize.c -o ccrun

containerize.o: containerize.c containerize.h
	gcc -g -Wall -c containerize.c

clean:
	rm ccrun *.o