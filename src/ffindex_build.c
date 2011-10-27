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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "ffindex.h"
#include "ffutil.h"

#define MAX_FILENAME_LIST_FILES 4096


void usage(char *program_name)
{
    fprintf(stderr, "USAGE: %s [-a|-v] [-s] [-f file]* data_filename index_filename [dir_to_index|file]*\n"
                    "\t-a\tappend\n"
                    "\t-d a second ffindex data file for inserting/appending\n"
                    "\t-i a second ffindex index file for insterting/appending\n"
                    "\t-f file\tfile each line containing a filename to index\n"
                    "\t\t-f can be specified up to %d times\n"
                    "\t-s\tsort index file\n"
                    "\t-v\tprint version and other info then exit\n"
                    "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n", program_name, MAX_FILENAME_LIST_FILES);
}

int main(int argn, char **argv)
{
  int append = 0, sort = 0, unlink = 0, version = 0;
  int opt, err = EXIT_SUCCESS;
  char* list_filenames[MAX_FILENAME_LIST_FILES];
  char* list_ffindex_data[MAX_FILENAME_LIST_FILES];
  char* list_ffindex_index[MAX_FILENAME_LIST_FILES];
  size_t list_ffindex_data_index = 0;
  size_t list_ffindex_index_index = 0;
  size_t list_filenames_index = 0;
  while ((opt = getopt(argn, argv, "asuvd:f:i:")) != -1)
  {
    switch (opt)
    {
      case 'a':
        append = 1;
        break;
      case 'd':
        list_ffindex_data[list_ffindex_data_index++] = optarg;
        break;
      case 'i':
        list_ffindex_index[list_ffindex_index_index++] = optarg;
        break;
      case 'f':
        list_filenames[list_filenames_index++] = optarg;
        break;
      case 's':
        sort = 1;
        break;
      case 'v':
        version = 1;
        break;
      default:
        usage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  if(version == 1)
  {
    /* Don't you dare running it on a platform where byte != 8 bits */
    printf("%s version %.2f, off_t = %zd bits\n", argv[0], FFINDEX_VERSION, sizeof(off_t) * 8);
    return EXIT_SUCCESS;
  }

  if(argn - optind < 2)
  {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  if(append && unlink)
  {
    fprintf(stderr, "ERROR: append (-a) and unlink (-u) are mutually exclusive\n");
    return EXIT_FAILURE;
  }

  if(list_ffindex_data_index != list_ffindex_index_index)
  {
    fprintf(stderr, "ERROR: -d and -i must be specified pairwise\n");
    return EXIT_FAILURE;
  }

  char *data_filename  = argv[optind++];
  char *index_filename = argv[optind++];
  FILE *data_file, *index_file;

  size_t offset = 0;

  /* open index and data file, seek to end if needed */
  if(append)
  {
    data_file  = fopen(data_filename, "a");
    if( data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    index_file = fopen(index_filename, "a+");
    if(index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }

    struct stat sb;
    fstat(fileno(data_file), &sb);
    fseek(data_file, sb.st_size, SEEK_SET);
    offset = sb.st_size;

    fstat(fileno(index_file), &sb);
    fseek(index_file, sb.st_size, SEEK_SET);
  }
  else
  {
    struct stat st;

    if(stat(data_filename, &st) == 0) { errno = EEXIST; perror(data_filename); return EXIT_FAILURE; }
    data_file  = fopen(data_filename, "w");
    if( data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    if(stat(index_filename, &st) == 0) { errno = EEXIST; perror(index_filename); return EXIT_FAILURE; }
    index_file = fopen(index_filename, "w+");
    if(index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }
  }


  /* For each list_file insert */
  if(list_filenames_index > 0)
    for(int i = 0; i < list_filenames_index; i++)
    {
      FILE *list_file = fopen(list_filenames[i], "r");
      if( list_file == NULL) { perror(list_filenames[i]); return EXIT_FAILURE; }
      if(ffindex_insert_list_file(data_file, index_file, &offset, list_file) < 0)
      {
        perror(list_filenames[i]);
        err = -1;
      }
    }

  /* Append other ffindexes */
  if(list_ffindex_data_index > 0)
  {
    for(int i = 0; i < list_ffindex_data_index; i++)
    {
      FILE* data_file_to_add  = fopen(list_ffindex_data[i], "r");  if(  data_file_to_add == NULL) { perror(list_ffindex_data[i]); return EXIT_FAILURE; }
      FILE* index_file_to_add = fopen(list_ffindex_index[i], "r"); if( index_file_to_add == NULL) { perror(list_ffindex_index[i]); return EXIT_FAILURE; }
      size_t data_size;
      char *data_to_add = ffindex_mmap_data(data_file_to_add, &data_size);
      ffindex_index_t* index_to_add = ffindex_index_parse(index_file_to_add, 0);
      for(size_t entry_i = 0; entry_i < index_to_add->n_entries; entry_i++)
      {
        ffindex_entry_t *entry = &index_to_add->entries[entry_i];
        FILE *file = ffindex_fopen(data_to_add, index_to_add, entry->name);
        ffindex_insert_filestream(data_file, index_file, &offset, file, entry->name);
        fclose(file);
      }
    }
  }


  /* Insert files and directories into the index */
  for(int i = optind; i < argn; i++)
  {
    char *path = argv[i];
    struct stat sb;
    if(stat(path, &sb) == -1)
    {
      fferror_print(__FILE__, __LINE__, __func__, path);
      continue;
    }

    if(S_ISDIR(sb.st_mode))
    {
      err = ffindex_insert_dir(data_file, index_file, &offset, path);
      if(err < 0)fferror_print(__FILE__, __LINE__, __func__, path);
    }
    else if(S_ISREG(sb.st_mode))
    {
      ffindex_insert_file(data_file, index_file, &offset, path, path);
    }
  }
  fclose(data_file);

  /* Sort the index entries and write back */
  if(sort)
  {
    rewind(index_file);
    ffindex_index_t* index = ffindex_index_parse(index_file, 0);
    if(index == NULL)
    {
      perror("ffindex_index_parse failed");
      exit(EXIT_FAILURE);
    }
    fclose(index_file);
    ffindex_sort_index_file(index);
    index_file = fopen(index_filename, "w");
    if(index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }
    err += ffindex_write(index, index_file);
  }

  return err;
}

/* vim: ts=2 sw=2 et
 */
