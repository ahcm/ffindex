CFLAGS+=-std=c99 -O3 -openmp -g -fPIC
LDFLAGS+=-Wl,-z,defs
#-Wl,--version-script=libffindex.map

ffindex_build: libffindex.so libffindex.so.0.1 ffindex_build.o
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_build.o -o $@ 

libffindex.so.0.1: ffindex.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libffindex.so.0.1 -o libffindex.so.0.1 ffindex.o -lc

libffindex.so:
	ln -s libffindex.so.0.1 libffindex.so

clean:
	rm flatfile

test:
	./flatfile /tmp/1
