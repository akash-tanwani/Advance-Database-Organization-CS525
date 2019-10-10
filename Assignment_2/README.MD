# Assignment 2 - Buffer Manager #

The goal of this assignment is to implement a buffer manager that manages a fixed number of pages in memory that represent pages from a page file 
managed by the storage manager implemented in assignment 1. The memory pages managed by the buffer manager are called page frames or 
frames for short. We call the combination of a page file and the page frames storing pages from that file a Buffer Pool. The Buffer manager 
is able to handle more than one open buffer pool at the same time. However, there is only one buffer pool for each page file. 
Each buffer pool uses one page replacement strategy that is determined when the buffer pool is initialized. The two replacement strategies
which are implemented are FIFO and LRU.

Following is the interface implemented:

-----------------------------------------------------------------------------------------------------------------
