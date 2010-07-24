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
  while((entry = readdir(dir)) != NULL)
  {
    if(entry->d_name[0] == '.')
      continue;
    strncpy(path + input_dir_name_len, entry->d_name, NAME_MAX);
    puts(path);
    FILE *file = fopen(path, "r");
    if(file == NULL)
      perror(path);
    fprintf(index_file, "%s\t%ld\n", entry->d_name, offset);
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
  }
}


/* vim: ts=2 sw=2 et
 */
