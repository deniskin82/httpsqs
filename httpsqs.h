#ifndef HTTPSQS_H_
#define HTTPSQS_H_

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
	void (*sync)(struct httpsqs *h);
	void (*close)(struct httpsqs *h);
	void (*open)(struct httpsqs *h, const char *path, struct evkeyvalq *args);
	void (*prefetch)(struct httpsqs *h, const char **keys, size_t *key_szs, size_t item_count);
};

struct httpsqs {
	struct evkeyvalq params;
	struct httpsqs_db *db;
	struct rb_root queues;
	struct evhttp *httpd;
};

struct httpsqs_db *httpsqs_get_db(struct httpsqs *h);

#endif
