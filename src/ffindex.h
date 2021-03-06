/*
 * Ffindex
 * written by Andreas Hauser <andy@splashground.de>.
 * Please add your name here if you distribute modified versions.
 * 
 * Ffindex is provided under the Create Commons license "Attribution-ShareAlike
 * 4.0", which basically captures the spirit of the Gnu Public License (GPL).
 * 
 * See:
 * http://creativecommons.org/licenses/by-sa/4.0/
 * 
 * Ffindex is a very simple database for small files. The files are stored
 * concatenated in one big data file, seperated by '\0'. A second file
 * contains a plain text index, giving name, offset and length of of the small
 * files.
 */

#ifndef _FFINDEX_H

#define _FFINDEX_H 1

#include <stdio.h>

#define FFINDEX_VERSION 1.0
#define FFINDEX_COPYRIGHT "\nDesigned and implemented by Andreas Hauser <andy@splashground.de>.\n"

#define FFINDEX_MAX_INDEX_ENTRIES_DEFAULT 80000000
#define FFINDEX_MAX_ENTRY_NAME_LENTH 63


enum ffindex_type { PLAIN_FILE, SORTED_FILE, SORTED_ARRAY, TREE };


typedef struct ffindex_entry {
  size_t offset;
  size_t length;
  char name[FFINDEX_MAX_ENTRY_NAME_LENTH + 1];
} ffindex_entry_t;


typedef struct ffindex_index {
  enum ffindex_type type;
  char* filename;
  FILE* file;
  char* index_data;
  size_t index_data_size;
  void* tree_root;
  size_t num_max_entries;
  size_t n_entries;
  ffindex_entry_t entries[]; /* This array is as big as the excess memory allocated for this struct. */
} ffindex_index_t;


typedef struct ffindex_db
{
  char * ffdata_filename;
  char * ffindex_filename;
  FILE * ffdata_file;
  FILE * ffindex_file;
  char * ffdata;
  size_t ffdata_size;
  ffindex_index_t * ffindex;
  char mode[3];
  size_t offset;
  size_t num_max_entries;
} ffindex_db_t;


ffindex_db_t * ffindex_index_db_open(ffindex_db_t * ffindex_db);

int ffindex_index_db_close(ffindex_db_t* ffindex_db);


int ffindex_index_close(ffindex_index_t* ffindex);

/* return *out_data_file, *out_index_file, out_offset. */
int ffindex_index_open(char *data_filename, char *index_filename, char* mode, FILE **out_data_file, FILE **out_index_file, size_t *out_offset);

int ffindex_insert_memory(FILE *data_file, FILE *index_file, size_t *offset, char *from_start, size_t from_length, char *name);

int ffindex_insert_file(FILE *data_file, FILE *index_file, size_t *offset, const char *path, char *name);

int ffindex_insert_filestream(FILE *data_file, FILE *index_file, size_t *offset, FILE* file, char *name);

int ffindex_insert_list_file(FILE *data_file, FILE *index_file, size_t *start_offset, FILE *list_file);

int ffindex_insert_dir(FILE *data_file, FILE *index_file, size_t *offset, char *input_dir_name);

int ffindex_insert_ffindex(FILE* data_file, FILE* index_file, size_t* offset, char* data_to_add, ffindex_index_t* index_to_add);


FILE* ffindex_fopen_by_entry(char *data, ffindex_entry_t* entry);

FILE* ffindex_fopen_by_name(char *data, ffindex_index_t *index, char *name);

char* ffindex_mmap_data(FILE *file, size_t* size);


int ffindex_compare_entries_by_name(const void *pentry1, const void *pentry2);


char* ffindex_get_data_by_offset(char* data, size_t offset);

char* ffindex_get_data_by_entry(char *data, ffindex_entry_t* entry);

char* ffindex_get_data_by_name(char *data, ffindex_index_t *index, char *name);

char* ffindex_get_data_by_index(char *data, ffindex_index_t *index, size_t entry_index);


ffindex_entry_t* ffindex_grep_entry_by_str(ffindex_index_t *ffindex, char *name, size_t* offset);


ffindex_entry_t* ffindex_get_entry_by_index(ffindex_index_t *index, size_t entry_index);

ffindex_entry_t* ffindex_get_entry_by_name(ffindex_index_t *index, char *name);

ffindex_index_t* ffindex_index_parse(FILE *index_file, size_t num_max_entries);

ffindex_entry_t* ffindex_bsearch_get_entry(ffindex_index_t *index, char *name);


void ffindex_sort_index_file(ffindex_index_t *index);

size_t ffindex_print_entry(FILE* file, ffindex_entry_t* entry);

int ffindex_write(ffindex_index_t* index, FILE* index_file);

ffindex_index_t* ffindex_unlink(ffindex_index_t* index, char *entry_name);

int ffsort_index(const char* index_filename);


char* ffindex_copyright();

#include <ffindex_posix_search.h>

#endif
