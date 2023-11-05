#!/bin/sh
gcc -O0 -fno-stack-protector rop-server.c -o 64-rop-server
gcc -m32 -O0 -fno-stack-protector rop-server.c -o 32-rop-server