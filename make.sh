#!/bin/sh
gcc -g -I. -I/usr/local/include/hiredis -L/usr/local/lib -lpthread -lhiredis main.c o2i2_types.h o2i2_redis_context_pool.h o2i2_redis_context_pool.c o2i2_redis_conn.h o2i2_redis_conn.c
