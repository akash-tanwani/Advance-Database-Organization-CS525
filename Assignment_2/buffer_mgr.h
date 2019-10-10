#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
  RS_FIFO = 0,
  RS_LRU = 1,
  RS_CLOCK = 2,
  RS_LFU = 3,
  RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1

typedef struct BM_PageHandle {
  PageNumber pageNum;
  char *data;
  bool dirty; // mark whether this is a dirty page.
  int fixCounts; // count how many clients are using this page.
  int *strategyAttribute; // record attribution for strategy, like midify time or create time.
} BM_PageHandle;

typedef struct BM_BufferPool {
  char *pageFile;
  int numPages;
  ReplacementStrategy strategy;
  BM_PageHandle *mgmtData; // use this one to store the bookkeeping info your buffer 
                  // manager needs for a buffer pool
  int numberReadIO; // the number of read from page file.                
  int numberWriteIO; // the number of write from page file.                               
  int buffertimer; // initial is 0, use this buffertimer to compare modify/create time.
} BM_BufferPool;


// convenience macros
#define MAKE_POOL()					\
  ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
  ((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,const int numPages, ReplacementStrategy strategy,void *stratData);
RC shutdownBufferPool(BM_BufferPool *const bm);
RC forceFlushPool(BM_BufferPool *const bm);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm);
bool *getDirtyFlags (BM_BufferPool *const bm);
int *getFixCounts (BM_BufferPool *const bm);
int getNumReadIO (BM_BufferPool *const bm);
int getNumWriteIO (BM_BufferPool *const bm);

// Custom Functions
int strategyForFIFOandLRU(BM_BufferPool *bm);
//int strategyLRU(BM_BufferPool *bm);
//int stratForLRU_k(BM_BufferPool *bm);
int *getAttributionArray(BM_BufferPool *bm);
void freePagesForBuffer(BM_BufferPool *bm);
RC updateBfrAtrbt(BM_BufferPool *bm, BM_PageHandle *pageHandle);
#endif
