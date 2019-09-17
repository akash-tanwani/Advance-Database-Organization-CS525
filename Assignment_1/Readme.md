# Assignment 1 - Storage Manager #

The goal of this assignment is to implement a simple storage manager - a module that is capable of reading blocks from a file on disk into 
memory and writing blocks from memory to a file on disk. The storage manager deals with pages (blocks) of fixed size (PAGE_SIZE = 4096 bytes). 
In addition to reading and writing pages from a file, it provides methods for creating, opening, and closing files. 
The storage manager maintains several types of information for an open file: The number of total pages in the file, the current page position 
(for reading and writing), the file name, and a POSIX file descriptor or FILE pointer.

Following is the interface implemented:

-----------------------------------------------------------------------------------------------------------------
### Page Related Methods ###

**- initStorageManager: **
This method is called to initialize the storage manager.

**- createPageFile: **
This method creates the given filename of default page size ( i.e. 4096 bytes) using function called fopen() and writes '\0' in it.

**- openPageFile: **
This method opens the recently created file and returns RC_FILE_NOT_FOUND if the file does not exist else returns RC_OK.

**- closePageFile: **
This method closes the opened file. The parameter used is fhandle.

**- destroyPageFile: **
This method destroys the file using remove() also a check is applied if a file is tried to open after deletion, it gives appropriate error.

-----------------------------------------------------------------------------------------------------------------
### Read Related Methods ###

**- readBlock: **
This method reads the pageNum'th block from a file & stores its content in the memory pointed to by the memPage page handle. 
If the file has less than pageNum pages, the method return RC_READ_NON_EXISTING_PAGE. lseek() is used to set the offset to the pointer. (PAGE_SIZE * pageNum)

**- getBlockPos: **
This method returns the current page position.

**- readFirstBlock: **
This method is used to read the first block. In lseek() the offset is set to 0 and whence parameter is set to SEEK_SET.

**- readPreviousBlock: **
This method reads the previous page relative to the curPagePos of the file. In lseek() offset is set to -PAGE_SIZE.

**- readCurrentBlock: **
This method reads the current page relative to the curPagePos of the file. In lseek() offset is set to 0 and whence parameter is set to SEEK_CUR.

**- readNextBlock: **
This method reads the next page relative to the curPagePos of the file. In lseek() offset is set to PAGE_SIZE.

**- readLastBlock: **
This method is used to read the last block. In lseek() the offset is set to -PAGE_SIZE and whence parameter is set to SEEK_END.

-----------------------------------------------------------------------------------------------------------------
### Write Related Methods ###

**- writeBlock: **
This method writes a page to disk using the given position of *pageNUM*. lseek() is used to set the offset to the pointer. (PAGE_SIZE * pageNum)

**- writeCurrentBlock: **
This method writes the current page to the disk. In lseek(), offset is set to 0 and whence is set to SEEK_CUR.

**- appendEmptyBlock: **
This method increases the number of pages in the file by adding one page at the end of the file. The new last page is filled with '\0'. 
In lseek() offset is set to 0 and whence is set to SEEK_END.

**- ensureCapacity: **
This method ensures that if the file has less than *numberOfPages pages* then increase the size to *numberOfPages*.

-----------------------------------------------------------------------------------------------------------------

## How to Run: ##
Step 1: Open Command Promt/PowerShell/Terminal.

Step 2: Navigate to the Assignment 1 directory.

Step 3: Type command ```make``` and hit enter. (Files are complied and ready to be executed)

Step 4: Type command ```./test_assign1``` if you are using a Windows machine

-----------------------------------------------------------------------------------------------------------------

## Contibutions: ##

Page Related Methods: ***Gandhali Khedlekar***

Read Realted Methods: ***Akash Tanwani***

Write Related Methods and Makefile: ***Chirag Khandhar***

-----------------------------------------------------------------------------------------------------------------