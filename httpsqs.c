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
#include <strings.h>
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

#include "httpsqs.h"

struct httpsqs*
httpsqs_init(void) {
	struct httpsqs* n = malloc(sizeof(struct httpsqs));	
	assert( n != NULL );
	memset(n, 0, sizeof(struct httpsqs));
	TAILQ_INIT(&n->params);
	n->queues = RB_ROOT;	
	return n;
}

int
httpsqs_set_option(struct httpsqs* h, const char *name, const char *value) {
	return evhttp_add_header(&h->params, name, value);
}

const char *
httpsqs_get_option(struct httpsqs* h, const char *name, const char *default_value) {
	const char *value = evhttp_find_header(&h->params, name);
	if( value == NULL ) return default_value;
	return value;
}

int
httpsqs_get_option_bool(struct httpsqs* h, const char *name, int default_value) {
	const char *value = httpsqs_get_option(h, name, NULL);
	if( value == NULL ) {
		return default_value;
	}
	
	if( strcasecmp(value,"true") == 0 ) return 1==1;
	if( strcmp(value,"1") == 0 ) return 1==1;
	return 1==0;
}

int
httpsqs_get_option_int(struct httpsqs *h, const char *name, int default_value) {
	const char *value = httpsqs_get_option(h, name, NULL);
	if( value == NULL ) {
		return default_value;
	}
	
	return atoi(value);
}

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
httpsqs_get_queue( struct httpsqs *h, const char *name ) {	
	int result = 0; 
	int name_sz = strlen(name);
	struct rb_node **node = &(h->queues.rb_node);
	struct rb_node *parent = NULL;
	while( *node ) {
		struct sqs_queue_status* item = container_of(*node, struct sqs_queue_status, rbnode);
		result = key_cmp(name, name_sz, item->name, item->name_sz);
		
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
	strncpy(st->name, name, 10);
	st->name_sz = name_sz;
	st->tail_id = 0;
	st->head_id = 0;
	st->maxqueue = 100000;
	st->count = 0;
	st->count_bytes = 0;
	
	st->total_get = 0;
	st->total_get_bytes = 0;
	st->total_put = 0;
	st->total_put_bytes = 0;
	st->total_info = 0;
	
	rb_link_node(&st->rbnode, parent, node);
	rb_insert_color(&st->rbnode, &h->queues);
		
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

struct httpsqs_db *
httpsqs_get_db(struct httpsqs *h) {
	assert( h != NULL );
	assert( h->db != NULL );
	return h->db;
}

void*
httpsqs_get_db_internal(struct httpsqs *h) {
	struct httpsqs_db *db = httpsqs_get_db(h);
	assert( db->internal_handle != NULL );
	return db->internal_handle;
}

static uint32_t
httpsqs_maxqueue(struct sqs_queue_status* st, uint32_t httpsqs_input_num) {
	if( st->count > httpsqs_input_num ) return 0;
	return st->maxqueue = httpsqs_input_num;
}

static bool
httpsqs_reset(struct httpsqs* h, struct sqs_queue_status* st) {
	rb_erase(&st->rbnode, &h->queues);
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
handle_get(struct httpsqs* h, struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args, bool do_remove) {	
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
	size_t content_length = 0;
	
	struct httpsqs_db *db = httpsqs_get_db(h);
	if( ! db->get(h, queue_name, strlen(queue_name), &content, &content_length) || ! content ) {	
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
		//ydb_del(httpsqs_db_tcbdb, queue_name, strlen(queue_name));
		db->del(h, queue_name, strlen(queue_name));
		qs->count--;	
		qs->tail_id++;
		qs->count_bytes -= content_length;
	}			
	qs->total_get_bytes += content_length;
	qs->total_get++;

	free(queue_name);

	evhttp_send_reply(req, HTTP_OK, "OK", buf);	
	evbuffer_free(buf);
}

static void
handle_put(struct httpsqs* h, struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
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

	struct httpsqs_db *db = httpsqs_get_db(h);
	db->add(h, queue_name, strlen(queue_name), (char*)EVBUFFER_DATA(row), row_size);
	//ydb_add(httpsqs_db_tcbdb, queue_name, strlen(queue_name), (char*)EVBUFFER_DATA(row), row_size);
	qs->count++;
	qs->count_bytes += row_size;
	qs->total_put_bytes += row_size;
	qs->total_put++;	
	qs->head_id++;

	free(queue_name);
	evbuffer_free(row);

	struct evbuffer *buf = evbuffer_new();
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");	
	evbuffer_add_printf(buf, "{'success':true,'id':'%llX'}", queue_put_value);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

static void
handle_status(struct httpsqs* h, struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
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
handle_reset(struct httpsqs* h, struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {	
	if (httpsqs_reset(h, qs)) {
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
handle_maxqueue(struct httpsqs* h, struct evhttp_request *req, struct sqs_queue_status *qs, struct evkeyvalq *args) {
	qs->total_info++;
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");	
	
	const char *input_maxqueue = evhttp_find_header(args, "num");		
	int maxqueue = input_maxqueue == NULL ? 0 : atoi(input_maxqueue);
	if ( maxqueue != 0 && httpsqs_maxqueue(qs, maxqueue) != 0) {
		handle_status(h, req, qs, args);
		return;
	} 	

	send_error(req, 400, "Internal Server Error", "New maxqueue too small or invalid");
}

static void
httpsqs_handler(struct evhttp_request *req, void *arg) {
	struct httpsqs* h = (struct httpsqs *)arg;
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
	
	struct sqs_queue_status *st = httpsqs_get_queue(h, path_queue);
	if( st == NULL ) {
		send_error(req, 404, "Not Found", "The queue couldn't be found");
		return;
	}
	
	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
	evhttp_add_header(req->output_headers, "Server:", "HTTPsqs/" SQS_VERSION);
	
	if( strcmp(path_action,"get") == 0 ) {
		handle_get(h, req, st, &httpsqs_http_query, true);
	}
	else if( strcmp(path_action,"put") == 0 ) {
		handle_put(h, req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action,"status") == 0 ) {
		handle_status(h, req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action, "reset") == 0 ) {
		handle_reset(h, req, st, &httpsqs_http_query);
	}
	else if( strcmp(path_action, "view") == 0 ) {
		handle_get(h, req, st, &httpsqs_http_query, false);
	}
	else if( strcmp(path_action, "maxqueue") == 0 ) {
		handle_maxqueue(h, req, st, &httpsqs_http_query);
	}
	else {
		send_error(req, 404, "Not Found", "Must be one of: get, put, status, reset, view, maxqueue");
		return;
	}
	
	free(decode_uri);	
}

static void
kill_signal(int sig, short what, void *arg) {
	struct httpsqs *h = (struct httpsqs*)h;
	struct httpsqs_db *db = httpsqs_get_db(h);
	db->sync(h);
	db->close(h);

	const char *pid_file = httpsqs_get_option(h, "pid_file", NULL);
	if( pid_file != NULL ) {
		remove(pid_file);
	}
	
    exit(0);
}

/*
static void
sync_signal(int sig, short what, void *arg) {
	struct httpsqs *h = (struct httpsqs*)h;
	struct httpsqs_db *db = httpsqs_get_db(h);
	db->sync(h);
	//ydb_sync(httpsqs_db_tcbdb);
}
*/


static int
httpsqs_start( struct httpsqs* h ) {
	const char *listen_addr = httpsqs_get_option(h, "listen_addr", "0.0.0.0");
	int listen_port = atoi(httpsqs_get_option(h, "listen_port", "1218"));
	int timeout = atoi(httpsqs_get_option(h, "libevent_timeout", "3"));

	h->httpd = evhttp_start(listen_addr, listen_port);
	if (h->httpd == NULL) {
		fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", listen_addr, listen_port);		
		return 0;
	}
	evhttp_set_timeout(h->httpd, timeout);
	evhttp_set_gencb(h->httpd, httpsqs_handler, h);

    event_dispatch();
    evhttp_free(h->httpd);
	return 1;
}

int
main(int argc, char **argv)
{
	int c;

	//const char *httpsqs_settings_listen = "0.0.0.0";
	//int httpsqs_settings_port = 1218;
	//char *httpsqs_settings_datapath = NULL;
	//httpsqs_settings_pidfile = "/tmp/httpsqs.pid";
	
	event_init();		
	struct httpsqs* h = httpsqs_init();

    /* process arguments */
    while ((c = getopt(argc, argv, "l:p:x:t:s:c:m:i:dh")) != -1) {
        switch (c) {
        case 'l':
			httpsqs_set_option(h, "http_listen_addr", optarg);
            //httpsqs_settings_listen = strdup(optarg);
            break;
        case 'p':
			httpsqs_set_option(h, "http_listen_port", optarg);
            //httpsqs_settings_port = atoi(optarg);
            break;
        case 'x':
			httpsqs_set_option(h, "data_path", optarg);
            //httpsqs_settings_datapath = strdup(optarg);
			if (access(optarg, W_OK) != 0) {
				if (access(optarg, R_OK) == 0) {
					chmod(optarg, S_IWOTH);
				} else {
					create_multilayer_dir(optarg);
				}
	
				if (access(optarg, W_OK) != 0) {
					fprintf(stderr, "httpsqs database directory not writable\n");
				}
			}
            break;
		case 's':
			httpsqs_set_option(h, "ydb_min_log_size", optarg);
			//httpsqs_ydb_min_log_size = atoi(optarg);
			//assert( httpsqs_ydb_min_log_size >= 1 );
			break;
		case 'o':
			httpsqs_set_option(h, "ydb_overcommit", optarg);
			//httpsqs_ydb_overcommit = atoi(optarg);
			//assert( httpsqs_ydb_overcommit >= 1);
			break;
        case 't':
			httpsqs_set_option(h, "libevent_timeout", optarg);
            //httpsqs_settings_timeout = atoi(optarg);
            break;			
        case 'i':
			httpsqs_set_option(h, "pid_file", optarg);
            //httpsqs_settings_pidfile = strdup(optarg);
            break;			
        case 'd':
			httpsqs_set_option(h, "daemon", "false");
            //httpsqs_settings_daemon = true;
            break;
		case 'h':
        default:
            show_help();
            return 1;
        }
    }
	
	if (httpsqs_get_option(h, "data_path", NULL) == NULL) {
		show_help();
		fprintf(stderr, "Attention: Please use the indispensable argument: -x <path>\n\n");		
		exit(1);
	}

	struct httpsqs_db *db = db_init(h);
	db->open(h);
	
	if( httpsqs_get_option_bool(h, "daemon", 0) ){
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
	fp_pidfile = fopen(httpsqs_get_option(h, "pid_file", "/tmp/httpsqs.pid"), "w");
	fprintf(fp_pidfile, "%d\n", getpid());
	fclose(fp_pidfile);
	
	signal(SIGPIPE, SIG_IGN);
	
	struct event kill_sig;
	signal_set(&kill_sig, SIGINT, kill_signal, h);	
	signal_set(&kill_sig, SIGKILL, kill_signal, h);	
	signal_set(&kill_sig, SIGQUIT, kill_signal, h);	
	signal_set(&kill_sig, SIGTERM, kill_signal, h);	
	signal_set(&kill_sig, SIGHUP, kill_signal, h);	
	signal_add(&kill_sig, NULL);
	
	//struct event sync_sig;
	//signal(SIGALRM, sync_signal);
	
	httpsqs_start(h);
	return 0;
}
