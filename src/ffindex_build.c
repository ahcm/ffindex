/*
 * Written by Andy Hauser.
 */

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <stdio.h>

#include "ffindex.h"


int main(int argn, char **argv)
{
  if(argn < 3)
  {
    fprintf(stderr, "USAGE: %s data_filename index_filename dir_to_index\n", argv[0]);
    return -1;
  }
  char *data_filename  = argv[1];
  char *index_filename = argv[2];
  FILE *data_file  = fopen(data_filename, "w+");
  FILE *index_file = fopen(index_filename, "w+");
  if( data_file == NULL) { perror(data_filename); return 1; }
  if(index_file == NULL) { perror(index_filename); return 1; }

  size_t offset = 0;
  for(int i = 3; i < argn; i++)
    if(ffindex_build(data_file, index_file, &offset, argv[i]) < 0)
      perror(argv[i]);

  return 0;
}

/* vim: ts=2 sw=2 et
 */
