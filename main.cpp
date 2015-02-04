
#include <pthread.h> 
#include <stdio.h> 

//#include "o2i2_types.h"
#include "hiredis.h"
#include "o2i2_redis_conn.h"
#include "o2i2_redis_context_pool.h"

void logger(REDIS_LOG_LEVEL level, const char* cmd, ...){
    va_list args;
    va_start(args, cmd);
    vprintf(cmd, args); 
    va_end(args);
}

int main(int argc, char const *argv[])
{
    /* code */
    RedisConnCBPool* pool; 
    REDIS_RESULT rv_redis = REDIS_RESULT_SUCCESS;
    rv_redis = initialize_redis_pool(&pool, 5, "127.0.0.1", 6379, 10000, 3, logger);
    if (REDIS_RESULT_SUCCESS != rv_redis){
        logger(REDIS_LOG_LEVEL_DEBUG, "Failed to create redis connetion pool."); 
    }

    log_pool(pool);
    redisReply* reply;
    rv_redis = do_redis_command(pool, &reply, "set helloworld %d", 65535);
    if (REDIS_RESULT_SUCCESS != rv_redis){
        logger(REDIS_LOG_LEVEL_DEBUG, "Failed to set helloworld to 1.\n"); 
    }else{
        freeReplyObject(reply);
        reply = NULL;
    }
    
    rv_redis = do_redis_command(pool, &reply, "get helloworld");
    if (REDIS_RESULT_SUCCESS != rv_redis){
        logger(REDIS_LOG_LEVEL_DEBUG, "Failed to get value of helloworld.\n"); 
    }else{
        logger(REDIS_LOG_LEVEL_DEBUG, "reply->type=%d.\n", reply->type);
        if (reply->type == REDIS_REPLY_STRING){
            logger(REDIS_LOG_LEVEL_DEBUG, "reply->str=%s.\n", reply->str);
        }
        freeReplyObject(reply);
        reply = NULL;
    }
    
    /*
    RedisConnCB* c1 = pop_cb(pool);
    log_pool(pool);
    RedisConnCB* c2 = pop_cb(pool);   
    log_pool(pool);
    RedisConnCB* c3 = pop_cb(pool);
    log_pool(pool);
    push_cb(pool, c1);    
    log_pool(pool);
    push_cb(pool, c2);
    log_pool(pool);
    push_cb(pool, c3);
    log_pool(pool);
    */

    rv_redis = shutdown_redis_pool(pool);
    if (REDIS_RESULT_SUCCESS != rv_redis){
       logger(REDIS_LOG_LEVEL_DEBUG, "Failed to create redis connetion pool."); 
    }

    pool = NULL;

    return 0;
}
