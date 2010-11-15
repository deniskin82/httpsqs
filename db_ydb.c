#define _GNU_SOURCE

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "httpsqs.h"
#include "rbtree.h"
#include "ydb.h"

static int
dbint_del(struct httpsqs *h, const char *key, size_t key_sz) {
	YDB ydb_h = httpsqs_get_db_internal(h);
	return ydb_del(ydb_h, (char *)key, key_sz);
}

static int
dbint_add(struct httpsqs *h, const char *key, size_t key_sz, char *value, size_t value_sz) {
	YDB ydb_h = httpsqs_get_db_internal(h);
	return ydb_add(ydb_h, (char *)key, key_sz, value, value_sz);
}

static int
dbint_get(struct httpsqs *h, const char *key, size_t key_sz, char **value, size_t *value_sz) {
	YDB ydb_h = httpsqs_get_db_internal(h);
	return ydb_geta(ydb_h, (char *)key, key_sz, value, value_sz);
}

static void
dbint_close(struct httpsqs *h) {
	YDB ydb_h = httpsqs_get_db_internal(h);
	ydb_close(ydb_h);
}

static int
dbint_open(struct httpsqs *h) {
	const char *data_path = httpsqs_get_option(h, "data_path", NULL);
	size_t overcommit = httpsqs_get_option_int(h, "ydb_overcommit", 3);
	size_t min_log_size = httpsqs_get_option_int(h, "ydb_min_log_size", 5 * 1024 * 1024);
	if( data_path == NULL ) data_path = "./";
	
	assert( data_path != NULL );
	assert( min_log_size > 0 );
	assert( overcommit > 0 );
	
	h->db->internal_handle = ydb_open((char *)data_path, overcommit, min_log_size, YDB_CREAT);
	return h->db->internal_handle != NULL;
}

static void
dbint_sync(struct httpsqs *h) {
	YDB ydb_h = httpsqs_get_db_internal(h);
	ydb_sync(ydb_h);
}

struct httpsqs_db *
db_init(struct httpsqs *h) {
	struct httpsqs_db *db;
	
	h->db = (struct httpsqs_db *)malloc(sizeof(struct httpsqs_db));
	assert( h->db != NULL );
	h->db = memset(h->db, 0, sizeof(struct httpsqs_db *));		
	db = h->db;
	
	db->del = dbint_del;
	db->add = dbint_add;
	db->get = dbint_get;
	db->close = dbint_close;
	db->open = dbint_open;
	db->sync = dbint_sync;
	
	return db;
}