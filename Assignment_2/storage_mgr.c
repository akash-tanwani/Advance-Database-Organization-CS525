#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "storage_mgr.h"

void initStorageManager(void) 
{
	 printf("\nStorage Manager has been initialized.");
}

RC createPageFile(char *fileName) 
{
	FILE *f;
	char load[PAGE_SIZE];
	int result;

	f = fopen(fileName, "w+");

	if (f != NULL)
	{
		memset(load, '\0', sizeof(load));
		result = fwrite(load, 1, PAGE_SIZE, f);

		if (result != PAGE_SIZE) 
		{
			fclose(f);
			destroyPageFile(fileName);
			return RC_CREATE_FILE_FAIL;
		}

		fclose(f);
		return RC_OK;	
	}

	else
	{
		fclose(f);
		return RC_CREATE_FILE_FAIL;
	}
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
	int sz;
	int getResult;

	FILE *f = fopen(fileName, "r+");

	if (f != NULL)
	{
		getResult = fseek(f, 0, SEEK_END);

		if (getResult != 0) 
		{
			fclose(f);
			return RC_GET_NUMBER_OF_BYTES_FAILED;
		}

		sz = ftell(f);
		if (sz == -1)
		{
			fclose(f);
			return RC_GET_NUMBER_OF_BYTES_FAILED;
		}

		fHandle->fileName = fileName;
		fHandle->totalNumPages = (int)(sz % PAGE_SIZE + 1);
		fHandle->curPagePos = 0;

		return RC_OK;
	} 

	else	
		return RC_FILE_NOT_FOUND;
}

RC closePageFile (SM_FileHandle *fHandle) 
{
	fHandle->fileName = "";
	fHandle->curPagePos = 0;
	fHandle->totalNumPages = 0;
	return RC_OK;
}

RC destroyPageFile (char *fileName) 
{
	int destroyResult = remove(fileName);
	if (destroyResult != 0)
		return RC_FILE_NOT_FOUND;
	else
	 	return RC_OK;	
}

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (pageNum > 0 || (pageNum < fHandle -> totalNumPages - 1))
	{
		FILE *f;
		f = fopen(fHandle->fileName, "r");
		int ofst;
		ofst = fHandle->curPagePos * PAGE_SIZE;
		fseek(f, ofst, SEEK_SET);
		fread(memPage, sizeof(char), PAGE_SIZE, f);
		fHandle->curPagePos = pageNum;
		fclose(f);
		return RC_OK;	
	}
	else
		return RC_READ_NON_EXISTING_PAGE;
}

int getBlockPos (SM_FileHandle *fHandle)
{
	return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	FILE *f;
	f = fopen(fHandle -> fileName, "r+");
	fseek(f, 0, SEEK_SET);
	fread(memPage, sizeof(char), PAGE_SIZE, f);
	fHandle->curPagePos = 0;
	fclose(f);
	return RC_OK;
}


RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if ((fHandle -> curPagePos > 0) || (fHandle -> curPagePos < fHandle -> totalNumPages - 1))
	{
		FILE *f;
		f = fopen(fHandle->fileName, "r+");
		int ofst = (fHandle->curPagePos - 1) * PAGE_SIZE;
		fseek(f, ofst, SEEK_SET);
		fread(memPage, sizeof(char), PAGE_SIZE, f);
		fHandle->curPagePos = fHandle->curPagePos - 1;
		fclose(f);
		return RC_OK;
	}
	else
		return RC_READ_NON_EXISTING_PAGE;
}



RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if ((fHandle -> curPagePos > 0) || (fHandle -> curPagePos < fHandle -> totalNumPages - 1))
	{
		FILE *f;
		f = fopen(fHandle->fileName, "r+");
		int ofst = fHandle->curPagePos * PAGE_SIZE;
		fseek(f, ofst, SEEK_SET);
		fread(memPage, sizeof(char), PAGE_SIZE, f);
		fclose(f);
		return RC_OK;
	}
	else
		return RC_READ_NON_EXISTING_PAGE;
}


RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if ((fHandle->curPagePos > 0) || (fHandle -> curPagePos < fHandle -> totalNumPages - 1))
	{
		FILE *f;
		f = fopen(fHandle->fileName, "r+");
		int ofst = (fHandle->curPagePos + 1) * PAGE_SIZE;
		fseek(f, ofst, SEEK_SET);
		fread(memPage, sizeof(char), PAGE_SIZE, f);
		fHandle->curPagePos = fHandle->curPagePos + 1;
		fclose(f);
		return RC_OK;
	}
	else
		return RC_READ_NON_EXISTING_PAGE;
}


RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	FILE *f;
	f = fopen(fHandle->fileName, "r+");
	int ofst = -PAGE_SIZE;
	fseek(f, ofst, SEEK_END);
	fread(memPage, sizeof(char), PAGE_SIZE, f);
	fHandle->curPagePos = fHandle->totalNumPages - 1;
	fclose(f);
	return RC_OK;
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) 
{
	ensureCapacity (pageNum, fHandle);														//Make sure the program have enough capacity to write block.
	FILE *f= fopen(fHandle->fileName, "rb+");
	RC returnValue;
	
	if (fseek(f, pageNum * PAGE_SIZE, SEEK_SET) == 0) 
	{
		if (fwrite(memPage, PAGE_SIZE, 1, f) != 1) 
			returnValue = RC_WRITE_FAILED;
		else 
		{
			fHandle->curPagePos = pageNum;													//Success write block, then curPagePos should be changed.
			returnValue = RC_OK;
		}
	}
	else
		returnValue = RC_READ_NON_EXISTING_PAGE; 

	fclose(f);
	return returnValue;
}



RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL) 
		return RC_FILE_HANDLE_NOT_INIT;
	else if (fHandle->curPagePos < 0) 
		return RC_WRITE_FAILED;
	else
		return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock (SM_FileHandle *fHandle) 
{
	if (fHandle == NULL) 
		return RC_FILE_HANDLE_NOT_INIT;
	else
	{
		RC returnValue;

		char *allocateData = (char*)calloc(1, PAGE_SIZE);
		FILE *f = fopen(fHandle->fileName, "ab+");

		if (fwrite(allocateData, PAGE_SIZE, 1, f) == 1)
		{
			fHandle -> totalNumPages = fHandle -> totalNumPages + 1;
			returnValue = RC_OK;
		} 
		else 
			returnValue = RC_WRITE_FAILED;

		free(allocateData);
		fclose(f);
		return  returnValue;
	}	
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) 
{
	if (fHandle == NULL)
		return RC_FILE_HANDLE_NOT_INIT;
	else if (fHandle -> totalNumPages >= numberOfPages)
		return RC_OK;
	else
	{
		FILE *f = fopen(fHandle->fileName, "ab+");;
		long allocateCapacity;
		char *allocateData;
		RC returnValue;

		allocateCapacity = (numberOfPages - fHandle -> totalNumPages) * PAGE_SIZE;
		allocateData = (char *)calloc(1, allocateCapacity);

		if (fwrite(allocateData, allocateCapacity, 1, f) != 0)
		{
			fHandle -> totalNumPages = numberOfPages;									//When write operation is successfull, the totalNumPages should be changed to numberOfPages.
			returnValue = RC_OK;
		} 
		else 
			returnValue = RC_WRITE_FAILED;

		free(allocateData);
		fclose(f);
		return returnValue;
	}	
}