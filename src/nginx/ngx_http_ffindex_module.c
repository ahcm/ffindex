
/*
 * Copyright (C) Andreas Hauser
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <limits.h>
#include <stdlib.h>

*/

#include <ffindex.h>
#include <ffutil.h>

typedef struct ffindex_config
{
  ngx_str_t ffdata_filename;
  ngx_str_t ffindex_filename;
  FILE * ffdata_file;
  FILE * ffindex_file;
  char * ffdata;
  size_t ffdata_size;
  ffindex_index_t * ffindex;
} ffindex_config_t;


//static char * ngx_http_ffindex_setup(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void * ngx_http_ffindex_create_loc_conf(ngx_conf_t *cf);
static char * ngx_http_ffindex_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_ffindex_handler(ngx_http_request_t *req);

static ngx_command_t  ngx_http_ffindex_commands[] =
{

    {
      ngx_string("ffdata_filename"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ffindex_config_t, ffdata_filename),
      NULL
    },

    {
      ngx_string("ffindex_filename"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ffindex_config_t, ffindex_filename),
      NULL
    },

    ngx_null_command
};



static ngx_http_module_t  ngx_http_ffindex_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_ffindex_create_loc_conf,  /* create location configuration */
    ngx_http_ffindex_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_ffindex_module = {
    NGX_MODULE_V1,
    &ngx_http_ffindex_module_ctx, /* module context */
    ngx_http_ffindex_commands,   /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static void * ngx_http_ffindex_create_loc_conf(ngx_conf_t *cf)
{
    ffindex_config_t  *mcf;

    mcf = ngx_palloc(cf->pool, sizeof(ffindex_config_t));
    if (mcf == NULL) {
        return NULL;
    }

    mcf->ffdata_filename.len  = 0;
    mcf->ffindex_filename.len = 0;

    return mcf;
}

static char * ngx_http_ffindex_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ffindex_config_t * parent_config = parent;
  ffindex_config_t * child_config = child;
  if(child_config->ffdata_filename.len && child_config->ffindex_filename.len)
  {
    size_t offset;
    fprintf(stderr, "plen: %ld, clen: %s %ld\n", parent_config->ffdata_filename.len, child_config->ffdata_filename.data, child_config->ffdata_filename.len);
    //ffindex_index_open(child_config->ffdata_filename.data, child_config->ffindex_filename.data, "r", &data_file, &index_file, &offset);

    child_config->ffdata_file  = fopen(child_config->ffdata_filename.data,  "r");
    child_config->ffindex_file = fopen(child_config->ffindex_filename.data, "r");

    if( child_config->ffdata_file == NULL)
    {
      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "error ffdata file");
      return NGX_CONF_ERROR;
    }
    if(child_config->ffindex_file == NULL)
    {
      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "error ffindex file");
      return NGX_CONF_ERROR;
    }

    size_t data_size;
    char *data = ffindex_mmap_data(child_config->ffdata_file, &data_size);
    child_config->ffdata_size = data_size;
    child_config->ffdata = data;

    ffindex_index_t* index = ffindex_index_parse(child_config->ffindex_file, 0);
    if(index == NULL)
    {
      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "error parsing ffindex file");
      return NGX_CONF_ERROR;
    }
    child_config->ffindex = index;

    ngx_http_core_loc_conf_t  *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_ffindex_handler;
  }

  return NGX_CONF_OK;
}


static ngx_str_t  ngx_http_ffindex_type = ngx_string("text/plain");

static ngx_int_t ngx_http_ffindex_handler(ngx_http_request_t *req)
{

  ffindex_config_t * ffindex_config = ngx_http_get_module_loc_conf(req, ngx_http_ffindex_module);

  if (!(req->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
    return NGX_HTTP_NOT_ALLOWED;
  }

  if(strncmp("get_data_by_index=", req->args.data, req->args.len))
  {
    size_t index = atol(req->args.data + sizeof("get_data_by_index=")) + 1; // index from 0 request from 1
    ffindex_entry_t * entry = ffindex_get_entry_by_index(ffindex_config->ffindex, index);
    if(entry)
    {
      char * data = ffindex_get_data_by_entry(ffindex_config->ffdata, entry);

      ngx_http_complex_value_t  cv;
      ngx_memzero(&cv, sizeof(ngx_http_complex_value_t));
      cv.value.data = data;
      cv.value.len  = entry->length;

      return ngx_http_send_response(req, NGX_HTTP_OK, &ngx_http_ffindex_type, &cv);
    }
    else
      return NGX_HTTP_NOT_ALLOWED;
  }
  else if(strncmp("get_data_by_name=", req->args.data, req->args.len))
  {
    char * name = req->args.data + sizeof("get_data_by_name=");
    ffindex_entry_t * entry = ffindex_get_entry_by_name(ffindex_config->ffindex, name);
    if(entry)
    {
      char * data = ffindex_get_data_by_entry(ffindex_config->ffdata, entry);

      ngx_http_complex_value_t  cv;
      ngx_memzero(&cv, sizeof(ngx_http_complex_value_t));
      cv.value.data = data;
      cv.value.len  = entry->length;

      return ngx_http_send_response(req, NGX_HTTP_OK, &ngx_http_ffindex_type, &cv);
    }
    else
      return NGX_HTTP_NOT_ALLOWED;
  }
  else
    return NGX_HTTP_NOT_ALLOWED;

}


