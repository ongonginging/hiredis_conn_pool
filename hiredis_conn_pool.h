
#ifndef _HIREDIS_CONN_POOL_H_
#define _HIREDIS_CONN_POOL_H_

#include <pthread.h>
#include "hiredis.h"
//#include "o2i2_types.h"

typedef enum REDIS_LOG_LEVEL{
	REDIS_LOG_LEVEL_DEBUG = 0,
	REDIS_LOG_LEVEL_INFO = 1,
	REDIS_LOG_LEVEL_WARNING = 2,
	REDIS_LOG_LEVEL_ERROR = 3,
}REDIS_LOG_LEVEL;

typedef enum REDIS_RESULT{
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
	void (* logger)(REDIS_LOG_LEVEL level, const char * format, ...);
}RedisConnCBPool;

REDIS_RESULT initialize_redis_pool(RedisConnCBPool** pool, 
	int size, const char* host, int port, int timeout, int retry_times, 
	void (* logger)(REDIS_LOG_LEVEL level, const char * format, ...));
REDIS_RESULT shutdown_redis_pool(RedisConnCBPool* pool);
REDIS_RESULT do_redis_command(RedisConnCBPool* pool, redisReply** reply, const char* cmd, ...);

#endif //_HIREDIS_CONN_POOL_H_
