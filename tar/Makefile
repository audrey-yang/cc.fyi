PROG=cctar
CC = clang
CFLAGS = -Wall -Werror -g

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(PROG) : $(OBJS)
	$(CC) -o $@ $^ 
