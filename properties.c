/*
Copyright (c) 2010 Harry Roberts.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of Zend Technologies USA, Inc. nor the names of its
      contributors may be used to endorse or promote products derived from this
      software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "properties.h"

#include <assert.h>
#include <ctype.h>

void
properties_parse_line( char *line, struct evkeyvalq *output ) {
	char *key, *value, *end;
	key = line;
	while( isspace(*key) ) key++;
	if( *key == ';' ) return;
	value = key;
	while( isalnum(*value) || *value == '_' ) value++;
	while( isspace(*value) ) { *value++ = 0; };
	if( *value != '=' ) return;
	*value++ = 0;
	while( isspace(*value) ) value++;
	end = value + strlen(value) - 1;
	while( end > value && isspace(*end) ) end--;
	*(end+1) = 0;
	
	if( strlen(key) && strlen(value) ) {
		evhttp_add_header(output, key, value);
	}
}

void
properties_parse_file( FILE *fh, struct evkeyvalq *output ) {
	char line[1024];
	assert( output != NULL );
	assert( fh != NULL );
	
	while( fgets(line, sizeof(line) / sizeof(char), fh) != NULL ) {
		properties_parse_line(line, output);
	}		
}