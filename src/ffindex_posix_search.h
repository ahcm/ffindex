#include <ffindex.h>

ffindex_index_t* ffindex_index_as_tree(ffindex_index_t* index);

ffindex_index_t* ffindex_tree_unlink(ffindex_index_t* index, char* name_to_unlink);

int ffindex_tree_write(ffindex_index_t* index, FILE* index_file);
