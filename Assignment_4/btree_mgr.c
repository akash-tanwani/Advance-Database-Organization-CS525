#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "btree_mgr.h"
#include "tables.h"
#include "expr.h"

int last_page = 0;
const RID rid = {-1,-1};
int scan_pointer = 0;

typedef struct Node
{
	RID right;
	int mother;
	RID left;
	bool leaf;
	int value1;
	int value2;
	RID mid;
}Node;

typedef struct TreeInfo
{
	BM_PageHandle *page;
	BM_BufferPool *bm;
	int globalCount;
	int maxCount;
	int root;

}TreeInfo;

extern RC initIndexManager (void *mgmtData)
{
	return RC_OK;
}

extern RC shutdownIndexManager ()
{
	return RC_OK;
}

extern RC createBtree (char *idxId, DataType keyType, int n)
{
	SM_FileHandle file_handle;
	createPageFile(idxId);
	openPageFile(idxId, &file_handle);
	ensureCapacity(1, &file_handle);
	SM_PageHandle page_handle = malloc(PAGE_SIZE*sizeof(char));

	if(keyType == DT_INT)
	{
		*((int *)page_handle) = n;
		writeCurrentBlock(&file_handle,page_handle);
		closePageFile(&file_handle);
		return RC_OK;
	}
	return RC_RM_UNKOWN_DATATYPE;
}

extern RC openBtree (BTreeHandle **tree, char *idxId)
{
	TreeInfo* tree_info = malloc(sizeof(TreeInfo));
	tree_info->bm = MAKE_POOL();
	tree_info->page = MAKE_PAGE_HANDLE();
	tree_info->globalCount = 0;
	tree_info->root = 0;

	initBufferPool(tree_info->bm,idxId, 10, RS_FIFO, NULL);
	pinPage(tree_info->bm, tree_info->page,1);

	BTreeHandle* treeTemp;
	treeTemp = (BTreeHandle*)malloc(sizeof(BTreeHandle));
	treeTemp->keyType = DT_INT;
	tree_info->maxCount =* ((int *)tree_info->page->data);
	treeTemp->idxId = idxId;
	treeTemp->mgmtData = tree_info;
	*tree = treeTemp;
	unpinPage(tree_info->bm, tree_info->page);
 
	return RC_OK;
}

extern RC closeBtree (BTreeHandle *tree)
{
	last_page = 0;
	scan_pointer = 0;
	free(tree);
	free(((TreeInfo*)(tree->mgmtData))->page);
	shutdownBufferPool(((TreeInfo* )(tree->mgmtData))->bm);

	return RC_OK;
}

extern RC deleteBtree (char *idxId)
{
	if(remove(idxId) == 0)
		return RC_OK;
	return RC_FILE_NOT_FOUND; 
}

extern RC getNumNodes (BTreeHandle *tree, int *result)
{
	*result = last_page+1;
	return RC_OK;
}

extern RC getNumEntries (BTreeHandle *tree, int *result)
{
	TreeInfo* tree_info = (TreeInfo*) (tree->mgmtData);
	*result = tree_info->globalCount;
	return RC_OK;
}

extern RC getKeyType (BTreeHandle *tree, DataType *result)
{
	return RC_OK;
}

// ********************************************** index access *********************************************
extern RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
	TreeInfo* tree_info = (TreeInfo*) (tree->mgmtData);
	int i=0, val1, val2, search_key = key->v.intV;
	Node* n;
	bool f = false;

	while(i <= last_page)
	{
		pinPage(tree_info->bm, tree_info->page, i);
		n = (Node*)tree_info->page->data+sizeof(bool);
		val1 = n->value1;
		val2 = n->value2;
		if(search_key == val1)
		{
			f = true;
			*result = n->left;
			break;
		}
		if(search_key == val2)
		{
			f = true;
			*result = n->mid;
			break;
		}
		unpinPage(tree_info->bm, tree_info->page);
		i++;
	}
	if(f == false)
		return RC_IM_KEY_NOT_FOUND;
	return RC_OK;
}

extern RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
	TreeInfo* tree_info=(TreeInfo*) (tree->mgmtData);
	Node* n;

	if(last_page != 0)
	{
		pinPage(tree_info->bm, tree_info->page, last_page);
		markDirty(tree_info->bm, tree_info->page);
		
		if((*(bool*)tree_info->page->data) == false)
		{
			n=(Node*)tree_info->page->data+sizeof(bool);
			n->mid=rid;
			n->value2=key->v.intV;
			*(bool*)tree_info->page->data=true;
			unpinPage(tree_info->bm, tree_info->page);
		}
		else
		{
			last_page++;
			unpinPage(tree_info->bm, tree_info->page);
			pinPage(tree_info->bm, tree_info->page, last_page);
			*(bool*)tree_info->page->data=false;
			n=(Node*)tree_info->page->data+sizeof(bool);
			n->mother=-1;
			n->leaf=true;
			n->left=rid;
			n->value1=key->v.intV;
			n->mid=rid;
			n->value2=-1;
			n->right=rid;
			unpinPage(tree_info->bm, tree_info->page);
		}
		
	}
	else	
	{
		last_page=1;
		tree_info->root=1;
		pinPage(tree_info->bm, tree_info->page, last_page);
		markDirty(tree_info->bm, tree_info->page);	
		*(bool*)tree_info->page->data=false;
		n=(Node*)tree_info->page->data+sizeof(bool);
		n->mother=-1;
		n->leaf=true;
		n->left=rid;
		n->value1=key->v.intV;
		n->mid=rid;
		n->value2=-1;
		n->right=rid;
		unpinPage(tree_info->bm, tree_info->page);
	}
	(tree_info->globalCount)++;		
	return RC_OK;
}

extern RC deleteKey (BTreeHandle *tree, Value *key)
{
	TreeInfo* tree_info=(TreeInfo*) (tree->mgmtData);
	int i=0, val1, val2, search_key=key->v.intV;
	Node* n;
	bool f=false;
	int no_val=0;
	RID rid_m;
	int val_m;

	while(i<=last_page)
	{
		pinPage(tree_info->bm, tree_info->page, i);
		markDirty(tree_info->bm, tree_info->page);
		n=(Node*)tree_info->page->data+sizeof(bool);
		val1=n->value1;
		val2=n->value2;
		if(search_key==val1)
		{
			f=true;
			no_val=1;
			break;
		}
		if(search_key==val2)
		{
			f=true;
			no_val=2;
			break;
		}
		unpinPage(tree_info->bm, tree_info->page);
		i++;
	}

	if(f==true)
	{
		pinPage(tree_info->bm, tree_info->page, last_page);
		markDirty(tree_info->bm, tree_info->page);

		if(i==last_page)
		{
			n=(Node*)tree_info->page->data+sizeof(bool);
			if(no_val==2)
			{			
				n->mid=rid;
				n->value2=-1;
				*(bool*)tree_info->page->data=false;			
			}
			else
			{
				if((*(bool*)tree_info->page->data)==true)
				{
					rid_m=n->mid;
					n->left=rid_m;
					val_m=n->value2;
					n->value1=val_m;
					n->mid=rid;
					n->value2=-1;
					*(bool*)tree_info->page->data=false;
				}
				else
				{
					n->left=rid;
					n->value1=-1;
					last_page--;
				}
			}
			unpinPage(tree_info->bm, tree_info->page);
		}
		else
		{
			if((*(bool*)tree_info->page->data)==true)
			{
				*(bool*)tree_info->page->data=false;
				n=(Node*)tree_info->page->data+sizeof(bool);
				rid_m=n->mid;
				val_m=n->value2;
				n->mid=rid;
				n->value2=-1;
				unpinPage(tree_info->bm, tree_info->page);
				pinPage(tree_info->bm, tree_info->page, i);
				markDirty(tree_info->bm, tree_info->page);
				if(no_val!=1)
				{
					n=(Node*)tree_info->page->data+sizeof(bool);
					n->mid=rid_m;
					n->value2=val_m;
					unpinPage(tree_info->bm, tree_info->page);
				}
				else
				{
					n=(Node*)tree_info->page->data+sizeof(bool);
					n->left=rid_m;
					n->value1=val_m;
					unpinPage(tree_info->bm, tree_info->page);
				}
			}
			else
			{
				n=(Node*)tree_info->page->data+sizeof(bool);
				rid_m=n->left;
				val_m=n->value1;
				n->left=rid;
				n->value1=-1;
				last_page--;
				unpinPage(tree_info->bm, tree_info->page);
				pinPage(tree_info->bm, tree_info->page, i);
				markDirty(tree_info->bm, tree_info->page);
				if(no_val!=1)
				{
					n=(Node*)tree_info->page->data+sizeof(bool);
					n->mid=rid_m;
					n->value2=val_m;
					unpinPage(tree_info->bm, tree_info->page);
				}
				else
				{
					n=(Node*)tree_info->page->data+sizeof(bool);
					n->left=rid_m;
					n->value1=val_m;
					unpinPage(tree_info->bm, tree_info->page);
				}
			}
		}
		(tree_info->globalCount)--;
	}
	else
		return RC_IM_KEY_NOT_FOUND;

	return RC_OK;
}

extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
	TreeInfo* tree_info=(TreeInfo*) (tree->mgmtData);
	Node *n;
	int *val,val1,val2;
	int i=1,j=0,t1,t2,minimum_val;
	val=(int*)malloc(sizeof(int)*(tree_info->globalCount));

	while(i<=last_page)
	{
		pinPage(tree_info->bm, tree_info->page, i);
		n=(Node*)tree_info->page->data+sizeof(bool);
		val1=n->value1;
		val2=n->value2;
		if(val1!=-1)
		{
			val[i]=n->value1;
			i++;
		}
		if(val2!=-1)
		{
			val[i]=n->value2;
			i++;
		}
		unpinPage(tree_info->bm, tree_info->page);
		i++;
	}
	i=0;
	while(i<(tree_info->globalCount))
	{
		minimum_val=i;
		j=i+1;
		while(j<(tree_info->globalCount))
		{
			if(val[minimum_val]>val[j])
				minimum_val=j;
			j++;
		}
		t1=val[minimum_val];
		t2=val[i];
		val[minimum_val]=t2;
		val[i]=t1;
		i++;
	}

	BT_ScanHandle *handleTemp;
	handleTemp=(BT_ScanHandle*)malloc(sizeof(BT_ScanHandle));
	handleTemp->tree=tree;
	handleTemp->mgmtData=val;
	*handle=handleTemp;
	scan_pointer=0;
	return RC_OK;
}

extern RC nextEntry (BT_ScanHandle *handle, RID *result)
{
	TreeInfo* tree_info=(TreeInfo*) (handle->tree->mgmtData);
	int* val=(int*)(handle->mgmtData);
	Value* new_val;
	new_val=(Value*)malloc(sizeof(Value));
	new_val->dt=DT_INT;
	new_val->v.intV=val[scan_pointer];
	RID* final_res;
	final_res=(RID*)malloc(sizeof(RID));

	if(scan_pointer!=(tree_info->globalCount))
	{
		findKey (handle->tree, new_val, final_res);
		scan_pointer++;
	}
	else
		return RC_IM_NO_MORE_ENTRIES;
	
	*result=*final_res;
	return RC_OK;
}

extern RC closeTreeScan (BT_ScanHandle *handle)
{
	scan_pointer=0;
	free(handle->mgmtData);
	return RC_OK;
}

extern char *printTree (BTreeHandle *tree)
{
	return tree->idxId;
}
