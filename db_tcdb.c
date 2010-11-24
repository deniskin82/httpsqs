#define _GNU_SOURCE

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "httpsqs.h"
#include "rbtree.h"

#include <tcutil.h>
#include <tchdb.h>

static int
dbint_del(struct httpsqs *h, const char *key, size_t key_sz) {
	TCHDB *db = httpsqs_get_db_internal(h);
	return tchdbout(db, key, key_sz);
}

static int
dbint_add(struct httpsqs *h, const char *key, size_t key_sz, char *value, size_t value_sz) {
	TCHDB *db = httpsqs_get_db_internal(h);
	return tchdbput(db, key, key_sz, value, value_sz);
}

static int
dbint_get(struct httpsqs *h, const char *key, size_t key_sz, char **value, size_t *value_sz) {
	TCHDB *db = httpsqs_get_db_internal(h);
	ssize_t x;
	*value = tchdbget(db, key, key_sz, &x);
	if( *value ) *value_sz = x;
	return *value != NULL;
}

static void
dbint_close(struct httpsqs *h) {
	TCHDB *db = (TCHDB *)httpsqs_get_db_internal(h);
	tchdbclose(db);
}

static int
dbint_open(struct httpsqs *h) {
	const char *data_path = httpsqs_get_option(h, "data_path", NULL);	
	if( data_path == NULL ) data_path = "./";
	
	// Tuning flags
	int64_t bnum = httpsqs_get_option_int(h, "tchdb_bnum", 131071);
	int8_t apow = httpsqs_get_option_int(h, "tchdb_apow", 4);
	int8_t fpow = httpsqs_get_option_int(h, "tchdb_fpow", 10);
	uint8_t opts = 0;
	if( httpsqs_get_option_int(h, "tchdb_HDBTLARGE", 0) ) opts |= HDBTLARGE;
	if( httpsqs_get_option_int(h, "tchdb_HDBTDEFLATE", 0) ) opts |= HDBTDEFLATE;
	if( httpsqs_get_option_int(h, "tchdb_HDBTBZIP", 0) ) opts |= HDBTBZIP;
	
	//Misc flags
	int32_t rcnum = httpsqs_get_option_int(h, "tchdb_rcnum", 20);
	int64_t xmsiz = httpsqs_get_option_int(h, "tchdb_xmsiz", 67108864);
	int32_t dfunit = httpsqs_get_option_int(h, "tchdb_dfunit", 0);
	int mode_nolck = httpsqs_get_option_int(h, "tchdb_nolck", 0);
	int mode_lcknb = httpsqs_get_option_int(h, "tchdb_lcknb", 0);
	
	int omode = HDBOWRITER | HDBOCREAT;
	if( mode_nolck ) omode |= HDBONOLCK;	
	if( mode_lcknb ) omode |= HDBOLCKNB;
		
	assert( data_path != NULL );
	
	TCHDB *db = tchdbnew();
	if( ! tchdbtune(db, bnum, apow, fpow, opts) ) return -1;
	if( ! tchdbsetcache(db, rcnum) ) return -2;
	if( ! tchdbsetxmsiz(db, xmsiz) ) return -3;
	if( ! tchdbsetdfunit(db, dfunit) ) return -4;
	
	if( ! tchdbopen(db, data_path, omode) ) return -6;
	
	h->db->internal_handle = (void*)db;
	return 1;
}

static void
dbint_sync(struct httpsqs *h) {
	TCHDB *db = httpsqs_get_db_internal(h);
	tchdbsync(db);
}

void
db_show_opts(void) {
	//     "   >------------------< X...n"
	printf("   tchdb_bnum           specifies the number of elements of the bucket array. (default 131071)\n");
	printf("   tchdb_apow           size of record alignment by power of 2 (default 4)\n");
	printf("   tchdb_fpow           maximum number of elements of the free block pool by power of 2 (default 10, e.g. 2^10)\n");
	printf("   tchdb_HDBTLARGE      the size of the database can be larger than 2GB by using 64-bit bucket array\n");
	printf("   tchdb_HDBTDEFLATE    each record is compressed with Deflate encoding\n");
	printf("   tchdb_HDBTBZIP       each record is compressed with BZIP2 encoding\n");
	printf("   tchdb_rcnum          maximum number of records to be cached (default 20)\n");
	printf("   tchdb_xmsiz          size of the extra mapped memory. (default 67108864)\n");
	printf("   tchdb_dfunit         unit step number of auto defragmentation of a hash database object. (default 0)\n");
	printf("   tchdb_nolck          opens the database file without file locking\n");
	printf("   tchdb_lcknb          locking is performed without blocking\n");
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