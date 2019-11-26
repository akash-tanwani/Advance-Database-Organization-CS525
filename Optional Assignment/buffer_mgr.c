#include "buffer_mgr.h"
#include "buffer_list.h"
#include "storage_mgr.h"
#include "dt.h"
#include <stdio.h>
#include <stdlib.h>

static EntryPointer entry_ptr_bp = NULL;
static long double time_uni = -32674;
static char *initFrames(const int numPages);
static Buffer_page_info *findReplace(BM_BufferPool *const bm, BufferPool_Entry *entry_bp);
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC initBufferPool(BM_BufferPool *const bm, const char *const pg_file_name, const int numPages, ReplacementStrategy strategy, void *stratData)
{

    int i;
    bool insert_ok;
    Buffer_page_info *buffer_pg_info;
    BufferPool_Entry *buffer_entry;
    BM_BufferPool *pool_temp;
    SM_FileHandle *fileHandle;
    
    buffer_entry = checkPoolsUsingFile(entry_ptr_bp, (int *)pg_file_name);                                             // Check for the buffer pools using the same file.
    
    if(buffer_entry != NULL)                                                                                    // Create a buffer pool if does not exists
    {
        pool_temp = buffer_entry->buffer_pool_ptr;
        bm->pageFile = (char *)pg_file_name;
        bm->numPages = numPages;
        bm->strategy = strategy;
        bm->mgmtData = pool_temp->mgmtData;
        buffer_pg_info = buffer_entry->buffer_page_info;
        return insert_bufpool(&entry_ptr_bp, bm, buffer_pg_info);
    }
    else
    {
        fileHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
        bm->numPages = numPages;
        bm->strategy = strategy;
        bm->pageFile = (char *)pg_file_name;
        bm->mgmtData = fileHandle;
        openPageFile((char *)pg_file_name,fileHandle);
        buffer_pg_info = (Buffer_page_info *)calloc(numPages,sizeof(Buffer_page_info));
        i=0;
        while(i < numPages)
        {
            (buffer_pg_info+i)->pageframes = initFrames(numPages);
            buffer_pg_info[i].fixcounts = 0;
            buffer_pg_info[i].isdirty = FALSE;
            buffer_pg_info[i].pagenums = NO_PAGE;
            i++;
        }
        return insert_bufpool(&entry_ptr_bp,bm,buffer_pg_info);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
static char *initFrames(const int numPages)
{
    int i=0;
    char *frame;
    frame = (char *)malloc(PAGE_SIZE);
    
    if(frame!=NULL)
    {
        while(i < PAGE_SIZE)
        {
            frame[i] = '\0';
            i++;
        }
    }
    return frame;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC shutdownBufferPool(BM_BufferPool *const bm)
{

    int i, *page_num, *fix_cnts, num_pools;
    bool is_spinned = FALSE;

    Buffer_page_info *pg_info;
    BufferPool_Entry *buff_entry;
    char *frame;

    fix_cnts = getFixCounts(bm);
    page_num = getFrameContents(bm);

    i = 0;
    while(i < bm->numPages)
    {
        if(fix_cnts[i] > 0)
            is_spinned=TRUE;
        i++;
    }
    free(fix_cnts);
   
    if(is_spinned == FALSE)
    {        
        buff_entry = find_bufferPool(entry_ptr_bp,bm);                                                // Search the current buffer pool
        num_pools = getPoolsUsingFile(entry_ptr_bp,bm->pageFile);                                     // Check for shared resources, and return the count
        pg_info = buff_entry->buffer_page_info;
    
        i=0;
        while(i < bm->numPages)
        {
            frame = pg_info[i].pageframes;
            if(pg_info[i].isdirty == TRUE)
               if (writeBlock(pg_info[i].pagenums,bm->mgmtData,frame) != RC_OK)
                    return RC_WRITE_FAILED;
            if(num_pools == 1)
                free(frame);
            i++;
        }
        if(num_pools == 1)
            free(pg_info);
        pg_info = NULL;

        delete_bufpool(&entry_ptr_bp,bm);
        if(num_pools == 1)
        {
            closePageFile(bm->mgmtData);
            free(bm->mgmtData);
        }
    }
    return RC_OK;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC forceFlushPool(BM_BufferPool *const bm)
{    
    int i;
    Buffer_page_info *page_info;
    BufferPool_Entry *bufentry;
    char *frame;
    
    bufentry = find_bufferPool(entry_ptr_bp,bm);
    page_info = bufentry->buffer_page_info;
   
    i=0;
    while(i < bm->numPages)
    {
        frame = page_info[i].pageframes;
        if((page_info[i].fixcounts == 0) && (page_info[i].isdirty == TRUE))
        {
            if(writeBlock(page_info[i].pagenums,bm->mgmtData,frame) != RC_OK)
                return RC_WRITE_FAILED;              
            page_info[i].isdirty = FALSE;
            bufentry->numwriteIO++;
        }
        i++;
    }
    return RC_OK;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    int i = 0;
    PageNumber *page_num = NULL;
    Buffer_page_info *buffer_pg_info;
    
    EntryPointer buffer_entry = find_bufferPool(entry_ptr_bp,bm);
    if(buffer_entry != NULL)
    { 
        page_num = (PageNumber *)calloc(bm->numPages,sizeof(PageNumber));
        buffer_pg_info = buffer_entry->buffer_page_info;
        while(i < bm->numPages)
        {
            page_num[i] = buffer_pg_info[i].pagenums;
            i++;
        }
    }
    return page_num;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
int *getFixCounts (BM_BufferPool *const bm)
{    
    int i;
    int *fix_cnts;
    Buffer_page_info *buffer_pg_info;
    
    EntryPointer buffer_entry = find_bufferPool(entry_ptr_bp,bm);
    fix_cnts = (int*)calloc(bm->numPages,sizeof(int));
    buffer_pg_info = buffer_entry->buffer_page_info;
    if(buffer_pg_info != NULL)
    {
        for(i = 0 ; i < bm->numPages; i++)
          fix_cnts[i] = buffer_pg_info[i].fixcounts;
    }
    else
    {
      free(buffer_pg_info);
      buffer_pg_info = NULL;
    }
    return fix_cnts;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
bool *getDirtyFlags (BM_BufferPool *const bm)
{
    int i=0;
    bool  *d_flag;
    Buffer_page_info *buffer_pg_info;
    
    EntryPointer buffer_entry = find_bufferPool(entry_ptr_bp,bm);
    d_flag = (bool*)calloc(bm->numPages,sizeof(bool));
    buffer_pg_info = buffer_entry->buffer_page_info;
    if(buffer_pg_info != NULL)
    {
        while(i < bm->numPages)
        {
            d_flag[i] = buffer_pg_info[i].isdirty;
            i++;
        }
    }
    else
    {
        free(buffer_pg_info);
        buffer_pg_info = NULL;
    }
    return d_flag;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
int getNumReadIO (BM_BufferPool *const bm)
{
    EntryPointer buffer_entry = find_bufferPool(entry_ptr_bp,bm);
    return buffer_entry->numreadIO;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
int getNumWriteIO (BM_BufferPool *const bm)
{    
    EntryPointer buffer_entry=find_bufferPool(entry_ptr_bp,bm);
    return buffer_entry->numwriteIO;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) 
{   
    int i = 0;
    BufferPool_Entry *page_entry;
    Buffer_page_info *buffer_pg_info;

    page_entry = find_bufferPool(entry_ptr_bp, bm);
    buffer_pg_info = page_entry->buffer_page_info;
    
    while(i < bm->numPages) 
    {
        
        if (((buffer_pg_info + i)->pagenums) == page->pageNum) 
        {
            (buffer_pg_info + i)->isdirty = TRUE;
            return RC_OK;
        }
        i++;
    }
   
    return RC_MARK_DIRTY_FAILED;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page)
{    
    BufferPool_Entry *entry_bp;
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    
    if (bm->mgmtData != NULL) 
    {
        writeBlock(page->pageNum, bm->mgmtData, page->data);
        entry_bp->numwriteIO = entry_bp->numwriteIO + 1;
        return RC_OK;
    }
    return RC_FORCE_PAGE_ERROR;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) 
{
    int i = 0, tot_pages = bm->numPages;;
 
    BufferPool_Entry *entry_bp;
    Buffer_page_info *page_info;
    
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    page_info = entry_bp->buffer_page_info;
    
    while(i < tot_pages) 
    {
        if ((page_info + i)->pagenums == page->pageNum)
        {
            (page_info + i)->fixcounts = (page_info + i)->fixcounts -  1;
            return RC_OK;
        }
        i++;
    }
    return RC_UNPIN_FAILED;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC applyFIFO(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) 
{
    BufferPool_Entry *entry_bp;
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    Buffer_page_info *rep_possible = NULL;
   
    rep_possible = findReplace(bm, entry_bp);
    
    RC write_ok = RC_OK;
    RC read_ok = RC_OK;
    
    if (rep_possible->isdirty == TRUE) 
    {
        write_ok = writeBlock(rep_possible->pagenums, bm->mgmtData, rep_possible->pageframes);
        rep_possible->isdirty = FALSE;
        entry_bp->numwriteIO++;
    }
    
    read_ok = readBlock(pageNum, bm->mgmtData, rep_possible->pageframes);
    if((read_ok == RC_READ_NON_EXISTING_PAGE) || (read_ok == RC_OUT_OF_BOUNDS) || (read_ok == RC_READ_FAILED))
        read_ok = appendEmptyBlock(bm->mgmtData);

    entry_bp->numreadIO++;
    page->pageNum  = pageNum;
    page->data = rep_possible->pageframes;
    rep_possible->pagenums = pageNum;
    rep_possible->fixcounts = rep_possible->fixcounts + 1;
    rep_possible->weight = rep_possible->weight + 1;

    if((read_ok == RC_OK) && (write_ok == RC_OK))
        return RC_OK;
    else
        return RC_FIFO_FAILED;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC applyLRU(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum)
 {
    BufferPool_Entry *entry_bp;
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    Buffer_page_info *rep_possible;
    rep_possible = findReplace(bm, entry_bp);

    RC write_ok = RC_OK;
    RC read_ok = RC_OK;

    if (rep_possible->isdirty == TRUE) 
    {
        write_ok = writeBlock(page->pageNum, bm->mgmtData, rep_possible->pageframes);
        rep_possible->isdirty = FALSE;
        entry_bp->numwriteIO = entry_bp->numwriteIO + 1;
    }
    
    read_ok = readBlock(pageNum, bm->mgmtData, rep_possible->pageframes);
    entry_bp->numreadIO++;
    page->pageNum  = pageNum;
    page->data = rep_possible->pageframes;
    rep_possible->pagenums = pageNum;
    rep_possible->fixcounts = rep_possible->fixcounts +1;
    rep_possible->weight = rep_possible->weight + 1;
    rep_possible->timeStamp = (long double)time_uni++;

    if ((read_ok == RC_OK) && (write_ok == RC_OK))
        return RC_OK;
    else
        return RC_LRU_FAILED;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC applyLFU(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum)
{
    BufferPool_Entry *entry_bp;
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    Buffer_page_info *rep_possible=NULL;
    rep_possible = findReplace(bm, entry_bp);

    RC write_ok = RC_OK;

    if (rep_possible->isdirty == TRUE) 
    {
        write_ok = writeBlock(rep_possible->pagenums, bm->mgmtData, rep_possible->pageframes);
        rep_possible->isdirty = FALSE;
        entry_bp->numwriteIO = entry_bp->numwriteIO + 1;
    }

    RC read_ok = readBlock(pageNum, bm->mgmtData, rep_possible->pageframes);
    if((read_ok == RC_READ_NON_EXISTING_PAGE) || (read_ok == RC_OUT_OF_BOUNDS) || (read_ok == RC_READ_FAILED))
        read_ok = appendEmptyBlock(bm->mgmtData);
    entry_bp->numreadIO = entry_bp->numreadIO + 1;
    page->pageNum  = pageNum;
    page->data = rep_possible->pageframes;
    rep_possible->pagenums = pageNum;
    rep_possible->fixcounts = rep_possible->fixcounts + 1;
    rep_possible->weight = rep_possible->weight + 1;
    
    if ((read_ok == RC_OK) && (write_ok == RC_OK))
        return RC_OK;
    else
        return RC_LFU_FAILED;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) 
{
    int i = 0, emptyFlag = 1;
    RC pin_ok = RC_OK;
    Buffer_page_info *rep_possible, *page_info;
    BufferPool_Entry *entry_bp;
   
    entry_bp = find_bufferPool(entry_ptr_bp, bm);
    page_info = entry_bp->buffer_page_info;

    if (page_info != NULL) 
    {
        while(i < bm->numPages)
        {           
            emptyFlag = 0;
            if ((page_info + i)->pagenums == pageNum) 
            {
                
                (page_info + i)->timeStamp = time_uni++;                                                      // LRU only
                page->pageNum = pageNum;
                page->data = (page_info + i)->pageframes;
                (page_info + i)->fixcounts = (page_info + i)->fixcounts + 1;
                return RC_OK;
            }
            i++;
        }
        
        i=0;
        while(i < bm->numPages) 
        {
            if ((page_info + i)->pagenums == -1)
            {
                rep_possible = ((page_info + i));
                emptyFlag = 1;
                break;
            }
            i++;
        }
    }
   
    if (emptyFlag != 1)
    {   
        if (bm->strategy == RS_FIFO) 
            return applyFIFO(bm, page, pageNum);
        else if (bm->strategy == RS_LRU) 
            return applyLRU(bm, page, pageNum);
        else if (bm->strategy == RS_LRU) 
            return applyLFU(bm, page, pageNum);
        else 
            return RC_PIN_FAILED; 
    }
    else 
    {
        page->pageNum = pageNum;
        page->data = rep_possible->pageframes;
        
        pin_ok = readBlock(pageNum, bm->mgmtData,rep_possible->pageframes);
        if((pin_ok == RC_READ_NON_EXISTING_PAGE) || (pin_ok == RC_OUT_OF_BOUNDS))
            pin_ok = appendEmptyBlock(bm->mgmtData);
        else
            pin_ok = RC_OK;
        entry_bp->numreadIO++;
        rep_possible->fixcounts = rep_possible->fixcounts + 1;
        rep_possible->pagenums = pageNum;
        return pin_ok;
    } 
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
Buffer_page_info *findReplace(BM_BufferPool * const bm, BufferPool_Entry *entry_bp) 
{   
    int i=0, cnt = 0;
    
    Buffer_page_info *bf_page_info = entry_bp->buffer_page_info, *bf_page_info_zeros[bm->numPages];

    while(i< bm->numPages)
    {
        bf_page_info_zeros[i] = NULL;
        i++;
    }
    
    i=0;
    while(i < bm->numPages)
    {
        if ((bf_page_info[i].fixcounts) == 0) 
        {
            bf_page_info_zeros[cnt] = (bf_page_info+i);
            cnt++;
        }
        i++;
    }
   
    #define sizeofa(array) sizeof array / sizeof array[0]
    Buffer_page_info *next_bf_page_info, *replace_bf_page_info;

    replace_bf_page_info = bf_page_info_zeros[0];
    i=0;
    while(i < sizeofa(bf_page_info_zeros))
     {
        next_bf_page_info = bf_page_info_zeros[i];
        if(next_bf_page_info != NULL)
            if(bm->strategy == RS_FIFO)
                if ((replace_bf_page_info->weight) > (next_bf_page_info->weight))
                    replace_bf_page_info = next_bf_page_info;
            else if(bm->strategy == RS_LRU)
                if(replace_bf_page_info->timeStamp > next_bf_page_info->timeStamp)
                    replace_bf_page_info = next_bf_page_info;
            else if (bm->strategy == RS_LFU)
                if ((replace_bf_page_info->weight) > (next_bf_page_info->weight))
                    replace_bf_page_info = next_bf_page_info;
        i++;
    }
    return replace_bf_page_info;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------