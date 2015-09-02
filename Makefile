OS:= $(shell uname)

ifeq ($(OS), Darwin)
MFILE=Makefile.osx
else
MFILE=Makefile
endif

all:
	$(MAKE) -C src -f $(MFILE) $@

%:
	$(MAKE) -C src -f $(MFILE) $@

release:
	git archive HEAD -o ffindex-`cat VERSION`.tar.gz

relnotes:
	git log --pretty=format:" - %s%n%b" 
