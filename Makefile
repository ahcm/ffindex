OS:= $(shell uname)

ifeq ($(OS), Darwin)
MFILE=Makefile.osx
else
MFILE=Makefile
endif

all:
	cd src ; make -f $(MFILE)

install:
	cd src ; make install

deinstall:
	cd src ; make deinstall

clean:
	cd src ; make clean
