test_assign1: test_assign1_1.c storage_mgr.c storage_mgr.h dberror.h
	gcc -c -o storage_mgr.o storage_mgr.c
	gcc -c -o dberror.o dberror.c
	gcc -c -o test_assign1_1.o test_assign1_1.c
	gcc -o test_assign1 test_assign1_1.o storage_mgr.o dberror.o -I.

clean:
	rm test_pagefile.bin
