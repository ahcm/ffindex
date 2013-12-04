/*
 * FFindex
 * written by Andreas Hauser <andy@splashground.de>.
 * Please add your name here if you distribute modified versions.
 * 
 * FFindex is provided under the Create Commons license "Attribution-ShareAlike
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
    fprintf(stderr, "USAGE: %s -v | [-s] [-k INDEX] data_filename index_filename fasta_filename\n"
                    "\t-k INDEX\tfield index from 1 which is used as key (default = 1)\n"
                    "\t-s\tsort index file (usually you want this)\n"
                    FFINDEX_COPYRIGHT,
                    program_name);
}

char* tsv_scan_line_for_field(char *tsv_current, int field_selected_index, char **field_start, size_t *field_length)
{
  *field_start = tsv_current;
  *field_length = 0;
  int field_index = 1;
  for(int c = *tsv_current; c != '\n'; c = *(tsv_current++))
  {
    if(c == '\t')
    {
      if(field_index == field_selected_index)
        *field_length = tsv_current - *field_start - 1;
      field_index++;
      if(field_index == field_selected_index)
        *field_start = tsv_current;
    }
  }
  if(*field_length == 0)
    *field_length = tsv_current - *field_start - 1;
  return tsv_current;
}

int main(int argn, char **argv)
{
  int sort = 0, version = 0;
  int opt, err = EXIT_SUCCESS;
  int user_selected_field_index = 1;
  while ((opt = getopt(argn, argv, "svk:")) != -1)
  {
    switch (opt)
    {
      case 'k':
        user_selected_field_index = optind;
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

  if(argn - optind < 3)
  {
    usage(argv[0]);
    return EXIT_FAILURE;
  }


  char *data_filename  = argv[optind++];
  char *index_filename = argv[optind++];
  char *tsv_filename = argv[optind++];
  FILE *data_file, *index_file;
  size_t offset = 0;

  /* open ffindex */
  err = ffindex_index_open(data_filename, index_filename, "w", &data_file, &index_file, &offset);
  if(err != EXIT_SUCCESS)
    return err;

  FILE* tsv_file = fopen(tsv_filename, "r");
  if(tsv_file == NULL) { perror(tsv_filename); return EXIT_FAILURE; }

  size_t tsv_size;
  char* tsv_data = ffindex_mmap_data(tsv_file, &tsv_size);
  char* tsv_current = tsv_data;
  char* tsv_part_begin = tsv_data;
  char* tsv_last = tsv_data + tsv_size;

  char field_current[FFINDEX_MAX_ENTRY_NAME_LENTH + 1]; // + seperator
  size_t field_current_length = 0; // + seperator

  char* tsv_selected_field_start = NULL;
  size_t tsv_selected_field_length = 0;
  tsv_current = tsv_scan_line_for_field(tsv_current, user_selected_field_index, &tsv_selected_field_start, &tsv_selected_field_length);
  strncpy(field_current, tsv_selected_field_start, tsv_selected_field_length); //XXX
  field_current_length = tsv_selected_field_length;
  field_current[field_current_length] = '\0';
  while(tsv_current < tsv_last)
  {
    char* tsv_next;
    tsv_next = tsv_scan_line_for_field(tsv_current, user_selected_field_index, &tsv_selected_field_start, &tsv_selected_field_length);
    if((tsv_selected_field_length != field_current_length ||
       strncmp(field_current, tsv_selected_field_start, tsv_selected_field_length) != 0)) // XXX got a new field value
    {
      ffindex_insert_memory(data_file, index_file, &offset, tsv_part_begin, tsv_current - tsv_part_begin, field_current);
      strncpy(field_current, tsv_selected_field_start, tsv_selected_field_length); //XXX
      field_current_length = tsv_selected_field_length;
      field_current[field_current_length] = '\0';
      tsv_part_begin = tsv_current;
    }
    tsv_current = tsv_next;
  }
  ffindex_insert_memory(data_file, index_file, &offset, tsv_part_begin, tsv_current - tsv_part_begin, field_current);
  strncpy(field_current, tsv_selected_field_start, tsv_selected_field_length); //XXX
  field_current_length = tsv_selected_field_length;
  field_current[field_current_length] = '\0';

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

/* vim: ts=2 sw=2 et: */
