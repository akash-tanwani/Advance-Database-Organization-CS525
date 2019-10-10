#include "buffer_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "storage_mgr.h"
//---------------------------------------------------------------------------------------------------------
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,const int numPages, ReplacementStrategy strategy,void *stratData) 
{
    FILE *f = fopen(pageFileName, "r+");;
    int i=0;

    if (f != NULL)
    {
        fclose(f);

        bm->pageFile = (char *)pageFileName;
        bm->numPages = numPages;
        bm->strategy = strategy;
        BM_PageHandle* buff = (BM_PageHandle *)calloc(numPages, sizeof(BM_PageHandle));
        bm->mgmtData = buff;
        while(i < numPages)
        {
            (i + bm->mgmtData)->dirty = 0;
            (i + bm->mgmtData)->fixCounts = 0;
            (i + bm->mgmtData)->data = NULL;
            (i + bm->mgmtData)->pageNum = -1;
            i++;
        }
         bm->buffertimer = 0;
         bm->numberReadIO = 0;
         bm->numberWriteIO = 0;
         return RC_OK;    
    } 
    else 
        return RC_FILE_NOT_FOUND;
}
//---------------------------------------------------------------------------------------------------------
RC shutdownBufferPool(BM_BufferPool *const bm) 
{
    int *fixCnt = getFixCounts(bm);
    int i=0;
    while(i < bm->numPages) 
    {
        if (!(*(i + fixCnt))) 
            i++;
        else
        {
            free(fixCnt);
            return RC_SHUTDOWN_POOL_FAILED;
        }
    }

    RC RCflg = forceFlushPool(bm);
    if (RCflg == RC_OK) 
    {
        freePagesForBuffer(bm);
        free(fixCnt);
        free(bm->mgmtData);
        return RC_OK;
    }
    else
    {
        free(fixCnt);
        return RCflg;
    }    
}
//---------------------------------------------------------------------------------------------------------
RC forceFlushPool(BM_BufferPool *const bm) 
{    
    BM_PageHandle* page;
    RC RCflg;

    bool *dFlg = getDirtyFlags(bm);
    int *fixCnt = getFixCounts(bm);

    int i =0;
    while (i < bm->numPages) 
    {
        if (*(dFlg + i) == 1) 
        {
            if (!(*(fixCnt + i))) 
            {
                page = ((bm->mgmtData) + i);
                RCflg = forcePage(bm, page);
                if (RCflg != RC_OK) 
                {
                    free(dFlg);
                    free(fixCnt);
                    return RCflg;
                }
            }
            else
                continue;
        }
        i++;
    }
    i = 0;
    while (i < bm->numPages)
    {
        if (*(dFlg + i))
            ((bm->mgmtData) + i)->dirty = 0;
        i++;
    }

    free(dFlg);
    free(fixCnt);
    return RC_OK;
}
//---------------------------------------------------------------------------------------------------------
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    int i=0;
    while(i < (bm->numPages))
    {
        if ((i+ bm->mgmtData)->pageNum == page->pageNum)
        {
            page->dirty = 1;
            (i + bm->mgmtData)->dirty = 1;
            break;
        }
        i++;
    }
    return RC_OK;
}
//---------------------------------------------------------------------------------------------------------
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    int i = 0;
    while (i < bm->numPages)
    {
        if ((bm->mgmtData + i) -> pageNum == page -> pageNum)
        {
            (bm->mgmtData + i)-> fixCounts = (bm->mgmtData + i)-> fixCounts - 1;
            break;
        }
        i++;
    }
    return RC_OK;
}
//---------------------------------------------------------------------------------------------------------
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    FILE *f = fopen(bm->pageFile, "rb+");;
    fseek(f, (page->pageNum)*PAGE_SIZE, SEEK_SET);
    fwrite(page->data, PAGE_SIZE, 1, f);
    bm->numberWriteIO = bm->numberWriteIO + 1;
    fclose(f);
    int i=0;
    while(i < bm->numPages)
    {
        if ((bm->mgmtData + i)->pageNum != page->pageNum)
            i++;
        else
        {
            (bm->mgmtData + i)->dirty = 0;
            break;
        }
    }
    page->dirty = 0;
    return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    int pnum, fg = 0, i= 0;
    
    while (i < bm->numPages)
    {
        if ((bm->mgmtData + i)->pageNum == -1)
        {
            (bm->mgmtData + i)->data = (char*)calloc(PAGE_SIZE, sizeof(char));
            fg = 1;
            pnum = i;
            break;
        }
        else if ((bm->mgmtData + i)->pageNum == pageNum)
        {
            fg = 2;
            pnum = i;
            if (bm->strategy == RS_LRU)
                updateBfrAtrbt(bm, bm->mgmtData + pnum);
            break;
        }
        else if (i == bm->numPages - 1)
        {

            fg = 1;
            if (bm->strategy == RS_FIFO)
            {
                pnum = strategyForFIFOandLRU(bm);
                if ((bm->mgmtData + pnum)->dirty)
                    forcePage (bm, bm->mgmtData + pnum);
            }
            else if (bm->strategy == RS_LRU)
            {
                pnum = strategyForFIFOandLRU(bm);
                if ((bm->mgmtData + pnum)->dirty)
                    forcePage (bm, bm->mgmtData + pnum);
            }
        }
        i++;
    }
        
    switch(fg)
    {
        case 1:
        {
            FILE *fp = fopen(bm->pageFile, "r");
            fseek(fp, pageNum * PAGE_SIZE, SEEK_SET);
            fread((bm->mgmtData + pnum)->data, sizeof(char), PAGE_SIZE, fp);
            page->data = (bm->mgmtData + pnum)->data;
            bm->numberReadIO = bm->numberReadIO + 1;
            (bm->mgmtData + pnum)->fixCounts = (bm->mgmtData + pnum)->fixCounts + 1;
            (bm->mgmtData + pnum)->pageNum = pageNum;
            page->fixCounts = (bm->mgmtData + pnum)->fixCounts;
            page->dirty = (bm->mgmtData + pnum)->dirty;
            page->pageNum = pageNum;
            page->strategyAttribute = (bm->mgmtData + pnum)->strategyAttribute;
            updateBfrAtrbt(bm, bm->mgmtData + pnum);
            fclose(fp);
            break;
        }

        case 2:
        {
            page->data = (bm->mgmtData + pnum)->data;
            (bm->mgmtData + pnum)->fixCounts = (bm->mgmtData + pnum)->fixCounts + 1;
            page->fixCounts = (bm->mgmtData + pnum)->fixCounts;
            page->dirty = (bm->mgmtData + pnum)->dirty;
            page->pageNum = pageNum;
            page->strategyAttribute = (bm->mgmtData + pnum)->strategyAttribute;
            break;
        }
    }
    return RC_OK;
}
//---------------------------------------------------------------------------------------------------------
PageNumber *getFrameContents (BM_BufferPool *const bm) 
{
    PageNumber *pageNoArray = (PageNumber*)malloc(bm->numPages * sizeof(PageNumber));
    int i = 0;
    while (i < bm->numPages) 
    {
        if ((bm->mgmtData + i)->data != NULL) 
            pageNoArray[i] = (bm->mgmtData+i)->pageNum;
        else 
            pageNoArray[i] = NO_PAGE;
        i++;
    }
    return pageNoArray;
}
//---------------------------------------------------------------------------------------------------------
bool *getDirtyFlags (BM_BufferPool *const bm) 
{
    bool *pageNoArray = (bool*)malloc(bm->numPages * sizeof(bool));
    for (int i = 0; i < bm->numPages; i++) 
        pageNoArray[i] = (bm->mgmtData + i) -> dirty;
    return pageNoArray;
}
//---------------------------------------------------------------------------------------------------------
int *getFixCounts (BM_BufferPool *const bm) 
{
    int i = 0;
    int *pageNoArray = (int*)malloc(bm->numPages * sizeof(int));
    while (i < bm->numPages)
    {
        pageNoArray[i] = (bm->mgmtData + i) -> fixCounts;
        i++;
    }
    return pageNoArray;
}
//---------------------------------------------------------------------------------------------------------
int getNumReadIO (BM_BufferPool *const bm) 
{
    return bm->numberReadIO;
}
//---------------------------------------------------------------------------------------------------------
int getNumWriteIO (BM_BufferPool *const bm) 
{
    return bm->numberWriteIO;
}
//----------------------------------------------------------------------------------------------------------
int strategyForFIFOandLRU(BM_BufferPool *bm) 
{
    int min, abrtPg, i=0;
    int *atrbts = (int *)getAttributionArray(bm);
    int *fixCnt = getFixCounts(bm);

    min = bm->buffertimer;
    abrtPg = -1;

    for (i = 0; i < bm->numPages; ++i) 
    {
        if (*(fixCnt + i) != 0) 
            continue;

        if (min >= (*(atrbts + i))) 
        {
            abrtPg = i;
            min = (*(atrbts + i));
        }
    }

    if ((bm->buffertimer) > 32000) 
    {
        bm->buffertimer = bm->buffertimer - min;
        i = 0;
        while (i < bm->numPages) 
        {
            *(bm->mgmtData->strategyAttribute) = *(bm->mgmtData->strategyAttribute) - min;
            i++;
        }
    }
    return abrtPg;
}
//----------------------------------------------------------------------------------------------------------
int *getAttributionArray(BM_BufferPool *bm) 
{
    int *flag;
    int i = 0;

    int *atrbts = (int*)calloc(bm->numPages, sizeof((bm->mgmtData)->strategyAttribute));
    while (i < bm->numPages) 
    {
        flag = atrbts + i;
        *flag = *((bm->mgmtData + i)->strategyAttribute);
        i++;
    }
    return atrbts;
}
//----------------------------------------------------------------------------------------------------------
void freePagesForBuffer(BM_BufferPool *bm) 
{
    int i=0;
    while (i < bm->numPages) 
    {
        free((i + bm->mgmtData)->data);
        free((i + bm->mgmtData)->strategyAttribute);
        i++;
    }
}
//----------------------------------------------------------------------------------------------------------
RC updateBfrAtrbt(BM_BufferPool *bm, BM_PageHandle *pageHandle) 
{
    if (pageHandle->strategyAttribute == NULL) 
    {
        if (bm->strategy == RS_LRU || bm->strategy == RS_FIFO)
            pageHandle->strategyAttribute = calloc(1, sizeof(int));
    }

    if (bm->strategy == RS_LRU || bm->strategy == RS_FIFO)                          // Assigning the number
    {
        int *atrbt;
        atrbt = (int*) pageHandle -> strategyAttribute;
        *atrbt = (bm -> buffertimer);
        bm -> buffertimer = bm -> buffertimer + 1;
        return RC_OK; 
    }
    return RC_STRATEGY_NOT_FOUND;
}
//----------------------------------------------------------------------------------------------------------