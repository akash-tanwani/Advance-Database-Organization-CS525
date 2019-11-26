#include "optional.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"

RC initiateOptional (int numPages) 
{
  initStorageManager();
  initRecordManager(NULL);
  return RC_OK;
}

RC shutOptional (void) 
{
  shutdownRecordManager();
  return RC_OK;
}

long getOptionalInputOutput (void) 
{
  return 1;
}
