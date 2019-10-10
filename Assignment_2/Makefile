
default: testcase

testcase: dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o test_assign2_1.o
	gcc -g -Wall -o testcase storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o test_assign2_1.o -I.

test_assign2_1.o: test_assign2_1.c dberror.h storage_mgr.h test_helper.h buffer_mgr.h buffer_mgr_stat.h
	gcc -g -Wall -c test_assign2_1.c 

buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h
	gcc -g -Wall -c buffer_mgr_stat.c

buffer_mgr.o: buffer_mgr.c buffer_mgr.h dt.h storage_mgr.h
	gcc -g -Wall -c buffer_mgr.c

storage_mgr.o: storage_mgr.c storage_mgr.h 
	gcc -g -Wall -c storage_mgr.c 

dberror.o: dberror.c dberror.h 
	gcc -g -Wall -c dberror.c

clean: 
	$(RM) testcase *.o *~
	$(RM) testbuffer.bin

run:
	./testcase
