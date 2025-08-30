#!/bin/sh

cc -O2 -I../include example.c ../src/enc.c ../src/dec.c -o fox
