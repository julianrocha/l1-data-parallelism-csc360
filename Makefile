CC := gcc
CFLAGS := -Wall -pthread

all: dataPar

diningOut: dataPar.c
	$(CC) $(CFLAGS) -o dataPar dataPar.c
