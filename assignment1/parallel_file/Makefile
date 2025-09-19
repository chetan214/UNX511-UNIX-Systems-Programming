CC = gcc
CFLAGS = -Wall -g
OBJS = main.o binary_generator.o worker.o logger.o signals.o

all: program

program: $(OBJS)
	$(CC) $(CFLAGS) -o program $(OBJS)

main.o: main.c logger.h
binary_generator.o: binary_generator.c
worker.o: worker.c
logger.o: logger.c logger.h
signals.o: signals.c logger.h

clean:
	rm -f *.o program *.bin syslog.log