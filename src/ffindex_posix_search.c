#include <search.h>
#include <string.h>
#include <stdlib.h>

#include <ffindex_posix_search.h>
#include <ffutil.h>

/* tree version */

ffindex_entry_t *ffindex_tree_get_entry(ffindex_index_t* index, char* name)
{
  ffindex_entry_t search;
  strncpy(search.name, name, FFINDEX_MAX_ENTRY_NAME_LENTH);
  return (ffindex_entry_t *)tfind((const void *)&search, &index->tree_root, ffindex_compare_entries_by_name);
}


ffindex_index_t* ffindex_tree_unlink(ffindex_index_t* index, char* name_to_unlink)
{
  if(index->tree_root == NULL)
  {
    fferror_print(__FILE__, __LINE__, __func__, "tree is NULL");
    return NULL;
  }
  ffindex_entry_t search;
  strncpy(search.name, name_to_unlink, FFINDEX_MAX_ENTRY_NAME_LENTH);
  tdelete((const void *)&search, &index->tree_root, ffindex_compare_entries_by_name);
  return index;
}

ffindex_index_t* ffindex_index_as_tree(ffindex_index_t* index)
{
  index->tree_root = NULL;
  for(size_t i = 0; i < index->n_entries; i++)
  {
    ffindex_entry_t *entry = &index->entries[i];
    tsearch((const void *)entry, &index->tree_root, ffindex_compare_entries_by_name);
    //entry = *(ffindex_entry_t **)tsearch((const void *)entry, &index->tree_root, ffindex_compare_entries_by_name);
    //printf("entry find: %s\n", entry->name); 
  }
  index->type = TREE;
  return index;
}

static __thread FILE * ffindex_tree_write_action_index_file = NULL;
static __thread int ffindex_tree_write_action_ret = EXIT_SUCCESS;

static void ffindex_tree_write_action(const void *node, const VISIT which, const int depth)
{
  switch (which)
  {
    case preorder:
      break;
    case endorder:
      break;
    case postorder:
    case leaf:
      if(ffindex_print_entry(ffindex_tree_write_action_index_file, *(ffindex_entry_t **) node) < 0)
        ffindex_tree_write_action_ret = EXIT_FAILURE;
      break;
  }
}

int ffindex_tree_write(ffindex_index_t* index, FILE* index_file)
{
  ffindex_tree_write_action_ret = EXIT_SUCCESS;
  ffindex_tree_write_action_index_file = index_file;
  twalk(index->tree_root, ffindex_tree_write_action);
  return ffindex_tree_write_action_ret; 
}
