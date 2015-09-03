all:
	$(MAKE) -C src $@

%:
	$(MAKE) -C src $@

release:
	git archive HEAD --prefix "ffindex-`cat VERSION`/" -o "ffindex-`cat VERSION`.tar.gz"

relnotes:
	git log --pretty=format:" - %s%n%b" 
