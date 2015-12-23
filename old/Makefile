# compiler to use
CC = gcc
 
# flags to use always
CCFLAGS = -g -Wall -fPIC -std=c99

# glib include flags 
GLIBINCFLAGS = -I/usr/include/glib-1.2 -I/usr/lib/glib/include 

# flags to build a shared library
SHAREDLIBFLAGS = -shared -lpthread

# directory where xmms visualization plugins are installed
XMMSLIBDIR = /usr/lib/xmms/Visualization/

# location of common src and obj files
COMMONSRC := $(wildcard *.c) 
COMMONOBJ := $(patsubst %.c, %.o, $(COMMONSRC))

# shared library to build
SHAREDLIB = lib23lights-S04.so

all: $(SHAREDLIB)

clean: FORCE
	rm -f $(COMMONOBJ) $(SHAREDLIB)

install: $(SHAREDLIB)
	cp $(SHAREDLIB) $(XMMSLIBDIR)

$(SHAREDLIB): $(COMMONOBJ) 	
	$(CC) $(CCFLAGS) $(GLIBINCFLAGS) $(SHAREDLIBFLAGS) $+ -o $(SHAREDLIB)

$(COMMONOBJ) : $(COMMONSRC)
	$(foreach file, $(COMMONSRC), $(CC) $(CCFLAGS) $(GLIBINCFLAGS) -c $(file) -o $(patsubst %.c, %.o, $(file));)


FORCE:
