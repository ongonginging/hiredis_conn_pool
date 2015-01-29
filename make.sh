#!/bin/sh
g++ -g -Wpointer-arith -I. -I/usr/include/hiredis  -lpthread -lhiredis -L/usr/lib main.cpp o2i2_types.h o2i2_redis_context_pool.h o2i2_redis_context_pool.cpp o2i2_redis_conn.h o2i2_redis_conn.cpp
