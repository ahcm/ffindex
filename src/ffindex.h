
int ffindex_build(FILE *data_file, FILE *index_file, size_t *offset, char *input_dir_name);

FILE* ffindex_fopen(char *data, FILE *index_file, char *filename);

char* ffindex_mmap_data(FILE *data_file);

int ffindex_get_next_entry_by_name(FILE *index_file, char *entry_name, size_t *offset, size_t *length);

int ffindex_get_entry(FILE *index_file, char *filename, size_t *offset, size_t *length);

char* ffindex_get_filedata(char* data, size_t offset);
