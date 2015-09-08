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

/* In addition ffutil.h and ffutil.c is licensed under MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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


int ffstrncmp(const char* s1, const char* s2, size_t len)
{
  for(size_t i = len; i > 0 && *s1 && *s2 && *s1 == *s2; i--, s1++, s2++)
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
