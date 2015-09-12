/*
 * Ffindex
 * written by Andreas Hauser <andy@splashground.de>.
 * Please add your name here if you distribute modified versions.
 * 
 * Ffindex is provided under the Create Commons license "Attribution-ShareAlike
 * 3.0", which basically captures the spirit of the Gnu Public License (GPL).
 * 
 * See:
 * http://creativecommons.org/licenses/by-sa/3.0/
*/

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ext/fmemopen.h" /* For OS not yet implementing this new standard function */
#include "ffutil.h"
#include "ffindex.h"

/* XXX Use page size? */
#define FFINDEX_BUFFER_SIZE 4096

char* ffindex_copyright_text = FFINDEX_COPYRIGHT;

char* ffindex_copyright()
{
  return ffindex_copyright_text;
}


ffindex_db_t * ffindex_index_db_open(ffindex_db_t * ffindex_db)
{
  int ret = ffindex_index_open(ffindex_db->ffdata_filename, ffindex_db->ffindex_filename, ffindex_db->mode, &ffindex_db->ffdata_file, &ffindex_db->ffindex_file, &ffindex_db->offset);
  if(ret)
    return NULL;

  ffindex_db->ffindex = ffindex_index_parse(ffindex_db->ffindex_file, ffindex_db->num_max_entries);
  ffindex_db->ffdata = ffindex_mmap_data(ffindex_db->ffdata_file, &ffindex_db->ffdata_size);

  return ffindex_db;
}


/* return *out_data_file, *out_index_file, out_offset.
 Setting to a given offset could be supported with a special mode.
 */
int ffindex_index_open(char *data_filename, char *index_filename, char* mode, FILE **out_data_file, FILE **out_index_file, size_t *out_offset)
{
  /* open index and data file, seek to end if needed */
  if(mode[0] == 'a')
  {
    *out_data_file  = fopen(data_filename, "a");
    if(*out_data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    *out_index_file = fopen(index_filename, "a+");
    if(*out_index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }

    struct stat sb;
    fstat(fileno(*out_index_file), &sb);
    fseek(*out_index_file, sb.st_size, SEEK_SET);

    fstat(fileno(*out_data_file), &sb);
    fseek(*out_data_file, sb.st_size, SEEK_SET);
    *out_offset = sb.st_size;
  }
  else if(mode[0] == 'r')
  {
    *out_data_file  = fopen(data_filename, "r");
    if(*out_data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    *out_index_file = fopen(index_filename, "r");
    if(*out_index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }

    *out_offset = 0;
  }
  else
  {
    struct stat st;

    if(stat(data_filename, &st) == 0) { errno = EEXIST; perror(data_filename); return EXIT_FAILURE; }
    *out_data_file  = fopen(data_filename, "w");
    if(*out_data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    if(stat(index_filename, &st) == 0) { errno = EEXIST; perror(index_filename); return EXIT_FAILURE; }
    *out_index_file = fopen(index_filename, "w+");
    if(*out_index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }

    *out_offset = 0;
  }
  return EXIT_SUCCESS;
}

/* Insert a memory chunk (string even without \0) into ffindex */
int ffindex_insert_memory(FILE *data_file, FILE *index_file, size_t *offset, char *from_start, size_t from_length, char *name)
{
    int myerrno = 0;
    size_t write_size;
    size_t offset_before = *offset;

    /* Write data */
    write_size = fwrite(from_start, sizeof(char), from_length, data_file);
    *offset += write_size;
    if(from_length != write_size)
    {
      myerrno = errno;
      fferror_print(__FILE__, __LINE__, __func__, name);
      goto EXCEPTION_ffindex_insert_memory;
    }

    /* Seperate by '\0' and thus also make sure at least one byte is written */
    char buffer[1] = {'\0'};
    if(fwrite(buffer, sizeof(char), 1, data_file) != 1)
    {
      myerrno = errno;
      fferror_print(__FILE__, __LINE__, __func__, name);
      goto EXCEPTION_ffindex_insert_memory;
    }

    *offset += 1;

    /* Write index entry */
    fprintf(index_file, "%s\t%zd\t%zd\n", name, offset_before, *offset - offset_before);

EXCEPTION_ffindex_insert_memory:
    return myerrno;
}


/* Insert all files from a list into ffindex */
int ffindex_insert_list_file(FILE *data_file, FILE *index_file, size_t *start_offset, FILE *list_file)
{
  size_t offset = *start_offset;
  char path[PATH_MAX];
  while(fgets(path, PATH_MAX, list_file) != NULL)
    ffindex_insert_file(data_file, index_file, &offset, ffnchomp(path, strnlen(path, PATH_MAX)), basename(path));

  /* update return value */
  *start_offset = offset;
  return 0;
}


/* Insert all files from directory into ffindex */
int ffindex_insert_dir(FILE *data_file, FILE *index_file, size_t *start_offset, char *input_dir_name)
{
  DIR *dir = opendir(input_dir_name);
  if(dir == NULL)
  {
    fferror_print(__FILE__, __LINE__, __func__, input_dir_name);
    return -1;
  }

  size_t input_dir_name_len = strnlen(input_dir_name, FILENAME_MAX);
  char path[PATH_MAX];
  strncpy(path, input_dir_name, NAME_MAX);
  if(input_dir_name[input_dir_name_len - 1] != '/')
  {
    path[input_dir_name_len] = '/';
    input_dir_name_len += 1;
  }

  size_t offset = *start_offset;
  struct dirent *entry;
  while((entry = readdir(dir)) != NULL)
  {
    if(entry->d_name[0] == '.')
      continue;
    strncpy(path + input_dir_name_len, entry->d_name, NAME_MAX);
    struct stat sb;
    if(stat(path, &sb) == -1)
      fferror_print(__FILE__, __LINE__, __func__, path);
    if(!S_ISREG(sb.st_mode))
      continue;
    ffindex_insert_file(data_file, index_file, &offset, path, entry->d_name);
  }
  closedir(dir);

  /* update return value */
  *start_offset = offset;

  return 0;
}


/* Insert one file by path into ffindex */
int ffindex_insert_file(FILE *data_file, FILE *index_file, size_t *offset, const char *path, char *name)
{
    FILE *file = fopen(path, "r");
    if(file == NULL)
      return errno;

    int ret = ffindex_insert_filestream(data_file, index_file, offset, file, name);
    fclose(file);
    return ret;
}

/* Insert one file by handle into ffindex */
int ffindex_insert_filestream(FILE *data_file, FILE *index_file, size_t *offset, FILE* file, char *name)
{
    int myerrno = 0;
    /* copy and paste file to data file */
    char buffer[FFINDEX_BUFFER_SIZE];
    size_t offset_before = *offset;
    size_t read_size;
    while((read_size = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0)
    {
      size_t write_size = fwrite(buffer, sizeof(char), read_size, data_file);
      *offset += write_size;
      if(read_size != write_size)
        fferror_print(__FILE__, __LINE__, __func__, name);
    }
    if(ferror(file))
      warn("fread");

    /* Seperate by '\0' and thus also make sure at least one byte is written */
    buffer[0] = '\0';
    if(fwrite(buffer, sizeof(char), 1, data_file) != 1)
      perror("ffindex_insert_filestream");
    *offset += 1;
    if(ferror(data_file) != 0)
      goto EXCEPTION_ffindex_insert_file;

    /* write index entry */
    fprintf(index_file, "%s\t%zd\t%zd\n", name, offset_before, *offset - offset_before);

    if(ferror(file) != 0)
      goto EXCEPTION_ffindex_insert_file;

    return myerrno;

EXCEPTION_ffindex_insert_file:
    {
      fferror_print(__FILE__, __LINE__, __func__, "");
      return myerrno;
    }
}


int ffindex_insert_ffindex(FILE* data_file, FILE* index_file, size_t* offset, char* data_to_add, ffindex_index_t* index_to_add)
{
  int err = EXIT_SUCCESS;
  for(size_t entry_i = 0; entry_i < index_to_add->n_entries; entry_i++)
  {
    ffindex_entry_t *entry = ffindex_get_entry_by_index(index_to_add, entry_i);
    if(entry == NULL) { fferror_print(__FILE__, __LINE__, __func__, ""); return EXIT_FAILURE; }
    err = ffindex_insert_memory(data_file, index_file, offset, ffindex_get_data_by_entry(data_to_add, entry), entry->length - 1, entry->name); // skip \0 suffix
    if(err != EXIT_SUCCESS) { fferror_print(__FILE__, __LINE__, __func__, ""); return EXIT_FAILURE;}
  }
  return EXIT_SUCCESS;
}

/* XXX not implemented yet, the functionality is provided by ffindex_unpack.c though */
int ffindex_restore(FILE *data_file, FILE *index_file, char *output_dir_name)
{
  return -1;
}


char* ffindex_mmap_data(FILE *file, size_t* size)
{
  struct stat sb;
  fstat(fileno(file), &sb);
  *size = sb.st_size;
  int fd =  fileno(file);
  if(fd < 0)
  {
    fferror_print(__FILE__, __LINE__, __func__, "mmap failed");
    return MAP_FAILED;
  }
  return (char*)mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
}


int ffindex_compare_entries_by_name(const void *pentry1, const void *pentry2)
{   
  ffindex_entry_t* entry1 = (ffindex_entry_t*)pentry1;
  ffindex_entry_t* entry2 = (ffindex_entry_t*)pentry2;
  return strncmp(entry1->name, entry2->name, FFINDEX_MAX_ENTRY_NAME_LENTH);
}


ffindex_entry_t* ffindex_grep_entry_by_str(ffindex_index_t *ffindex, char *name, size_t* offset)
{
  for(size_t i = *offset; i < ffindex->n_entries; i++)
    if(strstr(ffindex->entries[i].name, name))
    {
      *offset = i;
      return &ffindex->entries[i];
    }
  return NULL;
}


ffindex_entry_t* ffindex_get_entry_by_name(ffindex_index_t *index, char *name)
{
  return ffindex_bsearch_get_entry(index, name);
}

ffindex_entry_t* ffindex_bsearch_get_entry(ffindex_index_t *index, char *name)
{
  ffindex_entry_t search;
  strncpy(search.name, name, FFINDEX_MAX_ENTRY_NAME_LENTH);
  return (ffindex_entry_t*)bsearch(&search, index->entries, index->n_entries, sizeof(ffindex_entry_t), ffindex_compare_entries_by_name);
}


ffindex_index_t* ffindex_index_parse(FILE *index_file, size_t num_max_entries)
{
  if(num_max_entries == 0)
    num_max_entries = FFINDEX_MAX_INDEX_ENTRIES_DEFAULT;
  size_t nbytes = sizeof(ffindex_index_t) + (sizeof(ffindex_entry_t) * num_max_entries);
  ffindex_index_t *index = (ffindex_index_t *)malloc(nbytes);
  if(index == NULL)
  {
    fprintf(stderr, "Failed to allocate %ld bytes\n", nbytes);
    fferror_print(__FILE__, __LINE__, __func__, "malloc failed");
    return NULL;
  }
  index->num_max_entries = num_max_entries;

  index->file = index_file;
  index->index_data = ffindex_mmap_data(index_file, &(index->index_data_size));
  if(index->index_data_size == 0)
    warn("Problem mapping index file. Is it empty or is another process reading it?");
  if(index->index_data == MAP_FAILED)
  {
    free(index);
    return NULL;
  }
  index->type = SORTED_ARRAY; /* XXX Assume a sorted file for now */

  posix_madvise(index->index_data, index->index_data_size, POSIX_MADV_SEQUENTIAL);

  /* Faster than scanf per line */
  size_t names_too_long = 0;
  const char* d = index->index_data;
  size_t i;
  for(i = 0; d < (index->index_data + index->index_data_size); i++)
  {
    const char* end;
    int p;
    for(p = 0; *d != '\t'; d++)
      if(p < FFINDEX_MAX_ENTRY_NAME_LENTH)
        index->entries[i].name[p++] = *d;
      else
        names_too_long++;
    index->entries[i].name[p] = '\0';
    index->entries[i].offset = ffparse_ulong(++d, &end);
    d = end;
    index->entries[i].length = ffparse_ulong(++d, &end);
    d = end + 1; /* +1 for newline */
  }

  index->n_entries = i;

  if(index->n_entries == 0)
    warnx("index with 0 entries");

  if(names_too_long)
  {
    errno = ENAMETOOLONG;
    warn("Cut-off is %d characters. Are they still unique?"
         " Warning encountered %zd times", FFINDEX_MAX_ENTRY_NAME_LENTH, names_too_long);
    errno = 0;
  }

  return index;
}

ffindex_entry_t* ffindex_get_entry_by_index(ffindex_index_t *index, size_t entry_index)
{
  if(entry_index < index->n_entries)
    return &index->entries[entry_index];
  else
    return NULL;
}

/* Using a function for this looks like overhead. But a more advanced data format,
 * say a compressed one, can do it's magic here. 
 */
char* ffindex_get_data_by_offset(char* data, size_t offset)
{
  return data + offset;
}


char* ffindex_get_data_by_entry(char *data, ffindex_entry_t* entry)
{
  return ffindex_get_data_by_offset(data, entry->offset);
}


char* ffindex_get_data_by_name(char *data, ffindex_index_t *index, char *name)
{
  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, name);

  if(entry == NULL)
    return NULL;

  return ffindex_get_data_by_entry(data, entry);
}


char* ffindex_get_data_by_index(char *data, ffindex_index_t *index, size_t entry_index)
{
  ffindex_entry_t* entry = ffindex_get_entry_by_index(index, entry_index);

  if(entry == NULL)
    return NULL;

  return ffindex_get_data_by_entry(data, entry);
}


FILE* ffindex_fopen_by_entry(char *data, ffindex_entry_t* entry)
{
  char *filedata = ffindex_get_data_by_offset(data, entry->offset);
  return fmemopen(filedata, entry->length, "r");
}


FILE* ffindex_fopen_by_name(char *data, ffindex_index_t *index, char *filename)
{
  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, filename);

  if(entry == NULL)
    return NULL;

  return ffindex_fopen_by_entry(data, entry);
}


void ffindex_sort_index_file(ffindex_index_t *index)
{
  qsort(index->entries, index->n_entries, sizeof(ffindex_entry_t), ffindex_compare_entries_by_name);
}


/* Faster than fprintf but no error checking.
 * If more robustness is neeeded, prealocate the output file.
 */
size_t ffindex_print_entry(FILE* file, ffindex_entry_t* entry)
{
  size_t written;
  written = fwrite(entry->name, 1, strnlen(entry->name, FFINDEX_MAX_ENTRY_NAME_LENTH), file);
  fputc('\t', file);

  written += fffprint_ulong(file, entry->offset);
  fputc('\t', file);

  written += fffprint_ulong(file, entry->length);

  fputc('\n', file);

  return written + 3; // * fputc
}


int ffindex_write(ffindex_index_t* index, FILE* index_file)
{
  int ret = EXIT_SUCCESS;
#if _POSIX_C_SOURCE >= 200112L
  ret = posix_fallocate(fileno(index_file), 0, index->n_entries * 30); // guesstimate
  if(ret)
    return ret;
#endif

  /* Use tree if available */
  if(index->type == TREE)
    ret = ffindex_tree_write(index, index_file);
  else
    for(size_t i = 0; i < index->n_entries; i++)
      if(ffindex_print_entry(index_file,index->entries + i) < 6)
        return EXIT_FAILURE;

#if _POSIX_C_SOURCE >= 200112L
  ret = posix_fallocate(fileno(index_file), 0, index->n_entries * 30); // guesstimate
  ftruncate(fileno(index_file), ftell(index_file));
#endif

  return ret;
}


ffindex_index_t* ffindex_unlink(ffindex_index_t* index, char* name_to_unlink)
{
  /* Use tree if available */
  if(index->type == TREE)
    return ffindex_tree_unlink(index, name_to_unlink);

  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, name_to_unlink);
  if(entry == NULL)
  {
    fprintf(stderr, "Warning: could not find '%s'\n", name_to_unlink);
    return index;
  }
  /* Move entries after the unlinked one to close the gap */
  size_t n_entries_to_move = index->entries + index->n_entries - entry - 1;
  if(n_entries_to_move > 0) /* not last element of array */
    memmove(entry, entry + 1, n_entries_to_move * sizeof(ffindex_entry_t));
  index->n_entries--;
  return index;
}


/* vim: ts=2 sw=2 et
*/
