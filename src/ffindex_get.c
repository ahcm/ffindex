/*
 * Ffindex
 * written by Andy Hauser <hauser@genzentrum.lmu.de>.
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffindex.h"
#include "ffutil.h"


int main(int argn, char **argv)
{
  if(argn < 3)
  {
    fprintf(stderr, "USAGE: %s data_filename index_filename filename(s)\n"
                    "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n",
                    argv[0]);
    return -1;
  }
  char *data_filename  = argv[1];
  char *index_filename = argv[2];

  FILE *data_file  = fopen(data_filename,  "r");
  FILE *index_file = fopen(index_filename, "r");

  if( data_file == NULL) { fferror_print(__FILE__, __LINE__, "ffindex_get", data_filename);  exit(EXIT_FAILURE); }
  if(index_file == NULL) { fferror_print(__FILE__, __LINE__, "ffindex_get", index_filename);  exit(EXIT_FAILURE); }

  size_t data_size;
  char *data = ffindex_mmap_data(data_file, &data_size);

  ffindex_index_t* index = ffindex_index_parse(index_file, 0);
  if(index == NULL)
  {
    fferror_print(__FILE__, __LINE__, "ffindex_index_parse", index_filename);
    exit(EXIT_FAILURE);
  }

  for(int i = 3; i < argn; i++)
  {
    char *filename = argv[i];
    FILE *file = ffindex_fopen(data, index, filename);
    if(file == NULL)
    {
      errno = ENOENT; 
      fferror_print(__FILE__, __LINE__, "ffindex_fopen file not found in index", filename);
    }
    else
    {
      char line[LINE_MAX];
      while(fgets(line, LINE_MAX, file) != NULL)
        printf("%s", line); /* XXX Mask nonprintable characters */
    }
  }

  return 0;
}

/* vim: ts=2 sw=2 et
 */
