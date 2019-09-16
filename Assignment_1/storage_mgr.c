#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "storage_mgr.h"
#include "dberror.h"

//------------------------------------------------------------------------------------------------------------------------------------
void initStorageManager (void)
{
  printf("\nStorage Manager has been initialized.");
}
//------------------------------------------------------------------------------------------------------------------------------------
RC createPageFile (char *fileName)
{
  FILE *f;
  f=fopen(fileName,"w+");
  if (f < 0)
    return RC_WRITE_FAILED;// Return error for failure in READ Write

  size_t i;
    for (i = 0; i < PAGE_SIZE; i++)
    {
      //fwrite('\0' , 1 , sizeof(char) , f);
     fprintf(f, "%c", '\0');
    }
    fclose(f);
    return RC_OK;
}
//------------------------------------------------------------------------------------------------------------------------------------
RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
  FILE *f;
  f = fopen(fileName, "r+");
  if (f == NULL)// Return error if opening non existing file
    return RC_FILE_NOT_FOUND;
  
  fHandle->curPagePos = 0;
  fHandle->fileName = fileName;

  if(f != NULL)
  {
    struct stat fileStat;
    if(fstat(fileno(f), &fileStat)>=0)
      fHandle->totalNumPages = fileStat.st_size/PAGE_SIZE;
    else
      return RC_READ_NON_EXISTING_PAGE;
  }
    fclose(f);
    return RC_OK;
}
//------------------------------------------------------------------------------------------------------------------------------------
RC closePageFile (SM_FileHandle *fHandle)
{
	if(!fHandle)
    	return RC_FILE_HANDLE_NOT_INIT; 				                                                           // Closing a file which is not open
  //fHandle = NULL; 									                                                                   // Reset the pointer to NULL
  return RC_OK;
}
//------------------------------------------------------------------------------------------------------------------------------------
RC destroyPageFile (char *fileName)
{
  
  if(remove(fileName) == 0)
    return RC_OK;
  else
  {
    int fileD = open(fileName, O_RDONLY); //Open the file
    if (fileD < 0)
      return RC_FILE_NOT_FOUND;
    close(fileD);
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  if(pageNum<0)                                                                                 //check whether pageNum is less than zero
    return RC_READ_NON_EXISTING_PAGE;                                                           //return error if pageNum is less than zero 
  else if (pageNum > fHandle->totalNumPages)                                                    //check whether pageNum is greater than total number of pages
    return RC_READ_NON_EXISTING_PAGE;                                                           // return error if pageNum is greater than total number of pages
  else
  {
    int file_descriptor = open(fHandle->fileName, O_RDONLY);
    if (file_descriptor < 0)                                                                    //Check whether we can open the file
      return RC_FILE_NOT_FOUND;                                                                 //return error if we cannot open the file
    int temp=lseek(file_descriptor, (PAGE_SIZE * pageNum), SEEK_SET);
    if(temp >= 0)
    {
      read(file_descriptor, memPage, PAGE_SIZE);                                                //Read the data from file_descriptor to memory(memPage)
      fHandle -> curPagePos++;                                                                  //Update the current page position(increment it by 1)
      close(file_descriptor);
      return RC_OK;
    }
    else
    {
      close(file_descriptor);
      return RC_READ_NON_EXISTING_PAGE;                                                         // return Error in reading block
    }
  }    
}
//------------------------------------------------------------------------------------------------------------------------------------
int getBlockPos (SM_FileHandle *fHandle)
{
  return fHandle->curPagePos;
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  int file_descriptor = open(fHandle -> fileName, O_RDONLY);
  if (file_descriptor < 0)                                                                      // Check whether we can open the file
    return RC_FILE_NOT_FOUND;                                                                   // return error if we cannot open the file
  
  int temp = lseek(file_descriptor, 0, SEEK_SET); 
  if(temp >= 0)
  {
    read(file_descriptor, memPage, PAGE_SIZE);                                                  // Read the data from file_descriptor to memory(memPage)
    fHandle -> curPagePos = fHandle -> curPagePos + 1 ;
    close(file_descriptor);
    return RC_OK;
  }
  else
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;                                                           // Return Error in getting to first page
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  int file_descriptor = open(fHandle->fileName, O_RDONLY);
  if (file_descriptor < 0)                                                                       // Check whether we can open the file
    return RC_FILE_NOT_FOUND;                                                                    // return error if we cannot open the file
  

  if(fHandle -> curPagePos == 0)
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;
  }
  int temp = lseek(file_descriptor, -(PAGE_SIZE), SEEK_CUR);
  if(temp >= 0)
  {
   read(file_descriptor, memPage, PAGE_SIZE);                                                   // Read the data from file_descriptor to memory(memPage)
   fHandle -> curPagePos = fHandle -> curPagePos - 1;
   close(file_descriptor);
   return RC_OK;
  }
  else
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;                                                           // Return Error in getting to previous page
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  int file_descriptor = open(fHandle -> fileName, O_RDONLY);
  
  if (file_descriptor < 0)                                                                      // Check whether we can open the file
    return RC_FILE_NOT_FOUND;                                                                   // Return error if we cannot open the file
  
  int temp=lseek(file_descriptor, 0, SEEK_CUR);
  
  if(temp >= 0)
  {
    read(file_descriptor, memPage, PAGE_SIZE);                                                  // Read the data from file_descriptor to memory(memPage)
    close(file_descriptor);
    return RC_OK;
  }
  else
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;                                                          // Return Error in getting to current page
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  int file_descriptor = open(fHandle->fileName, O_RDONLY);
  if (file_descriptor < 0)                                                                    // Check whether we can open the file
    return RC_FILE_NOT_FOUND;                                                                 // Return error if we cannot open the file
  
  if(fHandle -> curPagePos == fHandle -> totalNumPages)                                       // Check if we are on last page
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;                                                         // There is no next page, we are on the last one
  }

  int temp=lseek(file_descriptor, PAGE_SIZE, SEEK_CUR);
  if(temp >= 0)
  {
    read(file_descriptor, memPage, PAGE_SIZE);                                               // Read the data from file_descriptor to memory(memPage)
    fHandle->curPagePos ++;
    close(file_descriptor);
    return RC_OK;
  }
  else    
  {
   close(file_descriptor);
   return RC_READ_NON_EXISTING_PAGE;                                                         // Return Error in getting to next page
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  int file_descriptor = open(fHandle->fileName, O_RDONLY);
  if (file_descriptor < 0)                                                                  // Check whether we can open the file
    return RC_FILE_NOT_FOUND;                                                               // Return error if we cannot open the file
  
  int temp=lseek(file_descriptor, -PAGE_SIZE, SEEK_END);
  if(temp >= 0)
  {
   read(file_descriptor, memPage, PAGE_SIZE);                                              // Read the data from file_descriptor to memory(memPage)
   fHandle -> curPagePos = fHandle -> totalNumPages;
   close(file_descriptor);
   return RC_OK;
  }
  else
  {
    close(file_descriptor);
    return RC_READ_NON_EXISTING_PAGE;                                                     // Return Error in getting to last page
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{	
	int file_descriptor;
	if(pageNum < 0)																	                          // if the page number is non negative
    return RC_WRITE_FAILED;

  else if(pageNum > fHandle-> totalNumPages)										            // if the page number exceeds the total number of pages
  	return RC_WRITE_FAILED;

  else
  {
  	file_descriptor = open(fHandle->fileName, O_RDWR);							        //O_RDWR: read and write flag, returns the file descriptor

    if (file_descriptor < 0)													                      // if file_descriptor == -1: failure
    	return RC_FILE_NOT_FOUND;

    else if(lseek(file_descriptor, (pageNum * PAGE_SIZE), SEEK_SET) < 0)		// on failure lseek() returns -1 | setting offset = pageNum * PAGE_SIZE as write in current block 
  	{
    	close(file_descriptor);
    	return RC_READ_NON_EXISTING_PAGE; 										                // Error getting to pageNum'th page
  	}

  	else if(write(file_descriptor, memPage, PAGE_SIZE) >= 0)						    // returns -1 on failure else it will write to file
  	{
      fHandle -> curPagePos = pageNum;                                      // setting the pageNum
      close(file_descriptor);
      return RC_OK;
  	}
  
  	else																		                                 
  	{	
  		close(file_descriptor);
      return RC_WRITE_FAILED;
  	}  	
  }    	
}
//------------------------------------------------------------------------------------------------------------------------------------
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  int file_descriptor;

  if(fHandle -> curPagePos + 1 > fHandle -> totalNumPages)                      
  	return RC_WRITE_FAILED;

  else
  {
    file_descriptor = open(fHandle -> fileName, O_RDWR);                      //O_RDWR: read and write flag, returns the file descriptor
    int temp = lseek(file_descriptor, 0, SEEK_CUR);
    if (file_descriptor < 0)                                                  // if file_descriptor == -1: failure
      return RC_FILE_NOT_FOUND;                           

    else if(temp < 0)                                                         // on failure lseek() returns -1 | setting offset = 0 as write in current block 
    {
      close(file_descriptor);
      return RC_READ_NON_EXISTING_PAGE;                                                      
    }

    else if(write(file_descriptor, memPage, PAGE_SIZE) >= 0)                   // returns -1 on failure else it will write to file
    {
      fHandle -> curPagePos = fHandle -> curPagePos + 1 ;
      close(file_descriptor);
      return RC_OK;
    }

    else
    {
      close(file_descriptor);
      return RC_WRITE_FAILED;
    }
  }	
}
//------------------------------------------------------------------------------------------------------------------------------------
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
  int file_descriptor = open(fHandle->fileName, O_RDWR);                    // O_RDWR: read and write flag, returns the file descriptor
  int temp = lseek(file_descriptor, 0, SEEK_END);
  if (temp >= 0)                                                             // on failure lseek() returns -1 | setting offset = 0 as write in current block
  {
    SM_PageHandle page = (SM_PageHandle) malloc(PAGE_SIZE * sizeof(char));
  
    for(int i = 0; i <PAGE_SIZE; i++)
      page[i] = '\0';

    if(write(file_descriptor, page, PAGE_SIZE) >= 0)                         //Writing '\0' to the file
    { 
       fHandle -> totalNumPages = fHandle -> totalNumPages + 1; 
      free(page); 
      close(file_descriptor);
      return RC_OK;                                            
    }
    else
    {
      close(file_descriptor);
      free(page);
      return RC_WRITE_FAILED; 
    }    
  }  
  else
    return RC_WRITE_FAILED;
}
//------------------------------------------------------------------------------------------------------------------------------------
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{ 
  int file_descriptor = open(fHandle -> fileName, O_RDWR);                    //O_RDWR: read and write flag, returns the file descriptor

  if(file_descriptor < 0)
    return RC_FILE_NOT_FOUND;
  
  else
  {
    close(file_descriptor);                                                  //Closing RC_FILE_NOT_FOUNDdescriptor else it might be opened up in appendEmptyBlock
    while( numberOfPages > fHandle -> totalNumPages )
    {
      appendEmptyBlock(fHandle);
      numberOfPages = numberOfPages - 1;
    }    
    return RC_OK;
  }  
}

//------------------------------------------------------------------------------------------------------------------------------------