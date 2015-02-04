
#include <stdarg.h>
#include "hiredis.h"
//#include "o2i2_types.h"
#include "hiredis_conn_pool.h"
#include "hiredis_context_pool.h"

REDIS_RESULT initialize_redis_pool(RedisConnCBPool** pool, 
	int size, const char* host, int port, int timeout, int retry_times, 
	void (* logger)(REDIS_LOG_LEVEL level, const char * format, ...)){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	*pool = construct_pool(size, host, port, timeout, retry_times, logger);
	if (NULL == pool){
		logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do construct_pool failed.\n", __FILE__, __LINE__, __func__);
		return REDIS_RESULT_CONSTRUCT_POOL_FAILED;
	}
	return rv;
}

REDIS_RESULT shutdown_redis_pool(RedisConnCBPool* pool){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	bool destruct_rv = destruct_pool(pool);
	if (false == destruct_rv){
		pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do destruct_pool failed.\n", __FILE__, __LINE__, __func__);
		return REDIS_RESULT_DESTRUCT_POOL_FAILED;
	}
	return rv;
}

REDIS_RESULT do_redis_command(RedisConnCBPool* pool, redisReply** reply, const char* cmd, ...){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	RedisConnCB* cb = pop_cb(pool);
	if (NULL == cb){
		rv = REDIS_RESULT_ERROR_POP_CONN_FAILED;
		return rv;
	}
	if (NULL == cb->context){
		bool conn_rv = conn(cb);
		if(false == conn_rv){
			pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do conn failed.\n", __FILE__, __LINE__, __func__);
			rv = REDIS_RESULT_CONNECT_SERVER_FAILED;
			bool push_rv = false;
			push_rv = push_cb(pool, cb);
			if (false == push_rv) {
				pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do push_cb failed.\n", __FILE__, __LINE__, __func__);
				rv = REDIS_RESULT_ERROR_PUSH_CONN_FAILED;
			}
			return rv;
		}
	}
	va_list args;
	va_start(args, cmd);
	*reply = (redisReply*)redisvCommand(cb->context, cmd, args);
	if (NULL == *reply){
		pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do redis command failed.\n", __FILE__, __LINE__, __func__);
		bool conn_rv = conn(cb);
		if(false == conn_rv){
			pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do conn failed.\n", __FILE__, __LINE__, __func__);
			rv = REDIS_RESULT_CONNECT_SERVER_FAILED;
			bool push_rv = push_cb(pool, cb);
			if (false == push_rv) {
				pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do push_cb failed.\n", __FILE__, __LINE__, __func__);
				rv = REDIS_RESULT_ERROR_PUSH_CONN_FAILED;
			}
			return rv;
		}
		*reply = (redisReply*)redisvCommand(cb->context, cmd, args);
	}
	va_end(args);

	bool push_rv = false;
	push_rv = push_cb(pool, cb);
	if (false == push_rv) {
		pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Do push_cb failed.\n", __FILE__, __LINE__, __func__);
		rv = REDIS_RESULT_ERROR_PUSH_CONN_FAILED;
	}
	return rv;
}
