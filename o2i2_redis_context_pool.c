
#include <pthread.h>
#include "hiredis.h"
#include "o2i2_types.h"
#include "o2i2_redis_context_pool.h"

bool connect(RedisConnCB* cb){

	bool rv = false;
	RedisConnCBPool* pool = cb->pool;
	redisContext* context = cb->context;
	int retry = pool->retry_times;

	if (null != context) {
		redisFree(context);
		context = null;
	}
	while(0 != retry--){
		context = redisConnectWithTimeout(pool->host, pool->port, pool->timeout);
		if(null == context){
			//TODO: LOG ERROR, "Connection error: \n"
			continue;
		}else{
			if (context->err != null) {
				//TODO: LOG ERROR, "Connection error: %s\n",cb->context->err
				redisFree(context);
				context = null;
				continue;
			}
			rv = true;
			break;
		}
	}
    cb->context = context;
	return rv;
}

RedisConnCBPool* construct_pool(int size, char* host, int port, int timeout, int retry_times){

	int size = size;
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
	pool->idle_size = size;
	pool->busy_size = 0;
	pool->idle_front = 0;
	pool->busy_front = -1;
	pool->retry_times = retry_times;
	pool->port = port;
	if (null != host || (strlen(host)<7 && strlen(host)>16)) {
		memcpy(pool->host, host, strlen(host));
	}else{
		free(pool);
		return null;
	}
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
		cb = pool->cbs + i;
		cb->pre = (i-1+size)%size;
		cb->next = (i+1)%size;
		cb->index = i;
		cb->pool = pool;
		cb->context = null;
		bool conn_ret = connect(cb);
	}

	return pool;
}

bool destruct_pool(RedisConnCBPool* pool){
	//TODO:unlock
	//int i = 0;
	RedisConnCB* cb = null;
	if (pool->cbs){
		for(int i=0; i<pool->size; i++){
			cb = pool->cbs + i;
			if(null != cb->context) {
				redisFree(cb->context);
			}
		}
		free(pool->cbs);
	}
	free(pool);
}

RedisConnCB* pop_cb(RedisConnCBPool* pool){

	RedisConnCB* rv = null;
	pthread_mutex_lock(&pool->mutex);

	if(pool->idle_front == -1){
		//TODO:LOG ERROR, no idle connection
		goto _return;
	}else{
		rv = pool->cbs[pool->idle_front];
		if(rv->pre == rv->next == rv->index){
			pool->idle_front = -1;
		}else{
			pool->cbs[rv->pre]->next = rv->next;
			pool->cbs[rv->next]->pre = rv->pre;
			pool->idle_front = rv->next;
		}
	}
	if(pool->busy_front == -1){
		rv->pre = rv->index;
		rv->next = rv->index;
	}else{
		RedisConnCB* front = pool->cbs[pool->busy_front];
		rv->pre = front->pre;
		rv->next = front->index;
		pool->cbs[rv->pre]->next = rv->index;
		pool->cbs[rv->next]->pre = rv->index;
	}	RedisConnCB* cb = null;

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

	if(pool->busy_front == -1){
		//TODO:LOG ERROR, impossible
		rv = false;
		goto _return;
	}else if(cb->pre == cb->next == cb->index){//only one cb in list
			pool->busy_front = -1;
		}else{
			pool->cbs[cb->pre]->next = cb->next;
			pool->cbs[cb->next]->pre = cb->pre;
			pool->busy_front = cb->next;
		}
	}
	if(pool->idle_front == -1){
		cb->pre = cb->index;
		cb->next = cb->index;
	}else{
		RedisConnCB* front = pool->cbs[pool->idle_front];
		cb->pre = front->pre;
		cb->next = front->index;
		pool->cbs[cb->pre]->next = cb->index;
		pool->cbs[cb->next]->pre = cb->index;
	}
	pool->idle_front = cb->index;
	pool->idle_size++;
	pool->busy_size--;

_return:
	pthread_mutex_unlock(&pool->mutex);
	return rv;
}