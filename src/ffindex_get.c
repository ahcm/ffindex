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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "ffindex.h"
#include "ffutil.h"

void usage(char* program_name)
{
    fprintf(stderr, "USAGE: %s data_filename index_filename entry name(s)\n"
                    "\t-f FILE\t\tfile containing a list of file names, one per line\n"
                    "\t-n\t\tuse index of entry instead of entry name\n"
                    FFINDEX_COPYRIGHT,
                    program_name);
}

int main(int argn, char **argv)
{
  int by_index = 0;
  int opt;
  char* list_filenames[4096];
  size_t list_filenames_index = 0;
  while ((opt = getopt(argn, argv, "f:n")) != -1)
  {
    switch (opt)
    {
      case 'f':
        list_filenames[list_filenames_index++] = optarg;
        break;
      case 'n':
        by_index = 1;
        break;
      default:
        usage(argv[0]);
        return EXIT_FAILURE;
    }
  }
  if(argn < 3)
  {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  char *data_filename  = argv[optind++];
  char *index_filename = argv[optind++];

  ffindex_db_t * ffindex_db = calloc(1, sizeof(ffindex_db_t));
  ffindex_db->ffdata_filename = data_filename;
  ffindex_db->ffindex_filename = index_filename;
  ffindex_db->mode[0] = 'r';

  ffindex_db = ffindex_index_db_open(ffindex_db);
  if(!ffindex_db)
  {
    errno = EINVAL;
    fferror_print(__FILE__, __LINE__, "ffindex_index_db_open failed", argv[0]);
    return EXIT_FAILURE;
  }

  if(by_index)
  {
    for(int i = optind; i < argn; i++)
    {
      size_t index_n = atol(argv[i]) - 1; // offset from 0 but specify from 1

      ffindex_entry_t* entry = ffindex_get_entry_by_index(ffindex_db->ffindex, index_n);
      if(entry == NULL)
      {
        errno = ENOENT; 
        fferror_print(__FILE__, __LINE__, "ffindex_get entry index out of range", argv[i]);
      }
      else
      {
        char *filedata = ffindex_get_data_by_entry(ffindex_db->ffdata, entry);
        if(filedata == NULL)
        {
          errno = ENOENT; 
          fferror_print(__FILE__, __LINE__, "ffindex_get entry index out of range", argv[i]);
        }
        else
          fwrite(filedata, entry->length - 1, 1, stdout);
      }
    }
  }
  else // by name
  {
    // get names given by file
    if(list_filenames_index > 0)
    {
      for(int i = 0; i < list_filenames_index; i++)
      {
        FILE *list_file = fopen(list_filenames[i], "r");
        if( list_file == NULL) { perror(list_filenames[i]); return EXIT_FAILURE; }

        char name[PATH_MAX];
        while(fgets(name, PATH_MAX, list_file) != NULL)
        {
          ffindex_entry_t* entry = ffindex_get_entry_by_name(ffindex_db->ffindex, ffnchomp(name, strnlen(name, PATH_MAX)));
          if(entry == NULL)
          {
            errno = ENOENT; 
            fferror_print(__FILE__, __LINE__, "ffindex_get key not found in index", name);
          }
          else
          {
            char *filedata = ffindex_get_data_by_entry(ffindex_db->ffdata, entry);
            if(filedata == NULL)
            {
              errno = ENOENT; 
              fferror_print(__FILE__, __LINE__, "ffindex_get key not found in index", name);
            }
            else
              fwrite(filedata, entry->length - 1, 1, stdout);
          }
        }
      }
    }

    // get files given on command line
    for(int i = optind; i < argn; i++)
    {
      char *filename = argv[i];

      ffindex_entry_t* entry = ffindex_get_entry_by_name(ffindex_db->ffindex, filename);
      if(entry == NULL)
      {
        errno = ENOENT; 
        fferror_print(__FILE__, __LINE__, "ffindex_get key not found in index", filename);
      }
      else
      {
        char *filedata = ffindex_get_data_by_entry(ffindex_db->ffdata, entry);
        if(filedata == NULL)
        {
          errno = ENOENT; 
          fferror_print(__FILE__, __LINE__, "ffindex_get key not found in index", filename);
        }
        else
          fwrite(filedata, entry->length - 1, 1, stdout);
      }
    }

      /* Alternative code using (slower) ffindex_fopen */
      /*
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
         printf("%s", line);
         }
         */
  }

  return 0;
}

/* vim: ts=2 sw=2 et
 */
