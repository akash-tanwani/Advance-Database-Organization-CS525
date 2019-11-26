all:test_assign4 test_expr 

test_assign4: test_assign4_1.o expr.o dberror.o btree_mgr.o rm_serializer.o storage_mgr.o buffer_list.o buffer_mgr.o buffer_mgr_stat.o record_scan.o record_mgr.o
	cc -o test_assign4 test_assign4_1.o expr.o dberror.o btree_mgr.o rm_serializer.o storage_mgr.o buffer_list.o buffer_mgr.o buffer_mgr_stat.o record_scan.o record_mgr.o

test_assign4_1.o: test_assign4_1.c dberror.h expr.h btree_mgr.h tables.h test_helper.h
	cc -c test_assign4_1.c

dberror.o: dberror.c dberror.h
	cc -c dberror.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	cc -c storage_mgr.c

buffer_list.o: buffer_list.c
	cc -c buffer_list.c

buffer_mgr.o: buffer_mgr.c buffer_mgr.h  storage_mgr.h dt.h buffer_list.h
	cc -c buffer_mgr.c

buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h
	cc -c buffer_mgr_stat.c

expr.o: expr.c dberror.h tables.h
	cc -c expr.c

record_scan.o: record_scan.c 
	cc -c record_scan.c

record_mgr.o: record_mgr.c record_mgr.h dberror.h expr.h tables.h record_scan.h
	cc -c record_mgr.c

btree_mgr.o: btree_mgr.c btree_mgr.h dberror.h expr.h tables.h
	cc -c btree_mgr.c

rm_serializer.o: rm_serializer.c dberror.h tables.h record_mgr.h
	cc -c rm_serializer.c

test_expr:  test_expr.o expr.o dberror.o record_scan.o record_mgr.o  rm_serializer.o storage_mgr.o buffer_list.o buffer_mgr.o buffer_mgr_stat.o 
	cc -o test_expr test_expr.o expr.o dberror.o record_scan.o record_mgr.o  rm_serializer.o storage_mgr.o buffer_list.o buffer_mgr.o buffer_mgr_stat.o 

test_expr.o: test_expr.c dberror.h expr.h record_mgr.h tables.h
	cc -c test_expr.c
clean:
	-rm -f *.o test_assign4 test_expr test_assign4.exe.stackdump testidx

run1:
	./test_assign4

run2:
	./test_expr
