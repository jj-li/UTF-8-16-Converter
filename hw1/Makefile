# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!

IDIR =include/
CC=gcc
CFLAGS=-Wall -Werror -I$(IDIR)
ODIR = build
EXEDIR = bin

_DEPS = const.h map_reduce.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ= main.o map_reduce.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: src/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXEDIR)/mapreduce: $(OBJ)
	@mkdir -p $(EXEDIR)
	gcc -o $@ $^ $(CLFAGS)

all: $(EXEDIR)/mapreduce

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(EXEDIR)/* *~ core $(INCDIR)/*~
