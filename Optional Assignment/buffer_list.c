#include "buffer_list.h"
#include <stdio.h>
#include <stdlib.h>
//-------------------------------------------------------------------------------------------------
RC insert_bufpool(EntryPointer *entry, void *buffer_pool_ptr, void *buffer_page_handle)
{    
    EntryPointer prvs, newptr, crnt;

    newptr = (EntryPointer)malloc(sizeof(BufferPool_Entry));
    if(newptr != NULL)
    {
        newptr->buffer_pool_ptr = buffer_pool_ptr;
        newptr->buffer_page_info = buffer_page_handle;
        newptr->numreadIO = 0;
        newptr->numwriteIO = 0;
        newptr->nextBufferEntry = NULL;

        prvs = NULL;
        crnt = *entry;

        while (crnt != NULL )
        {
            prvs = crnt;
            crnt = crnt->nextBufferEntry;
        }
        if(prvs != NULL)
            prvs->nextBufferEntry = newptr;
        else
            *entry = newptr;
        return RC_OK;
    }
    return RC_INSERT_BUFPOOL_FAILED;
}
//-------------------------------------------------------------------------------------------------
BufferPool_Entry *find_bufferPool(EntryPointer entryptr, void *buffer_pool_ptr)
{
    EntryPointer crnt = entryptr;
    while(crnt != NULL)
    {
        if(crnt->buffer_pool_ptr == buffer_pool_ptr)
            break;
        crnt = crnt->nextBufferEntry;
    }
    return crnt;
}
//-------------------------------------------------------------------------------------------------
bool delete_bufpool(EntryPointer *entryptr, void *buffer_pool_ptr )
{
    EntryPointer temp, prvs, crnt;

    prvs = NULL;
    crnt = *entryptr;

    while((crnt->buffer_pool_ptr != buffer_pool_ptr) && (crnt != NULL))
    {
        prvs = crnt;
        crnt = crnt->nextBufferEntry;
    }
    
    temp = crnt;

    if(prvs != NULL)
        prvs->nextBufferEntry = crnt->nextBufferEntry;
    else
        *entryptr = crnt->nextBufferEntry;
    free(temp);
    return TRUE;
}
//-------------------------------------------------------------------------------------------------
BufferPool_Entry *checkPoolsUsingFile(EntryPointer entry, int *filename)
{
    EntryPointer crnt = entry;
    BM_BufferPool *bufferpool;
    while(crnt!=NULL)
    {
        bufferpool = (BM_BufferPool *)crnt->buffer_pool_ptr;
        if(bufferpool->pageFile == (char *)filename)
           break;
        crnt = crnt->nextBufferEntry;
    }
    return crnt;
}
//-------------------------------------------------------------------------------------------------
int getPoolsUsingFile(EntryPointer entry, char *filename)
{
    EntryPointer crnt = entry;
    BM_BufferPool *bufferpool;

    int cnt = 0;
    while(crnt != NULL)
    {
        bufferpool = (BM_BufferPool *)crnt->buffer_pool_ptr;
        if(bufferpool->pageFile == filename)
            cnt++;
        crnt = crnt->nextBufferEntry;
    }
    return cnt;
}
//-------------------------------------------------------------------------------------------------