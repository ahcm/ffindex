#CC=g++
LANG=C
CFLAGS+=-std=c99 -O3 -openmp -g -fPIC
LDFLAGS+=-Wl,-z,defs
#-Wl,--version-script=libffindex.map

all: ffindex_get ffindex_build

ffindex.o: ffindex.c ffindex.h

ffindex_get: libffindex.so libffindex.so.0.1 ffindex_get.o
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_get.o -o $@ 

ffindex_build: libffindex.so libffindex.so.0.1 ffindex_build.o
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_build.o -o $@ 

libffindex.so.0.1: ffindex.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libffindex.so.0.1 -o libffindex.so.0.1 ffindex.o -lc

libffindex.so: libffindex.so.0.1
	ln -sf libffindex.so.0.1 libffindex.so

test:
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_build -s test.data test.ffindex ../test/data ../test/data2
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  test.data test.ffindex a b foo

time:
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" zsh -c "time ./ffindex_get  ../test_big/nr20_11Jan10_a3m_db ../test_big/nr20_11Jan10_a3m_db.ffindex BABBAMIBA.a3m KARDUZEBA.a3m HARPAQIBA.a3m WOQQATEBA.a3m BEBRAQIBA.a3m GIRSAKEBA.a3m NAQTIFIBA.a3m BIWCIQABA.a3m > /dev/null"

clean:
	rm -f ffindex.o ffindex_build ffindex_build.o ffindex_get ffindex_get.o libffindex.so libffindex.so.0.1

