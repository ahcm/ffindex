CFLAGS+=-std=c99 -O3 -openmp -g -fPIC
LDFLAGS+=-Wl,-z,defs
#-Wl,--version-script=libffindex.map

all: ffindex_get ffindex_build

ffindex_get: libffindex.so libffindex.so.0.1 ffindex_get.o
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_get.o -o $@ 

ffindex_build: libffindex.so libffindex.so.0.1 ffindex_build.o
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_build.o -o $@ 

libffindex.so.0.1: ffindex.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libffindex.so.0.1 -o libffindex.so.0.1 ffindex.o -lc

libffindex.so:
	ln -s libffindex.so.0.1 libffindex.so

test:
	LD_LIBRARY_PATH=. ./ffindex_build test.data test.ffindex ../test/data
	LD_LIBRARY_PATH=. ./ffindex_get  test.data test.ffindex a b

clean:
	rm ffindex.o ffindex_build ffindex_build.o ffindex_get ffindex_get.o libffindex.so libffindex.so.0.1

