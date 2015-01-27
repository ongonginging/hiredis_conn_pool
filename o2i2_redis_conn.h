
#ifndef _O2I2_REDIS_CONN_H_
#define _O2I2_REDIS_CONN_H_

#include "o2i2_types.h"

typedef enum {
	RESULT_SUCCESS = 0,
	RESULT_ERROR_NO_CONNECTION = 1,
}RESULT;

RESULT redis_command(RedisConnCBPool pool, redisReply** reply, char cmd, ...);

#endif //_O2I2_REDIS_CONN_H_
