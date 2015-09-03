#include <ffindex.h>

ffindex_index_t* ffindex_index_as_tree(ffindex_index_t* index);

ffindex_index_t* ffindex_tree_unlink(ffindex_index_t* index, char* name_to_unlink);

ffindex_index_t* ffindex_unlink_entries(ffindex_index_t* index, char** sorted_names_to_unlink, int n_names);

int ffindex_tree_write(ffindex_index_t* index, FILE* index_file);
