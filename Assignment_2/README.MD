# Assignment 2 - Buffer Manager #

The goal of this assignment is to implement a buffer manager that manages a fixed number of pages in memory that represent pages from a page file 
managed by the storage manager implemented in assignment 1. The memory pages managed by the buffer manager are called page frames or 
frames for short. We call the combination of a page file and the page frames storing pages from that file a Buffer Pool. The Buffer manager 
is able to handle more than one open buffer pool at the same time. However, there is only one buffer pool for each page file. 
Each buffer pool uses one page replacement strategy that is determined when the buffer pool is initialized. The two replacement strategies
which are implemented are FIFO and LRU.

Following is the interface implemented:

-----------------------------------------------------------------------------------------------------------------
### Buffer Pool Methods ###

**- initStorageManager: **
This method creates a new buffer pool with numPages page frames using the page 
replacement strategy strategy. The pool is used to cache pages from the page file 
with name pageFileName. Initially, all page frames are empty. The page file 
already exist, i.e., this method does not generate a new page file. stratData 
can be used to pass parameters for the page replacement strategy. 

**- shutdownBufferPool: **
This method destroys the buffer pool and prior to destroying the buffer pool, 
```forceFlushPool()``` is invoked to write back all the modified pages(dirty pages) to the disk.
Also the memory allocated to the buffer pool will be released by this method. The core logic of this method is written in ```freePagesForBuffer()```.

**- forceFlushPool: **
This method writes all the dirty pages to the disk.
Also it examines whether both conditions dirtyB = 1 i.e., content of the page frame is 
modified by some client and pin = 0 i.e., no user is using that page Frame are 
satisfied then method writes the page frame back to the page file on disk.

-----------------------------------------------------------------------------------------------------------------
### Page Management Methods ###

**- pinPage: **
This method pins the page with page number ```pageNum```. The buffer manager is responsible to set the ```pageNum``` field of the page handle 
passed to the method. Similarly, the data field points to the page frame the page is stored in (the area in memory storing the content of the page).

**- unpinPage: **
This method unpins the page. The ```pageNum``` field of page is used to figure out which page to unpin.

**- makeDirty: **
This method marks a page as dirty, i.e, if the page is modified the dirty bit is set to 1.

**- forcePage: **
This method writes the current content of the page back to the page file on disk by finding that specified page using ```pageNum```.

-----------------------------------------------------------------------------------------------------------------
### Statistics Methods ###

**- getFrameContents : **
This method returns an array of PageNumbers (of size ```numPages```) where the ith element is the number of the page stored in the ith page frame. 
An empty page frame is represented using the constant ```NO_PAGE```.

**- getDirtyFlags : **
This method returns an array of bools (of size ```numPages```) where the ith element is ```TRUE``` if the page stored in the ith page frame is dirty. 
Empty page frames are considered as clean.

**- getFixCounts : **
This method  returns an array of integers (of size ```numPages```) where the ith element is the fix count of the page stored in the ith page frame. 
Return 0 for empty page frames.

**- getNumReadIO : **
This method returns the number of pages that have been read from disk since a buffer pool has been initialized.

**- getNumWriteIO : **
This method returns the number of pages that have been wriiten to the page file since a buffer pool has been initialized.

-----------------------------------------------------------------------------------------------------------------
### Custom Methods ###

**- strategyForFIFOandLRU : **
This method implement the FIFO and LRU algorithms.

**- getAttributionArray : **
This  method returns an attribution array with is used in ```strategyForFIFOandLRU()```.

**- freePagesForBuffer : **
The method is used in ```shutdownBufferPool()``` which actually destroys the buffer pool.

**- updateBfrAtrbt : **
This method is used in ```pinPage()``` to upadte the buffer attribute.

-----------------------------------------------------------------------------------------------------------------

## How to Run: ##
Step 1: Open Command Promt/PowerShell/Terminal.

Step 2: Navigate to the Assignment 2 directory.

Step 3: Type command ```make``` and hit enter. (Files are complied and ready to be executed)

Step 4: Type command ```make run``` if you are using a Windows machine

-----------------------------------------------------------------------------------------------------------------

## Contibutions: ##

**Chirag Khandhar : **
```pinPage()```, ```strategyForFIFOandLRU()```, ```getAttributionArray()```, ```updateBfrAtrbt()```, ```unpinPage()``` and Updating ```buffer_mgr.h```.

**Akash Tanwani : **
```initBufferPool()```, ```shutdownBufferPool()```, ```forceFlushPool()```, ```freePagesForBuffer()```, ```markDirty()``` and ```forcePage ()```. 

**Gandhali Khedlekar : ** ```getFrameContents()```,```getDirtyFlags ```,```getFixCounts()```, ```getNumReadIO()``` and ```getNumWriteIO()```.






-----------------------------------------------------------------------------------------------------------------
