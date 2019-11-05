# Source
SRC    = dberror.c expr.c storage_mgr.c buffer_mgr.c record_scan.c record_mgr.c rm_serializer.c buffer_mgr_stat.c buffer_list.c test_assign3_1.c
OBJ    = $(SRC:.c=.o)

# Compile and Assemble C Files
%.o: %.c
	gcc -c -w $*.c

# Linking all Object Files
record_mgr: $(OBJ)
	gcc -o record_mgr -L. $(OBJ)

$(OBJ): dberror.h expr.h storage_mgr.h buffer_mgr.h record_scan.h record_mgr.h tables.h buffer_mgr_stat.h buffer_list.h test_helper.h

# Clean Up
clean:
	/bin/rm -f $(OBJ) record_mgr core a.out

# Run
run:
	./record_mgr