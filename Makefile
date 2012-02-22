OS:= $(shell uname)

ifeq ($(OS), Darwin)
MFILE=Makefile.osx
else
MFILE=Makefile
endif

.PHONY: clean install deinstall clean
all:
	$(MAKE) -C src -f $(MFILE)

install:
	$(MAKE) -C src install

deinstall:
	$(MAKE) -C src deinstall

clean:
	$(MAKE) -C src clean
