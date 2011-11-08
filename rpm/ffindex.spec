Summary: FFindex is a very simple index/database for huge amounts of small files. 
Name: ffindex
Version: 0.9.2
Release: 1
License: Create Commons license "Attribution-ShareAlike 3.0"
Group: Utilities/System
Source: http://housedata.genzentrum.lmu.de/public/software/linux/ffindex/ffindex-0.9.2.tar.gz
%description
FFindex is a very simple index/database for huge amounts of small files. The
files are stored concatenated in one big data file, seperated by '\0'. A second
file contains a plain text index, giving name, offset and length of of the
small files. The lookup is currently done with a binary search on an array made
from the index file.

%prep
%setup

%build
make

%install
make install INSTALL_DIR=/usr

%files
%doc README LICENSE

/usr/bin/ffindex_build
/usr/bin/ffindex_get
/usr/bin/ffindex_modify
/usr/lib/libffindex.a
/usr/lib/libffindex.so
/usr/lib/libffindex.so.0.1
