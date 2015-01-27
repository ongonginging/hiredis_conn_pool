
#include <pthread.h>
#include "hiredis.h"
#include "o2i2_types.h"
#include "o2i2_redis_context_pool.h"

RedisConnCBPool* construct_pool(int size, char* host, int port, int timeout, int retry_times){

	int size = redis_conn_poolsize;
	if (size <= 0) {
		//TODO: LOG
		return null;
	}

	RedisConnCBPool* pool = (RedisConnCBPool*)malloc(sizeof(RedisConnCBPool));
	if (!pool) {
		//TODO: LOG
		return null;
	}
	memset((void*)pool, 0, sizeof(RedisConnCBPool));

	pool->size_total = size;
	pool->size_idle = size;
	pool->size_busy = 0;

	pool->idle_front = 0;
	pool->idle_tail = size-1;

	pool->busy_front = -1;
	pool->busy_tail = -1;

	pthread_mutex_init(&pool->mutex, null);

	pool->cbs = (RedisConnCB*)malloc(sizeof(RedisConnCB)*size);
	if (!pool->cbs) {
		//TODO: LOG
		free(pool);
		return null;
	}
	memset((void*)pool->cbs, 0, sizeof(RedisConnCB)*size);
	//int i = 0;
	RedisConnCB* cb = null;
	for(int i=0; i<size; i++){
		cb = (RedisConnCB*)pool->cbs + i;
		cb->pre = (i-1+size)%size;
		cb->next = (i+1)%size;
		cb->index = i;
		//TODO: retry here when connecting failed.
		cb->context = redisConnectWithTimeout(host, port, timeout);
	}

	return pool;
}

bool destruct_pool(RedisConnCBPool* pool){
	//TODO:unlock when locked
	//int i = 0;
	RedisConnCB* cb = null;
	if (pool->cbs){
		for(int i=0; i<pool->size; i++){
			cb = (RedisConnCB*)pool->cbs + i;
			if(cb->context) {
				redisFree(cb->context);
			}
		}
		free(pool->cbs);
	}
	free(pool);
}

RedisConnCB* pop_cb(RedisConnCBPool* pool){
	pthread_mutex_lock(&pool->mutex);
	if(pool->idle_front == -1){
		//TODO:LOG, no idle connection
		return null;
	}else{
		if(pool->idle_front == pool->idle_tail){
			return (RedisConnCB*)pool->cbs + pool->idle->front;
			pool->idle_front = -1;
			pool->idle_tail = -1;
		}else{
			return (RedisConnCB*)pool->cbs + pool->idle->front;
			pool->idle_front = -1;
			pool->idle_tail = -1;
		}
	}
	pthread_mutex_unlock(&pool->mutex);
}

bool push_cb(RedisConnCBPool* pool, RedisConnCB* cb){
	pthread_mutex_lock(&pool->mutex);
	pthread_mutex_unlock(&pool->mutex);
}