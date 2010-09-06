
int ffindex_build(FILE *data_file, FILE *index_file, char *input_dir_name);

FILE* ffindex_fopen(void *data, FILE *index_file, char *filename);

void* ffindex_mmap_data(FILE *data_file);
