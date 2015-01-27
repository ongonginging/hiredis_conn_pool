
#ifndef _O2I2_REDIS_CONTEXT_POOL_H_
#define _O2I2_REDIS_CONTEXT_POOL_H_

#include <pthread.h>
#include "hiredis.h"
#include "o2i2_types.h"

/* redis connection control block
*/
typedef struct RedisConnCB{
	int pre;
	int next;
	int index;
	redisContext* context;
	RedisConnCBPool* pool;
}RedisConnCB;

/* pool of redis connection control blocks
*/
typedef struct RedisConnCBPool{
	int size_total;
	int idle_size;
	int idle_front;
	int busy_size;
	int busy_front;
	pthread_mutex_t mutex;
	int retry_times;
	redisContext* cbs;
}RedisConnCBPool;

/* pool constructor
*/
RedisConnCBPool* construct_pool(int size, char* host, int port, int timeout, int retry_times);

/* pool destructor
*/
bool destruct_pool(RedisConnCBPool* pool);

/* get a control block from pool
*/
RedisConnCB* pop_cb(RedisConnCBPool* pool);

/* return a control block to pool
*/
bool push_cb(RedisConnCBPool* pool, RedisConnCB* cb);

#endif //_O2I2_REDIS_CONTEXT_POOL_H_
