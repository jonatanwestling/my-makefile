#Makefile for mmake.c, mmake.h, parser.c and parser.h
CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

SRC1 = mmake.c
SRC2 = parser.c 
OUT1 = mmake.o
OUT2 = parser.o
HDR1 = mmake.h
HDR2 = parser.h 
EXEC = mmake

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OUT1) $(OUT2)
	$(CC) $(CFLAGS) -o $(EXEC) $(OUT1) $(OUT2)

$(OUT1): $(SRC1) $(HDR1) $(HDR2)
	$(CC) $(CFLAGS) -c $(SRC1) -o $(OUT1)

$(OUT2): $(SRC2) $(HDR2)
	$(CC) $(CFLAGS) -c $(SRC2) -o $(OUT2)

clean:
	rm -f $(EXEC) A.test B.test C.test D.test SHARED.test ABC.test