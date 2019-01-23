INSTALL_DIR?=..
libdir=`([ -d /usr/lib64 ] && echo lib64) || echo lib`
LANG=C
#CC=musl-gcc
#OMPI_CC=musl-gcc
#CC=gcc
#CC=icc
#CC=clang
CFLAGS+=-std=c99 -O2 -g -fPIC -Wall -pedantic -D_GNU_SOURCE=1 -I.
LDFLAGS=-lffindex -L.

OS:= $(shell uname)

FFINDEX_LIBS=libffindex.so libffindex.so.0.1 libffindex.a

FFINDEX_HEADERS=ffindex.h ffindex_posix_search.h ffutil.h
FFINDEX_OBJECTS=$(FFINDEX_HEADERS:.h=.o)

FFINDEX_PROGRAMS=ffindex_apply ffindex_from_fasta ffindex_get ffindex_build ffindex_modify ffindex_unpack ffindex_from_tsv

ifdef HAVE_MPI
FFINDEX_PROGRAMS+=ffindex_apply_mpi
endif

all: $(FFINDEX_LIBS) $(FFINDEX_PROGRAMS)

# for Darwin only
ext/fmemopen.o: ext/fmemopen.c
	make -C ext

ffindex.o: ffindex.c $(FFINDEX_HEADERS)

libffindex.a: $(FFINDEX_OBJECTS)
	ar rcs libffindex.a $^

ifeq ($(OS), Darwin)
libffindex.so.0.1: $(FFINDEX_OBJECTS) ext/fmemopen.o
		$(CC) -shared -o libffindex.so.0.1 $^
else

 ifdef FFINDEX_STATIC
STATIC=-static
 endif

libffindex.so.0.1: $(FFINDEX_OBJECTS)
		$(CC) -Wl,-z,defs -Wl,-soname,libffindex.so.0.1 -shared -o libffindex.so.0.1 $^

endif

libffindex.so: libffindex.so.0.1
	ln -sf libffindex.so.0.1 libffindex.so

ffindex_apply.o: ffindex_apply_mpi.c $(FFINDEX_LIBS)
	$(CC) $(CFLAGS) -c ffindex_apply_mpi.c -o $@ 

ffindex_apply: ffindex_apply.o $(FFINDEX_LIBS)
	$(CC) $(CFLAGS) -o $@  ffindex_apply.o $(LDFLAGS) $(STATIC)

ffindex_apply_mpi: ffindex_apply_mpi.o $(FFINDEX_LIBS)
	mpicc $(CFLAGS) -DHAVE_MPI -o $@  ffindex_apply_mpi.o $(LDFLAGS)

ffindex_apply_mpi.o: ffindex_apply_mpi.c $(FFINDEX_HEADERS)
	mpicc $(CFLAGS) -DHAVE_MPI -c ffindex_apply_mpi.c -o $@ 

ffindex_apply_mpi_spawn: ffindex_apply_mpi_spawn.o $(FFINDEX_LIBS)
	mpicc $(CFLAGS) -o $@  ffindex_apply_mpi_spawn.o $(LDFLAGS)

ffindex_apply_mpi_spawn.o: ffindex_apply_mpi_spawn.o $(FFINDEX_HEADERS)
	mpicc $(CFLAGS) -c ffindex_apply_mpi_spawn.c -o $@ 

ffindex_from_fasta: ffindex_from_fasta.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_from_fasta.o $(LDFLAGS) $(STATIC)

ffindex_from_tsv: ffindex_from_tsv.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_from_tsv.o $(LDFLAGS) $(STATIC)

ffindex_get: ffindex_get.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_get.o $(LDFLAGS) $(STATIC)

ffindex_build: ffindex_build.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_build.o $(LDFLAGS) $(STATIC)

ffindex_modify: ffindex_modify.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_modify.o $(LDFLAGS) $(STATIC)

ffindex_unpack: ffindex_unpack.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_unpack.o $(LDFLAGS) $(STATIC)

ffindex_order: ffindex_order.o $(FFINDEX_LIBS)
	$(CC) -o $@ ffindex_unpack.o $(LDFLAGS) $(STATIC)

test:
	rm -f /tmp/test.data /tmp/test.ffindex
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
	rm -f $(FFINDEX_PROGRAMS) $(FFINDEX_OBJECTS) $(FFINDEX_LIBS) \
		libffindex.so libffindex.so.0.1 libffindex.a \
		ffindex_build.o ffindex_get.o ffindex_modify.o ffindex_from_fasta.o ffindex_apply_mpi.o \
		ffindex_apply.o ffindex_from_tsv.o ffindex_unpack.o ffindex_posix_search.o ffindex_apply_mpi_spawn.o

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/$(libdir)
	mkdir -p $(INSTALL_DIR)/include
	install ffindex_get $(INSTALL_DIR)/bin/ffindex_get
	install ffindex_build $(INSTALL_DIR)/bin/ffindex_build
	install ffindex_modify $(INSTALL_DIR)/bin/ffindex_modify
	install ffindex_from_fasta $(INSTALL_DIR)/bin/ffindex_from_fasta
	install ffindex_from_tsv $(INSTALL_DIR)/bin/ffindex_from_tsv
	install ffindex_apply $(INSTALL_DIR)/bin/ffindex_apply
	install ffindex_unpack $(INSTALL_DIR)/bin/ffindex_unpack
	install libffindex.so.0.1 $(INSTALL_DIR)/$(libdir)/libffindex.so.0.1
	install libffindex.a $(INSTALL_DIR)/$(libdir)/libffindex.a
	ln -sf libffindex.so.0.1 $(INSTALL_DIR)/$(libdir)/libffindex.so
	install ffindex.h $(INSTALL_DIR)/include/ffindex.h
	install ffutil.h $(INSTALL_DIR)/include/ffutil.h
	install ffindex_posix_search.h $(INSTALL_DIR)/include/ffindex_posix_search.h
ifdef HAVE_MPI
	install ffindex_apply_mpi $(INSTALL_DIR)/bin/ffindex_apply_mpi
endif

deinstall:
	rm -f $(INSTALL_DIR)/bin/ffindex_get $(INSTALL_DIR)/bin/ffindex_build $(INSTALL_DIR)/bin/ffindex_modify \
		$(INSTALL_DIR)/$(libdir)/libffindex.so.0.1 $(INSTALL_DIR)/$(libdir)/libffindex.a \
		$(INSTALL_DIR)/$(libdir)/libffindex.so $(INSTALL_DIR)/include/ffindex.h $(INSTALL_DIR)/include/ffutil.h\
		$(INSTALL_DIR)/include/ffindex_posix_search.h $(INSTALL_DIR)/bin/ffindex_from_fasta $(INSTALL_DIR)/bin/ffindex_from_tsv
	rmdir $(INSTALL_DIR)/bin || true
	rmdir $(INSTALL_DIR)/$(libdir) || true
	rmdir $(INSTALL_DIR)/include || true
