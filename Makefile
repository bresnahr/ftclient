CFLAGS = -Wall -lm

ftserver: ftserver.c
	gcc -o ftserver -g ftserver.c $(CFLAGS)

all: ftserver

clean:
	rm -f *.o ftserver.exe ftserver
