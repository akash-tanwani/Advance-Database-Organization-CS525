#ifndef RECORD_SCAN_H
#define RECORD_SCAN_H

#include "buffer_mgr.h"
#include "record_mgr.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct AUX_Scan
{
    BM_PageHandle *pHandle;
    int _sPage;
    int _slotID;
    int _recLength;
    int _recsPage;
    int _numPages;
}AUX_Scan;

typedef struct Scan_Entry
{
    RM_ScanHandle *sHandle;
    AUX_Scan *auxScan;
    struct Scan_Entry *nextScan;
} Scan_Entry,*PTR_Scan;
//--------------------------------------------------------------------------
AUX_Scan *search(RM_ScanHandle *sHandle, PTR_Scan sEntry);
RC insert(RM_ScanHandle *sHandle, PTR_Scan *sEntry, AUX_Scan *auxScan);
RC delete(RM_ScanHandle *sHandle, PTR_Scan *sEntry);
//--------------------------------------------------------------------------
#endif 
