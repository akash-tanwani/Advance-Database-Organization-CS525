#include "storage_mgr.h"
#include "record_scan.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static Schema global_schema;
static PTR_Scan scan_pointer = NULL;

RC initRecordManager(void *mgmtData)
{
  return RC_OK;
}

RC shutdownRecordManager()
{
  return RC_OK;
}

RC createTable (char *name, Schema *schema)
{
  char local_fname[64] = {'\0'};
  strcat(local_fname,name);
  strcat(local_fname,".bin");
  createPageFile(local_fname);
  global_schema = *schema;
  return RC_OK;
}

RC openTable (RM_TableData *rel, char *name)
{
  char local_fname[64] = {'\0'};
  Schema *local_schema = (Schema *)malloc(sizeof(Schema));
  BM_BufferPool *buffer_pool = MAKE_POOL();
  strcat(local_fname,name);
  strcat(local_fname,".bin");
  initBufferPool(buffer_pool,local_fname,4,RS_FIFO,NULL);
  rel->name = name;
  rel->mgmtData = buffer_pool;  
  *local_schema = global_schema;
  rel->schema = local_schema;
  return RC_OK;
}

RC closeTable (RM_TableData *rel)
{
  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  shutdownBufferPool(buffer_pool);
  free(buffer_pool);
  return RC_OK;
}

RC deleteTable (char *name)
{
  char local_fname[50] = {'\0'};
  strcat(local_fname,name);
  strcat(local_fname,".bin");
  destroyPageFile(local_fname);
  return RC_OK;
}

int getNumTuples (RM_TableData *rel)
{
  int tuple_count = 0;
  PageNumber page_number = 1;
  BM_PageHandle *pagehandle = MAKE_PAGE_HANDLE();
  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  SM_FileHandle *file_handle = (SM_FileHandle *)buffer_pool->mgmtData;
  PageNumber numPages = file_handle->totalNumPages;
  while(page_number < numPages)
  {
	 pinPage(buffer_pool,pagehandle,page_number);
	 page_number++;
	 int i = 0;
	 while(i < PAGE_SIZE)
	 {
		if(tuple_count=pagehandle->data[i] == '-')
		  tuple_count++;
		i++;
	 }
  }
  return tuple_count;
}

RC insertRecord (RM_TableData *rel, Record *record)
{
  int slot_number = 0, page_length,total_rec_length;
  char *sp = NULL;
  RID id;
	 
  PageNumber page_number;
  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  BM_PageHandle *page_handle = MAKE_PAGE_HANDLE();
  SM_FileHandle *sm_handle = (SM_FileHandle *)buffer_pool->mgmtData;
  page_number=1;
  
  total_rec_length = getRecordSize(rel->schema);
  while(page_number < sm_handle->totalNumPages)
  {
		pinPage(buffer_pool,page_handle,page_number);
		page_length = strlen(page_handle->data);

		if(PAGE_SIZE-page_length > total_rec_length)
		{
			 slot_number = page_length/total_rec_length;
			 unpinPage(buffer_pool,page_handle);
			 break;
		}
		unpinPage(buffer_pool,page_handle);
		page_number++;
  }
  if(slot_number == 0)
  {
	 pinPage(buffer_pool,page_handle,page_number + 1);
	 unpinPage(buffer_pool,page_handle);
  }
  pinPage(buffer_pool,page_handle,page_number);
  sp = page_handle->data + strlen(page_handle->data);
  
  strcpy(sp,record->data);
  markDirty(buffer_pool,page_handle);
  unpinPage(buffer_pool,page_handle);
  id.page = page_number;
  id.slot = slot_number;
  record->id = id;
  return RC_OK;
}

RC deleteRecord (RM_TableData *rel, RID id)
{
  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  BM_PageHandle *page_handle = MAKE_PAGE_HANDLE();
  PageNumber page_number = id.page; 
  pinPage(buffer_pool,page_handle,page_number);
  markDirty(buffer_pool,page_handle);
  unpinPage(buffer_pool,page_handle);
  free(page_handle);
  return RC_OK;
}

RC updateRecord (RM_TableData *rel, Record *record)
{
  char *sp;
  int record_length;
  
  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  BM_PageHandle *page_handle = MAKE_PAGE_HANDLE();
  RID id = record->id;
  int slot_number = id.slot;  
  PageNumber page_number = id.page;
  
  record_length = getRecordSize(rel->schema);
  pinPage(buffer_pool,page_handle,page_number);
 
  sp = page_handle->data + record_length * slot_number;
  strncpy(sp,record->data,record_length);
  return RC_OK;
}

RC getRecord (RM_TableData *rel, RID id, Record *record)
{
  int slot_number = id.slot,record_length;
  char *sp;

  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  BM_PageHandle *page_handle = MAKE_PAGE_HANDLE();
  
  PageNumber page_number = id.page;
  record_length = getRecordSize(rel->schema);
  pinPage(buffer_pool,page_handle,page_number);
  sp = page_handle->data + record_length * slot_number;
  strncpy(record->data,sp,record_length);
  unpinPage(buffer_pool,page_handle);
  return RC_OK;
}

RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{

  BM_BufferPool *buffer_pool = (BM_BufferPool *)rel->mgmtData;
  SM_FileHandle *file_handle = (SM_FileHandle *)buffer_pool->mgmtData;
  
  AUX_Scan *aux_scan=(AUX_Scan *)malloc(sizeof(AUX_Scan));
 
  scan->mgmtData = cond;
  scan->rel = rel;
  
  aux_scan->_slotID = 1;
  aux_scan->_sPage = 1;
  aux_scan->_numPages = file_handle->totalNumPages;
  aux_scan->pHandle = MAKE_PAGE_HANDLE();
  aux_scan->_recLength = getRecordSize(rel->schema);
 
  return insert(scan,&scan_pointer,aux_scan);
}

RC next (RM_ScanHandle *scan, Record *record)
{
  RID id;
  Expr *exp = (Expr *)scan->mgmtData,*len,*rec,*aux_expression;
  RM_TableData *rm_data = scan->rel;
  Operator *res,*new_res;
  AUX_Scan *AUX_Scan = search(scan,scan_pointer);
  
  Value **cValue = (Value **)malloc(sizeof(Value *));
  *cValue = NULL;  
  res = exp->expr.op;
  
  switch(res->type)
  {
	 case OP_COMP_SMALLER: 
	 {
		len = res->args[0];
		rec = res->args[1];
		while(AUX_Scan->_sPage < AUX_Scan->_numPages)
		{
		  pinPage(rm_data->mgmtData,AUX_Scan->pHandle,AUX_Scan->_sPage);
		  AUX_Scan->_recsPage = strlen(AUX_Scan->pHandle->data) / AUX_Scan->_recLength;
		  while(AUX_Scan->_slotID < AUX_Scan->_recsPage)
		  {
			 id.slot = AUX_Scan->_slotID;
			 id.page = AUX_Scan->_sPage;
		  
			 getRecord(rm_data,id,record);
			 getAttr(record,rm_data->schema,rec->expr.attrRef,cValue); 
			 if((rm_data->schema->dataTypes[rec->expr.attrRef] == DT_INT) && (len->expr.cons->v.intV>cValue[0]->v.intV)) 
			 {
				AUX_Scan->_slotID++;
				unpinPage(rm_data->mgmtData,AUX_Scan->pHandle);
				return RC_OK;
			 }
			 AUX_Scan->_slotID++;
		  }
		  break;
		}
		break;
	 }
	 case OP_COMP_EQUAL:
	 {
		len = res->args[0];
		rec = res->args[1];
		while(AUX_Scan->_sPage < AUX_Scan->_numPages)
		{
		  pinPage(rm_data->mgmtData,AUX_Scan->pHandle,AUX_Scan->_sPage);
		  AUX_Scan->_recsPage = strlen(AUX_Scan->pHandle->data) / AUX_Scan->_recLength;
		  while(AUX_Scan->_slotID < AUX_Scan->_recsPage)
		  {
			 id.page = AUX_Scan->_sPage;
			 id.slot = AUX_Scan->_slotID;
		  
			 getRecord(rm_data,id,record);
			 getAttr(record,rm_data->schema,rec->expr.attrRef,cValue);
			 if((rm_data->schema->dataTypes[rec->expr.attrRef] == DT_STRING) && (strcmp(cValue[0]->v.stringV , len->expr.cons->v.stringV) == 0))
			 {
				AUX_Scan->_slotID++;
				unpinPage(rm_data->mgmtData,AUX_Scan->pHandle);
				return RC_OK;
			 } 
			 else if((rm_data->schema->dataTypes[rec->expr.attrRef] == DT_INT)&&(cValue[0]->v.intV == len->expr.cons->v.intV))
			 {
				AUX_Scan->_slotID++;
				unpinPage(rm_data->mgmtData,AUX_Scan->pHandle);
				return RC_OK;
			 } 
			 else if((cValue[0]->v.floatV == len->expr.cons->v.floatV) && (rm_data->schema->dataTypes[rec->expr.attrRef] == DT_FLOAT))
			 {
				AUX_Scan->_slotID++;
				unpinPage(rm_data->mgmtData,AUX_Scan->pHandle);
				return RC_OK;
			 } 
			 AUX_Scan->_slotID++;
		  }
		  break;     
		}
		break;
	 }
	 case OP_BOOL_NOT:
	 {
		aux_expression = exp->expr.op->args[0];
		new_res = aux_expression->expr.op;
		rec = new_res->args[0];
		len = new_res->args[1];
		if (new_res->type == OP_COMP_SMALLER)
		{
		  while(AUX_Scan->_numPages > AUX_Scan->_sPage)
		  {
			 pinPage(rm_data->mgmtData,AUX_Scan->pHandle,AUX_Scan->_sPage);
			 AUX_Scan->_recsPage=strlen(AUX_Scan->pHandle->data) / AUX_Scan->_recLength;
			 while(AUX_Scan->_slotID < AUX_Scan->_recsPage)
			 {
				id.slot = AUX_Scan->_slotID;
				id.page = AUX_Scan->_sPage;
				getRecord(rm_data,id,record);
				getAttr(record,rm_data->schema,rec->expr.attrRef,cValue);
				if((cValue[0]->v.intV > len->expr.cons->v.intV) && (rm_data->schema->dataTypes[rec->expr.attrRef] == DT_INT))
				{
				  AUX_Scan->_slotID++;
				  unpinPage(rm_data->mgmtData,AUX_Scan->pHandle);
				  return RC_OK; 
				}
				AUX_Scan->_slotID++;
			 }
			 break; 
		  }
		  break;
		}
		break;
	 }
	 case OP_BOOL_AND:
	 case OP_BOOL_OR:
		break;
  }
  return RC_RM_NO_MORE_TUPLES;
}

RC closeScan (RM_ScanHandle *scan)
{
  search(scan,scan_pointer);
  return delete(scan,&scan_pointer);
}

int getRecordSize(Schema *schema) 
{
  int *t_length = schema->typeLength,sz = 0,i;
  for (i = 0; i < schema->numAttr; i++) 
  {
	 switch(schema->dataTypes[i])
	 {
		case DT_INT: 
		sz = sz + sizeof(int); 
		break;

		case DT_FLOAT: 
		sz  = sz + sizeof(float); 
		break;

		case DT_BOOL: 
		sz = sz + sizeof(bool); 
		break;

		case DT_STRING: 
		sz = sz + t_length[i]; 
		break;
	 }
  }
  return sz+schema->numAttr;
}

Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,int *typeLength, int keySize, int *keys)
{
  Schema *sch = (Schema*) malloc(sizeof(Schema));
  
  sch->numAttr = numAttr;
  sch->attrNames = attrNames;
  sch->dataTypes = dataTypes;
  sch->typeLength = typeLength;
  sch->keySize = keySize;
  sch->keyAttrs = keys; 
  return sch;
}

RC freeSchema(Schema *schema)
{
	 free(schema);
	 return (RC_OK);
}

RC createRecord(Record **record, Schema *schema) 
{
  Record *rec = (Record*) malloc(sizeof(Record));
  int sz = getRecordSize(schema);
  rec->data = (char*) malloc(sz);
  memset(rec->data, 0, sz);
  *record = rec;
  return (RC_OK);
}

RC freeRecord(Record *record)
{
  free(record);
  return RC_OK;
}

RC getAttr(Record *record, Schema *schema, int attrNum, Value **value)
{
  DataType *data_type = schema->dataTypes;
  int *t_length = schema->typeLength,ofst = 0,i=0;
  char *dt = record->data;
  Value *vl = (Value*) malloc(sizeof(Value));

  while(i < attrNum) 
  {
	 switch(data_type[i])
	 {
		case DT_INT: 
		ofst = ofst + sizeof(int); 
		break;

		case DT_FLOAT: 
		ofst = ofst + sizeof(float); 
		break;

		case DT_BOOL: 
		ofst = ofst + sizeof(bool); 
		break;

		case DT_STRING: 
		ofst = ofst + t_length[i]; 
		break;
	 }
	 i++;
  }
  
  ofst = ofst + (attrNum+1);
  char *tmp = NULL;  
 
  switch(data_type[i])
  {
	 case DT_INT:
		  vl->dt = DT_INT;
		  tmp = malloc(sizeof(int)+1);
		  strncpy(tmp, dt + ofst, sizeof(int)); 
		  tmp[sizeof(int)]='\0';
		  vl->v.intV = atoi(tmp);
		  break;

	 case DT_FLOAT:
		  vl->dt = DT_FLOAT;
		  tmp = malloc(sizeof(float)+1);
		  strncpy(tmp, dt + ofst, sizeof(float)); 
		  tmp[sizeof(float)] = '\0'; 
		  vl->v.floatV = (float) *tmp;
		  break;

	 case DT_BOOL:
		  vl->dt = DT_BOOL;
		  tmp=malloc(sizeof(bool)+1);
		  strncpy(tmp, dt + ofst, sizeof(bool)); 
		  tmp[sizeof(bool)] = '\0';
		  vl->v.boolV = (bool) *tmp;
		  break;

	 case DT_STRING:
		  vl->dt = DT_STRING;
		  int sz = t_length[i];
		  tmp = malloc(sz+1);
		  strncpy(tmp, dt + ofst, sz); 
		  tmp[sz] = '\0';
		  vl->v.stringV = tmp;
		  break;
  }
  *value = vl;
  return RC_OK;
}

RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) 
{
  int j, tmp, i=0, ofst=0, *t_length = schema->typeLength;
  DataType *data_type = schema->dataTypes;
  
  while(i < attrNum) 
  {
	 switch(data_type[i])
	 {
		case DT_INT: 
		ofst = ofst + sizeof(int); 
		break;

		case DT_FLOAT: 
		ofst = ofst + sizeof(float); 
		break;

		case DT_BOOL: 
		ofst = ofst + sizeof(bool); 
		break;

		case DT_STRING: 
		ofst = ofst + t_length[i]; 
		break;
	 }
	 i++;
  }
  ofst = ofst + (attrNum + 1);
  char *record_ofst = record->data;
  if(attrNum != 0)
	 {
		  record_ofst += ofst;
		  (record_ofst - 1)[0] = ' ';
	 }
	 else
	 {
		  record_ofst[0] = '-';
		  record_ofst++;
	 } 
 
  switch(value->dt)
  {
	 case DT_INT:
		  sprintf(record_ofst,"%d",value->v.intV);
		  while(strlen(record_ofst)<sizeof(int)) strcat(record_ofst,"0");
		  for (j = strlen(record_ofst)-1,i = 0; i < j;i++,j--)
		  {
				tmp = record_ofst[i]; record_ofst[i]=record_ofst[j];
				record_ofst[j] = tmp;
		  }
		  break;

	 case DT_FLOAT:
		  sprintf(record_ofst,"%f",value->v.floatV);
		  while(strlen(record_ofst) != sizeof(float)) strcat(record_ofst,"0");
		  for (j = strlen(record_ofst)-1,i = 0; i < j; i++,j--)
		  {
			tmp = record_ofst[i]; record_ofst[i]=record_ofst[j];
			record_ofst[j] = tmp;
		  }
		  break;
				 
	 case DT_BOOL: 
		  sprintf(record_ofst,"%i",value->v.boolV);
		  break;
			
	 case DT_STRING: 
		  sprintf(record_ofst,"%s",value->v.stringV);
		  break;
  }
  return RC_OK;
}