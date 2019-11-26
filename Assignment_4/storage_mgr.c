#include "dberror.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
//--------------------------------------------------------------------------------
void initStorageManager (void)
{

}
//--------------------------------------------------------------------------------
RC createPageFile (char *fileName)
{
    if(access(fileName, F_OK) != 0)
    {
   
        FILE *f = fopen(fileName,"wb");
        int i = 0;
        while(i<PAGE_SIZE)
        {
            fprintf(f,"%c",'\0');
        	i++;
        }
        fclose(f);
        return RC_OK;
    }
    else
        return RC_OK; 
}
//--------------------------------------------------------------------------------
RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{

    if(access(fileName, F_OK)!=0)
    	return RC_FILE_NOT_FOUND;
    else
    {
        FILE *f = fopen(fileName,"rb+");
        struct stat s;
        stat(fileName, &s);
        int totalPages = (s.st_size-1)/PAGE_SIZE;

        fHandle->totalNumPages = totalPages+1;
        fHandle->fileName = fileName;
        fHandle->mgmtInfo = f;
        fHandle->curPagePos = 0;
        return RC_OK;
    }
}
//--------------------------------------------------------------------------------
RC closePageFile (SM_FileHandle *fHandle)
{
    if(fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if(fclose(fHandle->mgmtInfo) != 0)
    	return RC_FILE_NOT_FOUND;
    else
        return RC_OK;
}
//--------------------------------------------------------------------------------
RC destroyPageFile (char *fileName)
{
    if(remove(fileName) == 0)
    	return RC_OK;
	else
        return RC_FILE_NOT_FOUND;  
}
//--------------------------------------------------------------------------------
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
    int num_pages = fHandle->totalNumPages;    

    if((f == NULL) || (pageNum > num_pages ))
        return RC_READ_NON_EXISTING_PAGE;
    fseek(f,pageNum * PAGE_SIZE, SEEK_SET);
    fread(memPage, PAGE_SIZE,1,f); 
    fHandle->curPagePos = pageNum;
    return RC_OK;
}
//--------------------------------------------------------------------------------
int getBlockPos (SM_FileHandle *fHandle)
{   
    return fHandle->curPagePos;;
}
//--------------------------------------------------------------------------------
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
    if(f != NULL )
    {
	    fseek(f,0, SEEK_SET);
	    rewind(f);
	    fread(memPage, PAGE_SIZE,1,f); 
	    fHandle->curPagePos= 1;
	    return RC_OK;
	}
    else
    {
        printError(RC_READ_NON_EXISTING_PAGE);
        return RC_READ_NON_EXISTING_PAGE;
    }
}
//--------------------------------------------------------------------------------
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int pageNum = fHandle->totalNumPages, num_pages = fHandle->totalNumPages;
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
  
    if(f!=NULL )
    {
	    fseek(f,PAGE_SIZE*(pageNum-1), SEEK_SET);
	    fread(memPage, PAGE_SIZE,1,f); 
	    fHandle->curPagePos= num_pages;
	    return RC_OK;
    }
    else
        return RC_READ_NON_EXISTING_PAGE;
}
//--------------------------------------------------------------------------------
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int pageNum = fHandle->curPagePos, pos = fHandle->curPagePos, num_pages = fHandle->totalNumPages; 
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
    
    if((f == NULL) || (pos < 0) || (pos > num_pages) )
        return RC_READ_NON_EXISTING_PAGE;
    fseek(f,PAGE_SIZE*(pageNum-1), SEEK_CUR);
    fread(memPage, PAGE_SIZE,1,f); 
    return RC_OK;
}
//--------------------------------------------------------------------------------
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int pageNum = fHandle->curPagePos, pos = fHandle->curPagePos; 
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
   
    if((f == NULL) || ((pos - 2) <0 ))
        return RC_READ_NON_EXISTING_PAGE;
    fseek(f,PAGE_SIZE * (pageNum - 2), SEEK_SET);
    fread(memPage, PAGE_SIZE,1,f); 
    fHandle->curPagePos = pageNum - 1;
    return RC_OK;
}
//--------------------------------------------------------------------------------
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int pageNum = fHandle->curPagePos, pos = fHandle->curPagePos, num_pages = fHandle->totalNumPages;
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
    
    if((f == NULL) || ((pos + 1) > num_pages ))
        return RC_READ_NON_EXISTING_PAGE;
    
    fseek(f,1, SEEK_CUR);
    fread(memPage, PAGE_SIZE,1,f); 
    fHandle->curPagePos = pageNum + 1;
    return RC_OK;
}
//--------------------------------------------------------------------------------
static RC writePage(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    FILE *f = fHandle->mgmtInfo;
    char *fileName = fHandle->fileName;
    if((f == NULL) || (pageNum < -1))
        return RC_WRITE_FAILED;
    
    if (pageNum > fHandle->totalNumPages)
        fHandle->totalNumPages = pageNum + 1;
    
    fseek(f, pageNum * PAGE_SIZE, 0);
    fwrite(memPage,PAGE_SIZE,1,f); 
    if (ferror(f) || feof(f)) 
        return RC_WRITE_FAILED;
    return RC_OK;
}
//--------------------------------------------------------------------------------
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return writePage(pageNum, fHandle, memPage);
}
//--------------------------------------------------------------------------------
 RC appendEmptyBlock (SM_FileHandle *fHandle)
 {
    FILE *f = fHandle->mgmtInfo;
    
    if(f != NULL )
    {
	    int i = 0;
	    while(i<PAGE_SIZE)
	    {
	        fprintf(f,"%c",'\0');
	        i++;
	    }  
	    fHandle->totalNumPages = fHandle->totalNumPages + 1;
	    return RC_OK;
    }
    else
        return RC_FILE_NOT_FOUND;
}
//--------------------------------------------------------------------------------
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int pageNum = (fHandle->curPagePos);
    return writePage(pageNum, fHandle, memPage);
}
//--------------------------------------------------------------------------------
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
    if (fHandle->totalNumPages < numberOfPages)
    {
        int add_page = (numberOfPages - fHandle->totalNumPages)*PAGE_SIZE, i=0;
        while(i<add_page)
        {
            fprintf(fHandle->mgmtInfo, "%c",'\0');
            i++;
        }
        fHandle->totalNumPages = numberOfPages;
        return RC_OK;
    } 
    else 
        return RC_ENOUGH_NUMB_PAGES;
}
//--------------------------------------------------------------------------------