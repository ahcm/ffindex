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
 * 
 * Ffindex is a very simple database for small files. The files are stored
 * concatenated in one big data file, seperated by '\0'. A second file
 * contains a plain text index, giving name, offset and length of of the small
 * files.
 */

#include <stdio.h>
#include "ffutil.h"

int fferror_print(char *sourcecode_filename, int line, const char *function_name, const char *message)
{
  int myerrno = errno;
  char* errstr = strerror(myerrno);
  fprintf(stderr, "%s:%d %s: %s: %s\n", sourcecode_filename , line, function_name, message, errstr);
  return myerrno;
}


/* remove \n, assumes UNIX line endings! */
char* ffnchomp(char *s, size_t len)
{
  if(len >= 1 && s[--len] == '\n')
    s[len] = '\0';

  return s;
}

/* replacement for strtol streamlined for speed */
unsigned long ffparse_ulong(const char *s, const char** end)
{
  const unsigned int base = 10;
  unsigned long l = 0UL;
  unsigned char *c = (unsigned char*)s;
  for(unsigned int digit = *c-'0'; *c && digit < base; digit = *++c-'0')
    l = (l * base) + digit;
  *end = (char *)c;
  return l;
}


int ffstrncmp(char* s1, char* s2, size_t len)
{
  for(int i = 0; i < len && *s1 && *s2 && *s1 == *s2; i++, s1++, s2++)
    ;
  return *s1 - *s2;
}
    

size_t fffprint_ulong(FILE* file, unsigned long l)
{
  char p[21];
  char *c = p + 20;
  while(l >= 10)
  {
    *c-- = '0' + (l % 10);
    l /= 10;
  }
  *c = '0' + l;
  return fwrite(c, 1, p + 21 - c, file);
}


/* vim: ts=2 sw=2 et
*/
