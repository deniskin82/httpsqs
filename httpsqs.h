#ifndef HTTPSQS_H_
#define HTTPSQS_H_

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include <stdint.h>


#include <err.h>
#include <event.h>
#include <evhttp.h>
#include "rbtree.h"

#define SQS_VERSION "1.3.2.panda"

//YDB httpsqs_db_tcbdb;
//const char *httpsqs_settings_pidfile;

struct sqs_queue_status {
	char name[11];
	size_t name_sz;
	uint64_t tail_id;
	uint64_t head_id;
	uint32_t maxqueue;
	uint32_t count;
	uint64_t count_bytes;
	
	/* Totals for the lifetime of the queue */
	uint64_t total_get;
	uint64_t total_put;
	uint64_t total_info;
	uint64_t total_get_bytes;
	uint64_t total_put_bytes;
	
	/* RB node */
	struct rb_node rbnode;
};

struct httpsqs;

struct httpsqs_db {
	void *internal_handle;
	int (*del)(struct httpsqs *h, const char *key, size_t key_sz);
	int (*add)(struct httpsqs *h, const char *key, size_t key_sz, char *value, size_t value_sz);
	int (*get)(struct httpsqs *h, const char *key, size_t key_sz, char **value, size_t *value_sz);
	void (*close)(struct httpsqs *h);
	int (*open)(struct httpsqs *h);
	// Optional methods
	void (*sync)(struct httpsqs *h);
//	void (*prefetch)(struct httpsqs *h, const char **keys, size_t *key_szs, size_t item_count);
};

struct httpsqs {
	struct evkeyvalq params;
	struct httpsqs_db *db;
	struct rb_root queues;
	struct evhttp *httpd;
};

struct httpsqs_db *httpsqs_get_db(struct httpsqs *h);
void* httpsqs_get_db_internal(struct httpsqs *h);
const char *httpsqs_get_option(struct httpsqs* h, const char *name, const char *default_value);
int httpsqs_get_option_bool(struct httpsqs* h, const char *name, int default_value);
int httpsqs_get_option_int(struct httpsqs *h, const char *name, int default_value);
int httpsqs_set_option(struct httpsqs* h, const char *name, const char *value);

// Defined in whichever DB handler you're using
struct httpsqs_db *db_init(struct httpsqs *h);

#endif
