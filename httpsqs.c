/*
 * HTTPSQS by Zhang Yan
 * -- http://code.google.com/p/httpsqs/
 * -- http://blog.s135.com/
 * -- http://twitter.com/rewinx
 *
 * Improved by Harry Roberts <harry@midnight-labs.org>
 * -- http://midnight-labs.org/

Copyright (c) Zhang Yan.
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

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>

#include <err.h>
#include <event.h>
#include <evhttp.h>

#include "ydb.h"
#include "rbtree.h"

#define SQS_VERSION "1.3.2.panda"

//using namespace Lux::IO;

YDB httpsqs_db_tcbdb;
const char *httpsqs_settings_pidfile;

struct sqs_queue_status {
	char *name;
	uint64_t tail_id;
	uint64_t head_id;
	uint32_t maxqueue;
	uint32_t count;
	uint64_t count_bytes;
	
	/* Totals for the lifetime of the queue */
	uint64_t total_get;
	uint64_t total_put;
	uint64_t total_info;
	
	/* RB node */
	struct rb_node rbnode;
};

struct rb_root sqs_queues_tree = RB_ROOT;

#define MIN(a,b) ((a) <= (b)? (a) : (b))
static int key_cmp(const char *a, int a_sz, const char *b, int b_sz) {
        int r = memcmp(a, b, MIN(a_sz, b_sz));
        if(r == 0) {
                if(a_sz == b_sz)
                        return(0);
                if(a_sz < b_sz)
                        return(-1);
                return(1);
        }
        return(r);
}

struct sqs_queue_status*
get_queue_status( const char *name ) {		
	int result = 0; 
	int name_sz = strlen(name);
	struct rb_node **node = &(sqs_queues_tree.rb_node), *parent = NULL;
	while( *node ) {
		struct sqs_queue_status* item = container_of(*node, struct sqs_queue_status, rbnode);
		result = key_cmp(name, name_sz, item->name, strlen(item->name));
		
		parent = *node;
		if( result < 0 )
			node = &((*node)->rb_left);
		else if( result > 0 )
			node = &((*node)->rb_right);
		else
			return item;
	}
	
	struct sqs_queue_status* st;
	st = malloc(sizeof(struct sqs_queue_status));
	st->name = strdup(name);
	st->tail_id = 0;
	st->head_id = 0;
	st->maxqueue = 100000;
	st->count = 0;
	st->count_bytes = 0;
	
	st->total_get = 0;
	st->total_put = 0;
	st->total_info = 0;
	
	rb_link_node(&st->rbnode, parent, node);
	rb_insert_color(&st->rbnode, &sqs_queues_tree);
		
	return st;
}

static void
create_multilayer_dir( char *muldir ) 
{
    int    i,len;
    char    str[512];
    
    strncpy( str, muldir, 512 );
    len=strlen(str);
    for( i=0; i<len; i++ ) {
        if( str[i]=='/' ) {
            str[i] = '\0';
            if( access(str, F_OK)!=0 ) {
                mkdir( str, 0777 );
            }
            str[i]='/';
        }
    }
    if( len>0 && access(str, F_OK)!=0 ) {
        mkdir( str, 0777 );
    }

    return;
}

static void
show_help(void)
{
	const char *b = "--------------------------------------------------------------------------------------------------\n"
		  "HTTP Simple Queue Service - httpsqs v" SQS_VERSION " (November, 2010)\n\n"
		  "Author: Zhang Yan (http://blog.s135.com), E-mail: net@s135.com\n"
		  "Contributor: Harry Roberts <httpsqs@midnight-labs.org>\n"
		  "This is free software, and you are welcome to modify and redistribute it under the New BSD License\n"
		  "\n"
		   "-l <ip_addr>  interface to listen on, default is 0.0.0.0\n"
		   "-p <num>      TCP port number to listen on (default: 1218)\n"
		   "-x <path>     database directory (example: /opt/httpsqs/data)\n"
		   "-t <second>   timeout for an http request (default: 3)\n"
		   "-i <file>     save PID in <file> (default: /tmp/httpsqs.pid)\n"
		   "-s <num>      YDB log block size in megabytes (default: 5)\n"
		   "-o <num>	  YDB overcommit size (default: 3)\n"
		   "-d            run as a daemon\n"
		   "-h            print this help and exit\n\n"
		   "Use command \"killall httpsqs\", \"pkill httpsqs\" and \"kill `cat /tmp/httpsqs.pid`\" to stop httpsqs.\n"
		   "Please note that don't use the command \"pkill -9 httpsqs\" and \"kill -9 PID of httpsqs\"!\n"
		   "\n"
		   "Please visit \"http://code.google.com/p/httpsqs\" for more help information.\n\n"
		   "--------------------------------------------------------------------------------------------------\n"
		   "\n";
	fprintf(stderr, b, strlen(b));
}

static uint32_t
httpsqs_maxqueue(struct sqs_queue_status* st, uint32_t httpsqs_input_num) {
	if( st->count > httpsqs_input_num ) return 0;
	return st->maxqueue = httpsqs_input_num;
}

static bool
httpsqs_reset(struct sqs_queue_status* st) {
	rb_erase(&st->rbnode, &sqs_queues_tree);
	free(st->name);
	free(st);
	return true;	
}

static void
send_error(struct evhttp_request *req, int code, const char *reason, const char *message) {
	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf,"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
							"<html><head>"
							"<title>%d %s</title>"
							"</head><body>" 
							"<h1>%s</h1>" 
							"<p>%s</p>"
							"</body></html>", code, reason, reason, message);
	evhttp_send_reply(req, code, reason, buf);	
	evbuffer_free(buf);					
}

static void
send_content_length(struct evhttp_request *req, int content_length) {
	char *cl_header_num;
	asprintf(&cl_header_num, "%d", content_length);
	evhttp_add_header(req->output_headers, "Content-Length", cl_header_num);
	free(cl_header_num);
}

static void
handle_get(struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args, bool do_remove) {	
	uint64_t queue_get_value;
	
	if (qs->count == 0) {
		send_error(req, 204, "No Content", "There are no items in the queue to retrieve");
		return;
	}
	
	const char *arg_id = evhttp_find_header(args, "id");
	if( arg_id != NULL ) {
		if( sscanf(arg_id, "%llX", &queue_get_value) != 1 ) {
			arg_id = NULL;
		}
	}
	
	if( arg_id == NULL ) {
		queue_get_value = qs->tail_id;
	}
		
	char *queue_name;
	asprintf(&queue_name, "%s:%llX", qs->name, queue_get_value);
	
	char *content = NULL;
	int content_length = 0;
	
	if( ! ydb_geta(httpsqs_db_tcbdb, queue_name, strlen(queue_name), &content, &content_length) || ! content ) {	
		if( arg_id != NULL ) {
			send_error(req, 500, "404 Not Found", "Could not find ID");
			return;
		}
		
		send_error(req, 500, "Internal Server Error", "DB get failed or returned no content");
		return;
	}

	struct evbuffer *buf = evbuffer_new();		
	
	uint8_t ct_length;
	uint32_t data_length;
	char *content_type = NULL;
	char *data = NULL;
	
	ct_length = *(uint8_t*)(content);
	data_length = *(uint32_t*)(content + sizeof(uint8_t));
	if( ct_length > 0 ) {
		content_type = (content + sizeof(uint8_t) + sizeof(uint32_t));
		evhttp_add_header(req->output_headers, "Content-Type", content_type);
	}
	else {
		evhttp_add_header(req->output_headers, "Content-Type", "application/octet-stream");
	}
	data = (content + sizeof(uint8_t) + sizeof(uint32_t) + ct_length);
	
	send_content_length( req, data_length );
	evbuffer_add(buf, data, data_length);
	free(content);
	
	if( do_remove ) {
		ydb_del(httpsqs_db_tcbdb, queue_name, strlen(queue_name));
		qs->count--;	
		qs->tail_id++;
		qs->count_bytes -= content_length;
	}			
	qs->total_get++;

	free(queue_name);

	evhttp_send_reply(req, HTTP_OK, "OK", buf);	
	evbuffer_free(buf);
}

static void
handle_put(struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
	const char *input_data;
	const char *input_content_type;
	uint8_t input_content_type_length = 0;
 	uint32_t input_data_length = 0;

	if( qs->count >= qs->maxqueue ) {
		send_error(req, 400, "Bad Request", "The queue is full!");
		return;
	}
	
	if( req->type == EVHTTP_REQ_GET ) {
		input_data = evhttp_find_header(args, "data");
		if( input_data != NULL ) {
			input_data_length = strlen(input_data);
		}		
		input_content_type = evhttp_find_header(args, "content-type");		
	}
	else if( req->type == EVHTTP_REQ_POST ) {
		input_content_type = evhttp_find_header(req->input_headers, "Content-Type");
		input_data = (char*)EVBUFFER_DATA(req->input_buffer);
		input_data_length = EVBUFFER_LENGTH(req->input_buffer);
	}
	
	if( input_content_type != NULL ) {
		input_content_type_length = strlen(input_content_type) + 1;
	}
	
	if( input_data_length <= 0 ) {
		send_error(req, 400, "Bad Request", "The data field is empty");
		return;
	}
	
	uint64_t queue_put_value = qs->head_id;
	
	struct evbuffer *row = evbuffer_new();
	evbuffer_add(row, &input_content_type_length, sizeof(uint8_t));
	if( input_content_type_length > 0 ) {
		evbuffer_add(row, &input_content_type_length, input_content_type_length);
	}

	evbuffer_add(row, &input_data_length, sizeof(uint32_t));
	evbuffer_add(row, input_data, input_data_length);

	char *queue_name;
	asprintf(&queue_name, "%s:%llX", qs->name, queue_put_value);
	size_t row_size = sizeof(uint8_t) + input_content_type_length + sizeof(uint32_t) + input_data_length;

	ydb_add(httpsqs_db_tcbdb, queue_name, strlen(queue_name), (char*)EVBUFFER_DATA(row), row_size);
	qs->count = qs->count + 1;;
	qs->count_bytes += row_size;
	qs->total_put = qs->total_put + 1;
	qs->head_id = qs->head_id + 1;

	free(queue_name);
	evbuffer_free(row);

	struct evbuffer *buf = evbuffer_new();
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");	
	evbuffer_add_printf(buf, "{'success':true,'id':'%llX'}", queue_put_value);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

static void
handle_status(struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
	struct evbuffer *buf = evbuffer_new();
	qs->total_info++;
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");	
	
	evbuffer_add_printf(buf, "{\"success\":true,\"name\":\"%s\",\"maxqueue\":%u,\"head_id\":%llu,\"tail_id\":%llu,\"count\":%u,\"count_bytes\":%llu}\n",
							 qs->name, qs->maxqueue, qs->head_id, qs->tail_id, qs->count, qs->count_bytes);	
	send_content_length(req, EVBUFFER_LENGTH(buf));
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

static void
handle_reset(struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {	
	if (httpsqs_reset(qs)) {
		qs->total_info++;
		
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "%s", "{'success':true}");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);	
		evbuffer_free(buf);		
	} else {
		send_error(req, 500, "Internal Server Error", "Could not reset queue");
	}	
}

static void
handle_maxqueue(struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
	qs->total_info++;
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");	
	
	const char *input_maxqueue = evhttp_find_header(args, "num");		
	int maxqueue = input_maxqueue == NULL ? 0 : atoi(input_maxqueue);
	if ( maxqueue != 0 && httpsqs_maxqueue(qs, maxqueue) != 0) {
		handle_status(req, qs, args);
		return;
	} 	

	send_error(req, 400, "Internal Server Error", "New maxqueue too small or invalid");
}

static void
httpsqs_handler(struct evhttp_request *req, void *arg) {
	char *decode_uri = evhttp_decode_uri(evhttp_request_uri(req));
	char *uri_query = strchr(decode_uri, '?');
	char *uri_path = decode_uri;
	
	struct evkeyvalq httpsqs_http_query;
	evhttp_parse_query(decode_uri, &httpsqs_http_query);
	
	if( uri_query != NULL ) {		
		uri_query[0] = 0;
		uri_query++;
	}
	
	char path_queue[11];
	char path_action[11];	
	int path_count = sscanf(uri_path, "/%10[a-z0-9]/%10[a-z]", &path_queue[0], &path_action[0]);
	
	if( path_count != 2 ) {
		send_error(req, 400, "Bad Request", "Invalid URL specified");
		return;
	}
	
	struct sqs_queue_status *st = get_queue_status(path_queue);
	if( st == NULL ) {
		send_error(req, 404, "Not Found", "The queue couldn't be found");
		return;
	}
	
	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
	evhttp_add_header(req->output_headers, "Server:", "HTTPsqs/" SQS_VERSION);
	
	if( strcmp(path_action,"get") == 0 ) {
		handle_get(req, st, &httpsqs_http_query, true);
	}
	else if( strcmp(path_action,"put") == 0 ) {
		handle_put(req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action,"status") == 0 ) {
		handle_status(req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action, "reset") == 0 ) {
		handle_reset(req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action, "view") == 0 ) {
		handle_get(req, st, &httpsqs_http_query, false);
	}
	else if( strcmp(path_action, "maxqueue") == 0 ) {
		handle_maxqueue(req, st, &httpsqs_http_query);
	}
	else {
		send_error(req, 404, "Not Found", "Must be one of: get, put, status, reset, view, maxqueue");
		return;
	}
	
	free(decode_uri);	
}

static void
kill_signal(const int sig) {
	ydb_sync(httpsqs_db_tcbdb);
	ydb_close(httpsqs_db_tcbdb);

	remove(httpsqs_settings_pidfile);
	
    exit(0);
}

static void
sync_signal(const int sig) {
	ydb_sync(httpsqs_db_tcbdb);
}

int main(int argc, char **argv)
{
	int c;

	const char *httpsqs_settings_listen = "0.0.0.0";
	int httpsqs_settings_port = 1218;
	char *httpsqs_settings_datapath = NULL;
	bool httpsqs_settings_daemon = false;
	int httpsqs_settings_timeout = 3;
	httpsqs_settings_pidfile = "/tmp/httpsqs.pid";
	int httpsqs_ydb_min_log_size = 5;
	int httpsqs_ydb_overcommit = 3;

    /* process arguments */
    while ((c = getopt(argc, argv, "l:p:x:t:s:c:m:i:dh")) != -1) {
        switch (c) {
        case 'l':
            httpsqs_settings_listen = strdup(optarg);
            break;
        case 'p':
            httpsqs_settings_port = atoi(optarg);
            break;
        case 'x':
            httpsqs_settings_datapath = strdup(optarg);
			if (access(httpsqs_settings_datapath, W_OK) != 0) {
				if (access(httpsqs_settings_datapath, R_OK) == 0) {
					chmod(httpsqs_settings_datapath, S_IWOTH);
				} else {
					create_multilayer_dir(httpsqs_settings_datapath);
				}
	
				if (access(httpsqs_settings_datapath, W_OK) != 0) {
					fprintf(stderr, "httpsqs database directory not writable\n");
				}
			}
            break;
		case 's':
			httpsqs_ydb_min_log_size = atoi(optarg);
			assert( httpsqs_ydb_min_log_size >= 1 );
			break;
		case 'o':
			httpsqs_ydb_overcommit = atoi(optarg);
			assert( httpsqs_ydb_overcommit >= 1);
			break;
        case 't':
            httpsqs_settings_timeout = atoi(optarg);
            break;			
        case 'i':
            httpsqs_settings_pidfile = strdup(optarg);
            break;			
        case 'd':
            httpsqs_settings_daemon = true;
            break;
		case 'h':
        default:
            show_help();
            return 1;
        }
    }
	
	if (httpsqs_settings_datapath == NULL) {
		show_help();
		fprintf(stderr, "Attention: Please use the indispensable argument: -x <path>\n\n");		
		exit(1);
	}
	
	httpsqs_db_tcbdb = ydb_open(httpsqs_settings_datapath, httpsqs_ydb_overcommit, 1024 * 1024 * httpsqs_ydb_min_log_size, YDB_CREAT);
	if( ! httpsqs_db_tcbdb ) {
		fprintf(stderr, "Error: unable to open database!");
		exit(1);
	}
	
	if( httpsqs_settings_daemon == true ){
        pid_t pid;

        /* Fork off the parent process */       
        pid = fork();
        if (pid < 0) {
			exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
			exit(EXIT_SUCCESS);
        }
	}
	
	FILE *fp_pidfile;
	fp_pidfile = fopen(httpsqs_settings_pidfile, "w");
	fprintf(fp_pidfile, "%d\n", getpid());
	fclose(fp_pidfile);
	
	signal(SIGPIPE, SIG_IGN);
	
	signal (SIGINT, kill_signal);
	signal (SIGKILL, kill_signal);
	signal (SIGQUIT, kill_signal);
	signal (SIGTERM, kill_signal);
	signal (SIGHUP, kill_signal);
	
	signal(SIGALRM, sync_signal);
	
    struct evhttp *httpd;

    event_init();
    httpd = evhttp_start(httpsqs_settings_listen, httpsqs_settings_port);
	if (httpd == NULL) {
		fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", httpsqs_settings_listen, httpsqs_settings_port);		
		exit(1);		
	}
	
	evhttp_set_timeout(httpd, httpsqs_settings_timeout);
    evhttp_set_gencb(httpd, httpsqs_handler, NULL);

    event_dispatch();
    evhttp_free(httpd);
    return 0;
}
