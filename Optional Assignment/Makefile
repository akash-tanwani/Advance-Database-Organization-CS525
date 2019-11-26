# Source, Executable, Includes, Library Defines
SRC    = dberror.c expr.c storage_mgr.c buffer_mgr.c record_scan.c record_mgr.c optional_setup.c rm_serializer.c buffer_mgr_stat.c buffer_list.c test_optional.c
OBJ    = $(SRC:.c=.o)

# Compile and Assemble C Source Files into Object Files
%.o: %.c
	gcc -c -w $*.c

# Link all Object Files with external Libraries into Binaries
optional: $(OBJ)
	gcc -o optional -L. $(OBJ)

# Objects depend on these Libraries
$(OBJ): dberror.h expr.h storage_mgr.h buffer_mgr.h record_scan.h record_mgr.h optional.h  tables.h buffer_mgr_stat.h buffer_list.h test_helper.h 

# Run
run:
	./optional

# Clean Up Objects, Exectuables, Dumps out of source directory
clean:
	rm -f $(OBJ) optional test_table_r.bin
