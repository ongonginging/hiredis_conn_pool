#!/bin/sh
g++ -g -I. -I/usr/local/include/hiredis -L/usr/local/lib -lpthread -lhiredis main.cpp o2i2_types.h o2i2_redis_context_pool.h o2i2_redis_context_pool.cpp o2i2_redis_conn.h o2i2_redis_conn.cpp
