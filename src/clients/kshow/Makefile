CC=gcc
LIBS=-L/usr/local/lib -ljack -lfftw3 -lm 
CFLAGS=-O3 -I /opt/local/include -I /sw/include/ -std=gnu99
TARGETS=fft.o

all: kleitshow

kleitshow: $(TARGETS)
	$(CC) $(LIBS) $(TARGETS) -o $@

.o: $*.c
	$(CC) $(LIBS) $(CFLAGS) $< -o $%

clean:
	rm kleitshow
	rm *.o
