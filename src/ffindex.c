/*
 * Written by Andy Hauser.
 */

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1


#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#define N 4096

int ffindex_build(FILE *data_file, FILE *index_file, char *input_dir_name)
{
  DIR *dir = opendir(input_dir_name);
  if(dir == NULL)
    return -1;
  size_t input_dir_name_len = strlen(input_dir_name);
  char path[input_dir_name_len + NAME_MAX + 2];
  strncpy(path, input_dir_name, NAME_MAX);
  if(input_dir_name[input_dir_name_len - 1] != '/')
  {
    path[input_dir_name_len] = '/';
    input_dir_name_len += 1;
  }
  size_t offset = 0;
  struct dirent *entry;
  char buffer[N];
  while((entry = readdir(dir)) != NULL)
  {
    if(entry->d_name[0] == '.')
      continue;
    strncpy(path + input_dir_name_len, entry->d_name, NAME_MAX);
    struct stat sb;
    if(stat(path, &sb) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }
    if(!S_ISREG(sb.st_mode))
      continue;
    FILE *file = fopen(path, "r");
    if(file == NULL)
      perror(path);

    /* Paste file to data file */
    size_t offset_start = offset;
    size_t read_size;
    while((read_size = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0)
    {
      size_t write_size = fwrite(buffer, sizeof(char), read_size, data_file);
      offset += write_size;
      if(read_size != write_size) /* XXX handle better */
        perror(path);
    }

    /* Seperate by '\0' and make sure at least one byte is written */
    buffer[0] = 0;
    size_t write_size = fwrite(buffer, sizeof(char), 1, data_file);
    offset += 1;

    fprintf(index_file, "%s\t%ld\t%ld\n", entry->d_name, offset_start, offset - offset_start);

    if(ferror(file) != 0 || ferror(data_file) != 0)
    {
      perror(path);
      exit(1);
    }
    fclose(file);
  }
  closedir(dir);
}

int ffindex_restore(FILE *data_file, FILE *index_file, char *input_dir_name)
{
}

void* ffindex_mmap_data(FILE *data_file)
{
  struct stat sb;
  fstat(fileno(data_file), &sb);
  off_t size = sb.st_size;
  int fd =  fileno(data_file);
  if(fd < 0)
    return NULL;
  return mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
}

/* Starts to look for entry_name in index_file from the current position */
int ffindex_get_next_entry_by_name(FILE *index_file, char *entry_name, size_t *offset, size_t *length)
{
  char name[NAME_MAX];
  int n;
  while((n = fscanf(index_file, "%s\t%ld\t%ld\n", name, offset, length)) > 0)
  {
    if(n != 3)
    {
      fprintf(stderr, "broken index file: wrong numbers of elements in line");
      exit(1);
    }
    if(strncmp(entry_name, name, NAME_MAX) == 0)
      return 0;
  }
  return -1; /* Not found */
}

int ffindex_get_entry(FILE *index_file, char *filename, size_t *offset, size_t *length)
{
  int found = ffindex_get_next_entry_by_name(index_file, filename, offset, length);
  rewind(index_file);
  return found;
}

char* ffindex_get_filedata(void* data, size_t offset)
{
  return data + offset;
}

FILE* ffindex_fopen(void *data, FILE *index_file, char *filename)
{
  size_t offset, length;
  if(ffindex_get_entry(index_file, filename, &offset, &length) == 0)
  {
    char *filedata = ffindex_get_filedata(data, offset);
    return fmemopen(filedata, length, "r");
  }
  else
  {
    fprintf(stderr, "ERROR in ffindex_get_entry");
    exit(1);
  }
}


/* vim: ts=2 sw=2 et
*/
