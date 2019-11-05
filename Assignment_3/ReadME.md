# Assignment 3 - Record Manager #

The record manager handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table. A scan is associated with a search condition 
and only returns records that match the search condition. Each table should be stored in a separate page file and your record manager should access the pages of the file through the buffer manager 
implemented in the last assignment.

Following is the interface implemented:

-----------------------------------------------------------------------------------------------------------------

**- tables.h: **

This header defines basic data structures for schemas, tables, records, record ids (```RIDs```), and values. Furthermore, this header defines functions for serializing these data structures as strings. 
The serialization functions are provided (```rm_serializer.c```). There are four datatypes that can be used for records of a table: integer (```DT_INT```), float (```DT_FLOAT```), strings of a fixed length (```DT_STRING```), 
and boolean (```DT_BOOL```). All records in a table conform to a common schema defined for this table. 
A record is simply a record id (rid consisting of a page number and slot number) and the concatenation of the binary representation of its attributes according to the schema (data).

**- expr.h: **

This header defines data structures and functions to deal with expressions for scans. These functions are implemented in ```expr.c```. Expressions can either be constants (stored as a Value struct), references to 
attribute values (represented as the position of an attribute in the schema), and operator invocations. Operators are either comparison operators (equals and smaller) that are defined for all data types and 
boolean operators AND, OR, and NOT. Operators have one or more expressions as input. The expression framework allows for arbitrary nesting of operators as long as their input types are correct.

**- record_mgr.h: **

There are five types of functions in the record manager: functions for table and record manager management, functions for handling the records in a table, functions related to scans, functions for dealing with 
schemas, and function for dealing with attribute values and creating records. 

*  	**Table and Record Manager Methods:** It contains functions such as create, delete, open and close table. Storage Manager from Assignment 1 and Buffer Manager from Assignment 2 are used in order to access pages and perform operations respectively. Following functions are used.
	*	```initRecordManager()```: It is used to initialize Record Manager.
	*	```shutdownRecordManager()```: It is used to shutdown Record Manager.
	*	```createTable()```: It is used to create and open table according to the specified schema.
	*	```openTable()```: It is used to initialize the bufferpool using initBufferPool() function.
	*	```closeTable()```: It is used to close the table using shutdownBufferPool() function and free the space from the buffer pool.
	*	```deleteTable()```: It is used to delete the table name, page and deallocate memory using destroyPageFile() function.
	*	```getNumTuples()```: It is used to retrieve the number of records in the table. 
*  	**Record Methods:** It is used to perform manipulation operations on records as in insert, update, delete records. 
	*	```insertRecord()```: It is used to insert record in the table based on the Record ID. Page with an empty slot is pinned and marked as dirty. Data is copied to new record and page is unpinned.
	*	```updateRecord()```: It is used to update the record from the table. Data is copied to a new record and space is updated using ```pinpage()``` function.
	*	```deleteRecord()```: It is used to delete a record from the table based on the specified Record ID.
	*	```getRecord()```: It is used to retrieve the record from the table based on the record ID and value is assigned to the record. Size of the record is fetched by creating pointers.
*  	**Scan Methods:** It is used to retrieve records from the table.
	*	```startScan()```: The data is scanned using ```startScan()``` function by passing an argument as ```RM_ScanHandle``` to this function. Auxiliary scan values are fetched and assigned to scan.
	*	```closeScan()```: It is used to close the scan operation.
	*	```next()```: Records are scanned until the specified record is found. The record list is iterated for finding the required record. The page containing the tuple is pinned and all the tuples are scanned for the specified record. After scanning the entire page, it is unpinned.
*	**Schema Methods:** It is used to create schema, retrieve record size and free space allocated to the schema.
	*	```createSchema()```: It is used to create new schema.
	*	```freeSchema()```: It is used to free up the space allocated to schema.
	*	```getRecordSize()```: It is used to retrieve the record size mentioned in the schema.
*	**Attribute Methods:** It is used to create record and get/set attribute of record.
	*	```createRecord()```: It is used to create a new record, allocate memory to the record and retrieve the size of the record.
	*	```freeRecord():``` It is used to free up space allocated to the record.
	*	```getAttr():``` It is used to retrieve the attribute value from the record.
	*	````setAttr()```: It is used to set the attribute value to the record.
-----------------------------------------------------------------------------------------------------------------

## How to Run: ##
Step 1: Open Command Promt/PowerShell/Terminal.

Step 2: Navigate to the Assignment 3 directory.

Step 3: Type command ```make``` and hit enter. (Files are complied and ready to be executed)

Step 4: Type command ```make run``` if you are using a Windows machine

Step 5: Type command ```make clean``` for clean up

-----------------------------------------------------------------------------------------------------------------
## Contibutions: ##

**Chirag Khandhar : **
Record methods,```dberror```, ```buffer_list```, ```record_scan```  and related documentation

**Akash Tanwani : **
Schema,  Scan, Attribute, test methods and related documentation

**Gandhali Khedlekar : ** 
Table and Manager functions and related documentation

-----------------------------------------------------------------------------------------------------------------

