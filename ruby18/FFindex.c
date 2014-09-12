#include "../src/ffindex.h"
#include "ruby.h"

VALUE FFindex = Qnil;

void Init_ffindex();

VALUE method_ffindex_initialize(VALUE self, VALUE ffdata_filename, VALUE ffindex_filename);

static VALUE ffindex_db_allocate(VALUE klass);

static void  ffindex_db_deallocate(void * ffindex_db);

VALUE method_ffindex_get_data_by_index(VALUE self, VALUE key);

VALUE method_ffindex_get_data_by_name(VALUE self, VALUE key);


void Init_ffindex()
{
  FFindex = rb_define_class("FFindex", rb_cObject);
  rb_define_alloc_func(FFindex, ffindex_db_allocate);
  rb_define_method(FFindex, "initialize", method_ffindex_initialize, 2);
  rb_define_method(FFindex, "get_data_by_index", method_ffindex_get_data_by_index, 1);
  rb_define_method(FFindex, "get_data_by_name", method_ffindex_get_data_by_name, 1);
}

static void ffindex_db_deallocate(void * ffindex_db)
{
    free((ffindex_db_t *)ffindex_db);
}


static VALUE ffindex_db_allocate(VALUE klass)
{
  ffindex_db_t * ffindex_db = (ffindex_db_t *)calloc(1, sizeof(ffindex_db_t));
  return Data_Wrap_Struct(klass, NULL, ffindex_db_deallocate, ffindex_db);
}


VALUE method_ffindex_initialize(VALUE self, VALUE ffdata_filename, VALUE ffindex_filename)
{
  Check_Type(ffdata_filename, T_STRING);
  Check_Type(ffindex_filename, T_STRING);

  ffindex_db_t * ffindex_db;
  Data_Get_Struct(self, ffindex_db_t, ffindex_db);

  ffindex_db->ffdata_filename  = calloc(RSTRING_LEN(ffdata_filename) + 1, sizeof(char));
  memcpy(ffindex_db->ffdata_filename, StringValuePtr(ffdata_filename), RSTRING_LEN(ffdata_filename));

  ffindex_db->ffindex_filename = calloc(RSTRING_LEN(ffindex_filename) + 1, sizeof(char));
  memcpy(ffindex_db->ffindex_filename, StringValuePtr(ffindex_filename), RSTRING_LEN(ffindex_filename));

  ffindex_db->mode[0] = 'r';

  ffindex_db = ffindex_index_db_open(ffindex_db);

  return self;
}


VALUE method_ffindex_get_data_by_index(VALUE self, VALUE key)
{
  ffindex_db_t * ffindex_db;
  Data_Get_Struct(self, ffindex_db_t, ffindex_db);
  
  size_t index = FIX2INT(key);
  ffindex_entry_t * entry = ffindex_get_entry_by_index(ffindex_db->ffindex, index);
  if(entry)
  {
    char * data = ffindex_get_data_by_entry(ffindex_db->ffdata, entry);
    return rb_str_new2(data);
  }
  else
    return Qnil;
}


VALUE method_ffindex_get_data_by_name(VALUE self, VALUE key)
{
  Check_Type(key, T_STRING);
  char * name = calloc(RSTRING_LEN(key) + 1, sizeof(char));
  memcpy(name, StringValuePtr(key), RSTRING_LEN(key));

  ffindex_db_t * ffindex_db;
  Data_Get_Struct(self, ffindex_db_t, ffindex_db);
  ffindex_entry_t * entry = ffindex_get_entry_by_name(ffindex_db->ffindex, name);
  if(entry)
  {
    char * data = ffindex_get_data_by_entry(ffindex_db->ffdata, entry);
    return rb_str_new2(data);
  }
  else
    return Qnil;
}
