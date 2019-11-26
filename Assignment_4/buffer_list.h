
#ifndef BUFFER_LIST_H_INCLUDED
#define BUFFER_LIST_H_INCLUDED

#include "buffer_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include "dt.h"

typedef struct BufferPool_Entry
{
    void *buffer_pool_ptr;
    void *buffer_page_info;
    int numreadIO;
    int numwriteIO;
    struct BufferPool_Entry *nextBufferEntry;
} BufferPool_Entry, *EntryPointer;

typedef struct Buffer_page_info
{
    char *pageframes;
    PageNumber pagenums;
    bool isdirty;
    int fixcounts;
    int weight;
    long double timeStamp;
} Buffer_page_info;


RC insert_bufpool(EntryPointer *entry, void *buffer_pool_ptr, void * buffer_page_dtl);
BufferPool_Entry *find_bufferPool(EntryPointer entryptr, void * buffer_pool_ptr);
bool delete_bufpool(EntryPointer *entryptr, void *buffer_pool_ptr);
BufferPool_Entry *checkPoolsUsingFile(EntryPointer entry, int *filename);
int getPoolsUsingFile(EntryPointer entry, char *filename);

#endif 
