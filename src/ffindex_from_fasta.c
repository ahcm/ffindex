/*
 * FFindex
 * written by Andreas Hauser <andy@splashground.de>.
 * Please add your name here if you distribute modified versions.
 * 
 * FFindex is provided under the Create Commons license "Attribution-ShareAlike
 * 4.0", which basically captures the spirit of the Gnu Public License (GPL).
 * 
 * See:
 * http://creativecommons.org/licenses/by-sa/4.0/
*/

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

#include "ffindex.h"
#include "ffutil.h"

#define MAX_FILENAME_LIST_FILES 4096


void usage(char *program_name)
{
    fprintf(stderr, "USAGE: %s -v | [-s] data_filename index_filename fasta_filename\n"
                    "\t-s\tsort index file\n"
                    "\t-i\tuse identifier as name (you probably want -s too)\n"
                    "\t-n\tuse incremental number as id (default)\n"
                    "\t-v\tprint version\n"
                    FFINDEX_COPYRIGHT,
                    program_name);
}

int main(int argn, char **argv)
{
  int sort = 0, version = 0, use_identifier = 0;
  int opt, err = EXIT_SUCCESS;
  while ((opt = getopt(argn, argv, "sinv")) != -1)
  {
    switch (opt)
    {
      case 'n':
        break;
      case 'i':
        use_identifier = 1;
        break;
      case 's':
        sort = 1;
        break;
      case 'v':
        version = 1;
        break;
      default:
        usage(basename(argv[0]));
        return EXIT_FAILURE;
    }
  }

  if(version == 1)
  {
    /* Don't you dare running it on a platform where byte != 8 bits */
    printf("%s version %.2f, off_t = %zd bits\n", argv[0], FFINDEX_VERSION, sizeof(off_t) * 8);
    return EXIT_SUCCESS;
  }

  if(argn - optind < 3)
  {
    usage(basename(argv[0]));
    return EXIT_FAILURE;
  }


  char *data_filename  = argv[optind++];
  char *index_filename = argv[optind++];
  char *fasta_filename = argv[optind++];
  FILE *data_file, *index_file, *fasta_file;
  size_t offset = 0;

  /* open ffindex */
  err = ffindex_index_open(data_filename, index_filename, "w", &data_file, &index_file, &offset);
  if(err != EXIT_SUCCESS)
    return err;

  fasta_file = fopen(fasta_filename, "r");
  if(fasta_file == NULL) { perror(fasta_filename); return EXIT_FAILURE; }

  size_t fasta_size;
  char *fasta_data = ffindex_mmap_data(fasta_file, &fasta_size);
  size_t from_length = 0;
  char name[FFINDEX_MAX_ENTRY_NAME_LENTH];
  int seq_id = 0;
  for(size_t fasta_offset = 1; fasta_offset < fasta_size; fasta_offset++) // position after first ">"
  {
    seq_id++;
    from_length = 1;
    while(fasta_offset < fasta_size && !(*(fasta_data + fasta_offset) == '>' && *(fasta_data + fasta_offset - 1) == '\n'))
    {
      fasta_offset++;
      from_length++;
    }
    if(use_identifier)
    {
      size_t len = strcspn(fasta_data + (fasta_offset + 1 - from_length), " \n");
      if(len > FFINDEX_MAX_ENTRY_NAME_LENTH)
        len = FFINDEX_MAX_ENTRY_NAME_LENTH - 1;
      strncpy(name, fasta_data + (fasta_offset + 1 - from_length), len);
      name[len] = '\0';
    }
    else
      sprintf(name, "%d", seq_id);
    ffindex_insert_memory(data_file, index_file, &offset, fasta_data + (fasta_offset - from_length), from_length, name);
  }
  fclose(data_file);

  /* Sort the index entries and write back */
  if(sort)
  {
    rewind(index_file);
    ffindex_index_t* index = ffindex_index_parse(index_file, seq_id);
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

/* vim: ts=2 sw=2 et: */
