INSTALL_DIR?=..
LANG=C
CFLAGS+=-std=c99 -O3 -openmp -g -fPIC -Wall
LDFLAGS+=-Wl,-z,defs
#-Wl,--version-script=libffindex.map

all: ffindex_get ffindex_build ffindex_modify

ffindex.o: ffindex.c ffindex.h fferror.h

ffindex_get: libffindex.so libffindex.so.0.1 ffindex_get.o ffindex.h
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_get.o -o $@ 

ffindex_build: libffindex.so libffindex.so.0.1 ffindex_build.o ffindex.h
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_build.o -o $@ 

ffindex_modify: libffindex.so libffindex.so.0.1 ffindex_modify.o ffindex.h
	$(CC) $(CFLAGS) -L. -lffindex ffindex.o ffindex_modify.o -o $@ 

libffindex.so.0.1: ffindex.o fferror.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libffindex.so.0.1 -o libffindex.so.0.1 ffindex.o fferror.o -lc

libffindex.so: libffindex.so.0.1
	ln -sf libffindex.so.0.1 libffindex.so

test:
	rm /tmp/test.data /tmp/test.ffindex
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_build -s /tmp/test.data /tmp/test.ffindex ../test/data ../test/data2
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /tmp/test.data /tmp/test.ffindex a b foo | tee /tmp/test.out
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_modify -u /tmp/test.ffindex b
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /tmp/test.data /tmp/test.ffindex a b foo | tee /tmp/test-unlink.out
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_build -a -s /tmp/test.data /tmp/test.ffindex ../test/testfile
	@echo ======== compare results =========
	diff /tmp/test.out test.should
	diff /tmp/test-unlink.out test-unlink.should
	@echo ======== error handling ==========
	@echo
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /tmp/test.data /tmp/test.ffindex nonexistquery b foo || echo
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /nonexist/data /tmp/test.ffindex a foo || echo
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /tmp/test.data /nonexist/index a foo || echo

time:
	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" zsh -c "time ./ffindex_get  ../test_big/nr20_11Jan10_a3m_db ../test_big/nr20_11Jan10_a3m_db.ffindex BABBAMIBA.a3m KARDUZEBA.a3m HARPAQIBA.a3m WOQQATEBA.a3m BEBRAQIBA.a3m GIRSAKEBA.a3m NAQTIFIBA.a3m BIWCIQABA.a3m > /dev/null"

clean:
	rm -f ffindex.o ffindex_build ffindex_build.o ffindex_get ffindex_get.o libffindex.so libffindex.so.0.1 ffindex_modify ffindex_modify.o

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include
	install ffindex_get $(INSTALL_DIR)/bin/ffindex_get
	install ffindex_build $(INSTALL_DIR)/bin/ffindex_build
	install ffindex_modify $(INSTALL_DIR)/bin/ffindex_modify
	install libffindex.so.0.1 $(INSTALL_DIR)/lib/libffindex.so.0.1
	ln -sf libffindex.so.0.1 $(INSTALL_DIR)/lib/libffindex.so
	install ffindex.h $(INSTALL_DIR)/include/ffindex.h
	install fferror.h $(INSTALL_DIR)/include/fferror.h
