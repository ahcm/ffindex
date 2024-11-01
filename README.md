# Copyright

FFindex was written by Andreas Hauser <Andreas.Hauser@LMU.de>.
Please add your name here if you distribute modified versions.

FFindex is a registered trademark of the Ludwig-Maximilians-Universität, Munich (LMU).

FFindex is provided under the Create Commons license "Attribution-ShareAlike 4.0",
which basically captures the spirit of the Gnu Public License (GPL).

See:
http://creativecommons.org/licenses/by-sa/4.0/

In addition, modified distributions need to add additional authors in files they distribute.
Also mainline code must be refered to inlcuding a link:

https://github.com/ahcm/ffindex

Should the mainline code URL change, these changes must be reflected, so that the link is always pointing to current mainline.

# Notice

This is mainline FFindex:
https://github.com/ahcm/ffindex

A prominent fork is https://github.com/soedinglab/ffindex_soedinglab

It introduces many bugs, limitations and security flaws.
In violation of what the license asks, the authors do not add their names to the modified files.

There is an open github issue: https://github.com/soedinglab/ffindex_soedinglab/issues/9

From looking at the code I can only advise not to use it.
Certainly not in a public service.

If you have a problem with the fork, it might be worthwhile to just try mainline.

# Thanks

Thanks to Laszlo Kajan for creating and maintaining Debian packages
and many suggestions to improve the build and user experience.


# Overview

FFindex is a very simple index/database for huge amounts of small files. The
files are stored concatenated in one big data file, seperated by '\0'. A second
file contains a plain text index, giving name, offset and length of of the
small files. The lookup is currently done with a binary search on an array made
from the index file.


# Installation

```
$ cd src
$ make
$ make test
```
If you have MPI and want to compile ffindex_apply_mpi:
```
$ make HAVE_MPI=1
```
On OS X use for the first make line:
```
$ make -f Makefile.osx
```
Please use a sensible value for INSTALL_DIR, e.g. /usr/local or /opt/ffindex
or $HOME/ffindex instead of "..".
```
$ make install INSTALL_DIR=.. 
```
and with MPI:
```
$ make install INSTALL_DIR=.. HAVE_MPI=1
```

# Usage

Please note that before querying or unlinking entries a ffindex must be
sorted, although you can add to it without. So either specify -s with
ffindex_build or sorted later with ffindex_modify -s.
Also the length of the entry names is restricted. See the usage output of
the ffindex_build program.

Setup environment:
```
$ export PATH="$INSTALL_DIR/bin:$PATH"
$ export LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
```
On OS X set DYLD_LIBRARY_PATH instead of LD_LIBRARY_PATH.

Build index from files in test/data and test/data2.
```
$ ffindex_build -s /tmp/test.data /tmp/test.ffindex test/data test/data2
```
Retrieve three entries:
```
$ ffindex_get  /tmp/test.data /tmp/test.ffindex a b foo
```
Unlink (Remove reference from index) an entry:
```
$ ffindex_modify -u /tmp/test.ffindex b
```
Retrieve three entries, "b" should now be missing:
```
$ ffindex_get /tmp/test.data /tmp/test.ffindex a b foo
```
Convert a Fasta file to ffindex, entry names are incerental IDs starting from 1:
```
$ ffindex_from_fasta -s fasta.ffdata fasta.ffindex NC_007779.ffn
```
Get first entry by name:
```
$ ffindex_get fasta.ffdata fasta.ffindex 1
```
Get first and third entry by entry index, this a little faster:
```
$ ffindex_get fasta.ffdata fasta.ffindex -n 1 3
```
Count the characters including header in each entry:
```
$ ffindex_apply fasta.ffdata fasta.ffindex wc -c
```
Count the number of characters in each sequence, without the header:
```
$ ffindex_apply fasta.ffdata fasta.ffindex perl -ne '$x += length unless(/^>/); END{print "$x\n"}'
```
Parallel version for counting the characters including header in each entry:
```
$ mpirun -np 4 ffindex_apply_mpi fasta.ffdata fasta.ffindex -- wc -c
```
Parallel version for counting the characters including header in each entry and
saving the output to a new ffindex:
```
$ mpirun -np 4 ffindex_apply_mpi fasta.ffdata fasta.ffindex -i out-wc.ffindex -d out-wc.ffdata -- wc -c
```
