CC = gcc
SRC = ./src/utfconverter.c
INCLUDE = ./include/
EXEDIR = bin
BIN = $(EXEDIR)/utf
CFLAGS = -g -Wall -Werror -pedantic -Wextra -I$(INCLUDE)
REQ = $(SRC) include/*.h
ODIR = build
OBJ = $(ODIR)/utfconverter.o
DEPS = include/utfconverter.h

all: $(BIN)

$(ODIR)/utfconverter.o: src/utfconverter.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $(ODIR)/utfconverter.o src/utfconverter.c $(CFLAGS)

$(BIN): $(OBJ)
	@mkdir -p $(EXEDIR)
	gcc -o $(BIN) $(OBJ) $(CLFAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BIN) *~ core $(INCDIR)/*~
	rmdir $(ODIR) $(EXEDIR)

