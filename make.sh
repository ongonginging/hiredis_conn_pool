#!/bin/sh
gcc -g -I. -I/usr/local/include/hiredis  -lpthread -lhiredis -L/usr/local/lib main.c o2i2_types.h o2i2_redis_context_pool.h o2i2_redis_context_pool.c o2i2_redis_conn.h o2i2_redis_conn.c
