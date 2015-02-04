
#ifndef _HIREDIS_CONTEXT_POOL_H_
#define _HIREDIS_CONTEXT_POOL_H_

#include <pthread.h>
#include "hiredis.h"
//#include "o2i2_types.h"
#include "hiredis_conn_pool.h"

/* connect redis server
*/
bool conn(RedisConnCB* cb);

/* pool constructor
*/
RedisConnCBPool* construct_pool(int size, const char* host, int port, int timeout, int retry_times,
	void (* logger)(REDIS_LOG_LEVEL level, const char * format, ...));

/* pool destructor
*/
bool destruct_pool(RedisConnCBPool* pool);

/* debug
*/
void log_pool(RedisConnCBPool* pool);

/* get a control block from pool
*/
RedisConnCB* pop_cb(RedisConnCBPool* pool);

/* return a control block to pool
*/
bool push_cb(RedisConnCBPool* pool, RedisConnCB* cb);

#endif //_HIREDIS_CONTEXT_POOL_H_
