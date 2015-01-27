
#include <stdarg.h>
#include "hiredis.h"
#include "o2i2_types.h"
#include "o2i2_conn.h"

RESULT redis_command(RedisConnCBPool* pool, redisReply** reply, char* cmd, ...){
	va_list args;
	RESULT rv = RESULT_SUCCESS;
	redisContext * redis_context = NULL;
	va_start(args, cmd);
	*reply = (redisReply *)redisCommand(redis_context, args);
	va_end(args);
	return rv;
}
