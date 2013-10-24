CC=gcc
CFLAGS=-Wall
SRCS=main.c list.c user.c kernel.c exception.c 
OBJS=$(addsuffix .o, $(basename ${SRCS}))
TARGET=prog

default: $(OBJS)
	$(CC) -ggdb  $(OBJS) -o $(TARGET)

%.o: %.s
	$(CC) -ggdb $(CFLAGS) -o $*.o

%.o: %.c
	$(CC) -ggdb $(CFLAGS) -c $?

clean:
	rm -f *.o $(TARGET)
