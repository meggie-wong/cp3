IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=build
SDIR=src

LIBS=-lm

_DEPS = proxy.h httpparser.h customsocket.h logger.h mydns.h dns_helper.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = proxy.o httpparser.o customsocket.o logger.o mydns.o dns_helper.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

BINS = proxy nameserver

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: proxy nameserver

proxy: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

nameserver: $(ODIR)/nameserver.o $(ODIR)/logger.o $(ODIR)/dns_helper.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

customsocket: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

httpparser: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(BINS) $(IDIR)/*~ 