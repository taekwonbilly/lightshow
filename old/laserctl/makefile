default: laserctl

laserctl: laserctl.c lasercirclemode.c sound.c thread.c \
          laserctl.h lasercirclemode.h sound.h thread.h makefile
	gcc -O4 -Wall -DLINUX -o laserctl laserctl.c lasercirclemode.c sound.c thread.c \
            -lpthread -lm

clean:
	rm -f *.o *~ "#"*"#"

clean_all: clean
	rm -f laserctl

dist: clean_all
	cd ..; tar -cv laserctl | gzip > laserctl.tar.gz

