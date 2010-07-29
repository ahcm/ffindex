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

#define N 1024

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
  size_t buffer_size = N * sizeof(double);
  double *buffer = calloc(N, sizeof(double));
  while((entry = readdir(dir)) != NULL)
  {
    if(entry->d_name[0] == '.')
      continue;
    strncpy(path + input_dir_name_len, entry->d_name, NAME_MAX);
    struct stat sb;
    if (stat(path, &sb) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }
    if(!S_ISREG(sb.st_mode))
      continue;
    // puts(path);
    FILE *file = fopen(path, "r");
    if(file == NULL)
      perror(path);

    fprintf(index_file, "%s\t%ld\n", entry->d_name, offset);
    size_t read_size;
    while((read_size = fread(buffer, N, sizeof(double), file)) > 0)
      offset += fwrite(buffer, N, sizeof(double), data_file);

    if(ferror(file) != 0)
    {
      perror(path);
      exit(1);
    }
    fclose(file);
  }
  closedir(dir);
  free(buffer);
}

int ffindex_restore(FILE *data_file, FILE *index_file, char *input_dir_name)
{
}

void* ffindex_mmap_data(FILE *data_file)
{
  struct stat sb;
  fstat(data_file, &sb);
  off_t size = sb.st_size;
  int fd =  fileno(data_file);
  if(fd < 0)
    return NULL;
  return mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
}


size_t ffindex_get_offset(FILE *index_file, char *filename)
{
  char name[NAME_MAX];
  size_t offset;
  int n;
  while((n = fscanf(index_file, "%s\t%ld", name, &offset)) > 0)
  {
    if(n != 2)
    {
      fprintf(stderr, "wrong numbers of elements in line");
      exit(1);
    }

    if(strncmp(filename, name, NAME_MAX) == 0)
      return offset;
  }
  return -1; /* Not found */
}

char* ffindex_get_filedata(void* data, offset)

FILE* ffindex_fopen(void *data, FILE *index_file, char *filename)
{

}


/* vim: ts=2 sw=2 et */
