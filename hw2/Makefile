CC = gcc
SRC = ./src/utfconverter.c
INCLUDE = ./include/
BIN = utfconverter
CFLAGS = -g -Wall -Werror -pedantic -Wextra
REQ = $(SRC) include/*.h

all: $(BIN)

$(BIN): $(REQ)
	$(CC) $(CFLAGS) -I $(INCLUDE) $(SRC) -o $(BIN)

.PHONY: clean

clean:
	rm -f *.o $(BIN)

