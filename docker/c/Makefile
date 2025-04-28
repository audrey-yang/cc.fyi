ccrun: main.o containerize.o dockerhub.o
	gcc -g -Wall -lcurl main.o containerize.o dockerhub.o -o ccrun

dockerhub.o: dockerhub.c dockerhub.h
	gcc -g -Wall -lcurl -c dockerhub.c

containerize.o: containerize.c containerize.h
	gcc -g -Wall -c containerize.c

main.o: main.c
	gcc -g -Wall -c main.c

clean:
	rm ccrun *.o

.PHONY: clean