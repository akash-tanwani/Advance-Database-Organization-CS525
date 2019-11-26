# Assignment 4 - B+-Tree #

In this assignment we are implementing a B+-tree index. The index is backed up by a page file and pages of the index are accessed through the previously implemented buffer manager. 
As discussed in the lecture each node occupies one page. 
A B+-tree stores pointer to records (the RID introduced in the last assignment) index by a keys of a given datatype. The assignment only supports DT_INT (integer) keys.
Pointers to intermediate nodes should be represented by the page number of the page the node is stored in. 

Following is the interface implemented:

-----------------------------------------------------------------------------------------------------------------