
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "hiredis.h"
//#include "o2i2_types.h"
#include "o2i2_redis_conn.h"
#include "o2i2_redis_context_pool.h"

bool conn(RedisConnCB* cb){

	bool rv = false;
	RedisConnCBPool* pool = cb->pool;
	redisContext* context = cb->context;
	int retry = pool->retry_times;

	if (NULL != context) {
		redisFree(context);
		context = NULL;
	}

	struct timeval timeout = {0, pool->timeout};
	while(0 != retry--){
		context = redisConnectWithTimeout(pool->host, pool->port, timeout);
		if(NULL == context){
			pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Connecting timeout.", __FILE__, __LINE__, __func__);
			continue;
		}else{
			if (0 != context->err) {
				pool->logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] %s", __FILE__, __LINE__, __func__, context->err);
				redisFree(context);
				context = NULL;
				continue;
			}
			rv = true;
			break;
		}
	}
    cb->context = context;
	return rv;
}

RedisConnCBPool* construct_pool(int size, char* host, int port, int timeout, int retry_times, 
	void (* logger)(REDIS_LOG_LEVEL level, const char * format, ...)){

	if (size <= 0) {
		logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] pool size <= 0.", __FILE__, __LINE__, __func__);
		return NULL;
	}
	RedisConnCBPool* pool = (RedisConnCBPool*)malloc(sizeof(RedisConnCBPool));
	if (!pool) {
		logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Failed to allocate memory for pool.", __FILE__, __LINE__, __func__);
		return NULL;
	}
	memset((void*)pool, 0, sizeof(RedisConnCBPool));

	pool->size_total = size;
	pool->idle_size = size;
	pool->busy_size = 0;
	pool->idle_front = 0;
	pool->busy_front = -1;
	pool->retry_times = retry_times;
	pool->port = port;
	pool->logger = logger;
	if (NULL != host || (strlen(host)<7 && strlen(host)>16)) {
		memcpy(pool->host, host, strlen(host));
	}else{
		free(pool);
		return NULL;
	}
	pthread_mutex_init(&pool->mutex, NULL);
	pool->cbs = (RedisConnCB*)malloc(sizeof(RedisConnCB)*size);
	if (!pool->cbs) {
		logger(REDIS_LOG_LEVEL_ERROR, "[hiredis_conn_pool|%s|%d|%s] Failed to allocate memory for pool->cbs.", __FILE__, __LINE__, __func__);
		free(pool);
		return NULL;
	}
	memset((void*)pool->cbs, 0, sizeof(RedisConnCB)*size);

	int i;
	RedisConnCB* cb = NULL;
	for(i=0; i<size; i++){
		cb = pool->cbs + i;
		cb->pre = (i-1+size)%size;
		cb->next = (i+1)%size;
		cb->index = i;
		cb->pool = pool;
		cb->context = NULL;
		conn(cb);
	}

	return pool;
}

bool destruct_pool(RedisConnCBPool* pool){
	int i = 0;
	RedisConnCB* cb = NULL;
	if (pool->cbs){
		for(i=0; i<pool->size_total; i++){
			cb = pool->cbs + i;
			if(NULL != cb->context) {
				redisFree(cb->context);
			}
		}
		free(pool->cbs);
	}
	free(pool);
}

RedisConnCB* pop_cb(RedisConnCBPool* pool){

	RedisConnCB* rv = NULL;
	pthread_mutex_lock(&pool->mutex);

	if (-1 == pool->idle_front){
		pool->logger(REDIS_LOG_LEVEL_WARNING, "[hiredis_conn_pool|%s|%d|%s] No Idle connection.", __FILE__, __LINE__, __func__);
		goto _return;
	}else{
		rv = pool->cbs + pool->idle_front;
		if(rv->pre == rv->next == rv->index){
			pool->idle_front = -1;
		}else{
			(pool->cbs + rv->pre)->next = rv->next;
			(pool->cbs + rv->next)->pre = rv->pre;
			pool->idle_front = rv->next;
		}
	}
	if (-1 == pool->busy_front){
		rv->pre = rv->index;
		rv->next = rv->index;
	}else{
		RedisConnCB* front = pool->cbs + pool->busy_front;
		rv->pre = front->pre;
		rv->next = front->index;
		(pool->cbs + rv->pre)->next = rv->index;
		(pool->cbs + rv->next)->pre = rv->index;
	}

	pool->busy_front = rv->index;
	pool->idle_size--;
	pool->busy_size++;

_return:
	pthread_mutex_unlock(&pool->mutex);
	return rv;
}

bool push_cb(RedisConnCBPool* pool, RedisConnCB* cb){

	bool rv = true;
	pthread_mutex_lock(&pool->mutex);

	if (-1 == pool->busy_front){
		pool->logger(REDIS_LOG_LEVEL_WARNING, "[hiredis_conn_pool|%s|%d|%s] Impossible.", __FILE__, __LINE__, __func__);
		rv = false;
		goto _return;
	}else{
		if(cb->pre == cb->next == cb->index){//one element in list
			pool->busy_front = -1;
		}else{
			(pool->cbs + cb->pre)->next = cb->next;
			(pool->cbs + cb->next)->pre = cb->pre;
			pool->busy_front = cb->next;
		}
	}
	if (-1 == pool->idle_front){
		cb->pre = cb->index;
		cb->next = cb->index;
	}else{
		RedisConnCB* front = pool->cbs + pool->idle_front;
		cb->pre = front->pre;
		cb->next = front->index;
		(pool->cbs + cb->pre)->next = cb->index;
		(pool->cbs + cb->next)->pre = cb->index;
	}
	pool->idle_front = cb->index;
	pool->idle_size++;
	pool->busy_size--;

_return:
	pthread_mutex_unlock(&pool->mutex);
	return rv;
}