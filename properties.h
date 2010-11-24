#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <stdio.h>
#include <sys/queue.h>
#include <string.h>
#include <sys/types.h>
#include <evhttp.h>

void properties_parse_line( char *line, struct evkeyvalq *output );
void properties_parse_file( FILE *fh, struct evkeyvalq *output );

#endif

