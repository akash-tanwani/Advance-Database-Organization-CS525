#include "record_scan.h"
#include <stdlib.h>
#include <stdio.h>
//------------------------------------------------------------------------------------
AUX_Scan *search(RM_ScanHandle *sHandle, PTR_Scan sEntry)
{
    PTR_Scan curr_node = sEntry;
    while((curr_node != NULL) && (curr_node->sHandle != sHandle))
        curr_node = curr_node->nextScan;
    return curr_node->auxScan;
}
//------------------------------------------------------------------------------------
RC insert(RM_ScanHandle *sHandle, PTR_Scan *sEntry, AUX_Scan *auxScan)
{
    PTR_Scan prev_node = NULL, curr_node = *sEntry, new_node = (Scan_Entry *)malloc(sizeof(Scan_Entry));

    new_node->auxScan = auxScan;
    new_node->sHandle = sHandle;

    while(curr_node != NULL)
    {
        prev_node = curr_node;
        curr_node = curr_node->nextScan;
    }

    if(prev_node != NULL) 
        prev_node->nextScan = new_node;
    else 
        *sEntry = new_node;

    return RC_OK;
}
//------------------------------------------------------------------------------------
RC delete(RM_ScanHandle *sHandle, PTR_Scan *sEntry)
{
    PTR_Scan curr_node = *sEntry;
    if(curr_node != NULL)
        *sEntry = curr_node->nextScan;
    return RC_OK;
}
//------------------------------------------------------------------------------------