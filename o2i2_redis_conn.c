
#include <stdarg.h>
#include "hiredis.h"
#include "o2i2_types.h"
#include "o2i2_redis_conn.h"
#include "o2i2_redis_conn_manager.h"
#include "o2i2_redis_context_pool.h"

REDIS_RESULT init_redis_pool(RedisConnCBPool** pool, int size, char* host, int port, int timeout, int retry_times){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	*pool = construct_pool(size, host, port, timeout, retry_times);
	if (null == pool){
		return REDIS_RESULT_CONSTRUCT_POOL_FAILED;
	}
	return rv;
}

REDIS_RESULT deinit_redis_pool(RedisConnCBPool* pool){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	bool destruct_rv = destruct_pool(pool);
	if (false == destruct_rv){
		return REDIS_RESULT_DESTRUCT_POOL_FAILED;
	}
	return rv;
}

REDIS_RESULT do_redis_command(RedisConnCBPool* pool, redisReply** reply, char* cmd, ...){
	REDIS_RESULT rv = REDIS_RESULT_SUCCESS;
	RedisConnCB* cb = pop_cb(pool);
	if (null == cb){
		rv = REDIS_RESULT_ERROR_POP_CONN_FAILED;
		return rv;
	}
	if (null == cb->context){
		bool conn_rv = connect(cb);
		if(false == conn_rv){
			//TODO: LOG ERROR, ""
			rv = REDIS_RESULT_CONNECT_SERVER_FAILED;
			goto _return;
		}
	}
	va_list args;
	va_start(args, cmd);
	*reply = (redisReply*)redisCommand(cb->context, args);
	if (null == reply){
		//TODO: LOG ERROR, ""
		bool conn_rv = connect(cb);
		if(false == bool){
			//TODO: LOG ERROR, ""
			rv = REDIS_RESULT_CONNECT_SERVER_FAILED;
			goto _return;
		}
		*reply = (redisReply*)redisCommand(cb->context, args);
	}
	va_end(args);

_return:
	bool push_rv = push_cb(pool, cb);
	if (false == push_rv) {
		//TODO: LOG ERROR, ""
		return REDIS_RESULT_ERROR_PUSH_CONN_FAILED;
	}
	return rv;
}
