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
 * 
 * Ffindex is a very simple database for small files. The files are stored
 * concatenated in one big data file, seperated by '\0'. A second file
 * contains a plain text index, giving name, offset and length of of the small
 * files.
 */

#include <stdio.h>
#include "fferror.h"

int fferror_print(char *sourcecode_filename, int line, const char *function_name, char *message)
{
        int myerrno = errno;
        char* errstr = strerror(myerrno);
        fprintf(stderr, "%s:%d %s: %s: %s\n", sourcecode_filename , line, function_name, message, errstr);
	return myerrno;
}

