# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

LIB_DIRS = ["../src"]

# Give it a name
extension_name = 'ffindex'

# The destination
dir_config(extension_name, "../src", "../src")

find_library('ffindex', 'ffindex_index_db_open')

# Do the work
create_makefile(extension_name)

