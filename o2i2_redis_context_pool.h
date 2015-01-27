
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
}RedisConnCB;

/* pool of redis connection control blocks
*/
typedef struct RedisConnCBPool{
	int size_total;
	int size_idle;
	int size_busy;
	int idle_front;
	int idle_tail;
	int busy_front;
	int busy_tail;
	pthread_mutex_t mutex;
	redisContext* cbs;
}RedisConnCBPool;

/* pool constructor
*/
RedisConnCBPool* construct_pool(int size);

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
