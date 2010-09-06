/*
 * Written by Andy Hauser.
 */

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <stdio.h>
#include <limits.h>

#include "ffindex.h"


int main(int argn, char **argv)
{
  if(argn < 3)
  {
    fprintf(stderr, "USAGE: %s data_filename index_filename filename\n", argv[0]);
    return -1;
  }
  char *data_filename  = argv[1];
  char *index_filename = argv[2];
  char *filename = argv[3];

  FILE *data_file  = fopen(data_filename, "r");
  FILE *index_file = fopen(index_filename, "r");

  if( data_file == NULL) { perror(data_filename); return 1; }
  if(index_file == NULL) { perror(index_filename); return 1; }

  void *data = ffindex_mmap_data(data_file);
  FILE *file = ffindex_fopen(data, index_file, filename);
  char line[LINE_MAX];
  while(fgets(line, LINE_MAX, file) != NULL)
  {
    puts(line);
  }

  return 0;
}

/* vim: ts=2 sw=2 et
 */