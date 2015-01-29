
#ifndef _O2I2_REDIS_CONN_H_
#define _O2I2_REDIS_CONN_H_

#include "o2i2_types.h"
#include "o2i2_redis_context_pool.h"

typedef enum {
	REDIS_RESULT_SUCCESS = 0,
	REDIS_RESULT_ERROR_UNKNOWN = 1,
	REDIS_RESULT_ERROR_NULL_POINTER = 2,
	REDIS_RESULT_CONSTRUCT_POOL_FAILED = 3,
	REDIS_RESULT_DESTRUCT_POOL_FAILED = 4,
	REDIS_RESULT_ERROR_POP_CONN_FAILED = 5,
	REDIS_RESULT_ERROR_PUSH_CONN_FAILED = 6,
	REDIS_RESULT_CONNECT_SERVER_FAILED = 7,
}REDIS_RESULT;

/* redis connection control block
*/
typedef struct RedisConnCB{
	int pre;
	int next;
	int index;
	redisContext* context;
	struct RedisConnCBPool* pool;
}RedisConnCB;

/* pool of redis connection control blocks
*/
typedef struct RedisConnCBPool{
	int size_total;
	int idle_size;
	int idle_front;
	int busy_size;
	int busy_front;
	int retry_times;
	int timeout;
	int port;
	char host[16];
	pthread_mutex_t mutex;
	struct RedisConnCB* cbs;
}RedisConnCBPool;


REDIS_RESULT init_redis_pool(RedisConnCBPool** pool, int size, char* host, int port, int timeout, int retry_times);
REDIS_RESULT deinit_redis_pool(RedisConnCBPool* pool);
REDIS_RESULT do_redis_command(RedisConnCBPool* pool, redisReply** reply, char* cmd, ...);

#endif //_O2I2_REDIS_CONN_H_
