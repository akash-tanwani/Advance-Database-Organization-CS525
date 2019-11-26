# Assignment 4 - B+-Tree #

In this assignment we are implementing a B+-tree index. The index is backed up by a page file and pages of the index are accessed through the previously implemented buffer manager. 
As discussed in the lecture each node occupies one page. 
A B+-tree stores pointer to records (the RID introduced in the last assignment) index by a keys of a given datatype. The assignment only supports DT_INT (integer) keys.
Pointers to intermediate nodes should be represented by the page number of the page the node is stored in. 

This Assignment is  built on top of the previous assignment **Record Manager**.

-----------------------------------------------------------------------------------------------------------------

**- Index Manager Functions: **

It is used to initialize and shut down index manager. It is also used to deallocate the resources.

**- B+ Tree Functions: **

*	```createBtree()``` It is used to create B+ tree index.
*	```openBtree()``` It is used to open B+ tree index.
*	```closeBtree()``` It is used to close B+ tree index.
*	```deleteBtree()``` It is used to delete B+ tree index. It also removes the corresponding page.

**- Debug Functions: **

*	```printTree()``` It is used to create string representation of B+ tree. It is used for debugging. 

**- Key Functions: **


*	```findKey()``` It is used to find the key in the B+ tree. It returns RID for the entry along with the search key.
*	```insertKey()``` It is used to insert a new key and record pointer pair into the index.
*	```deleteKey()``` It is used to remove the key and the corresponding record pointer from the index.
*	```openTreeScan(), closeTreeScan()``` These functions are used to scan all the entries of B+ tree.

-----------------------------------------------------------------------------------------------------------------

## How to Run: ##
Step 1: Open Command Promt/PowerShell/Terminal.

Step 2: Navigate to the Assignment 4 directory.

Step 3: Type command ```make``` and hit enter. (Files are complied and ready to be executed)

Step 4: Type command ```make run1``` this will execute test_assign4.c file

Step 5: Type command ```make run2``` this will execute test_expr.c file

Step 6: Type command ```make clean``` for clean up

-----------------------------------------------------------------------------------------------------------------
## Contibutions: ##

**Chirag Khandhar : **
Index Access related functions and related documentation

**Akash Tanwani : **
Create, Destroy, Open, and Close functions and related documentation

**Gandhali Khedlekar : ** 
Access information functions, init and shutdown index manager and related documentation

-----------------------------------------------------------------------------------------------------------------

