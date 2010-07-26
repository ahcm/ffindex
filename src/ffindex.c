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
    else
    {
      fprintf(index_file, "%s\t%ld\n", entry->d_name, offset);
      size_t read_size;
      while((read_size = fread(buffer, N, sizeof(double), file)) > 0)
        offset += fwrite(buffer, N, sizeof(double), data_file);
      if(ferror(file) != 0)
      {
        perror(path);
        exit(1);
      }
      /*
      int c;
      while((c = fgetc(file)) != EOF)
      {
        c = putc(c, data_file);
        if(c == EOF)
        {
          perror(path);
          exit(1);
        }
        offset++;
      }
      putc('\0', data_file);
      offset++;
      */
      fclose(file);
    }
  }
}

int fftindex_read()
{
}

/* vim: ts=2 sw=2 et
 */
