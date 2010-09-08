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
    fprintf(stderr, "USAGE: %s data_filename index_filename filename(s)\n", argv[0]);
    return -1;
  }
  char *data_filename  = argv[1];
  char *index_filename = argv[2];

  FILE *data_file  = fopen(data_filename, "r");
  FILE *index_file = fopen(index_filename, "r");

  if( data_file == NULL) { perror(data_filename); return 1; }
  if(index_file == NULL) { perror(index_filename); return 1; }

  char *data = ffindex_mmap_data(data_file);

  for(int i = 3; i < argn; i++)
  {
    char *filename = argv[i];
    FILE *file = ffindex_fopen(data, index_file, filename);
    char line[LINE_MAX];
    /* XXX Mask nonprintable characters */
    while(fgets(line, LINE_MAX, file) != NULL)
      printf("%s", line);
  }

  return 0;
}

/* vim: ts=2 sw=2 et
 */
