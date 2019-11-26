#include <time.h>
#include <stdio.h>

#include "dberror.h"
#include "optional.h"
#include "test_helper.h"
#include "record_mgr.h"
#include "tables.h"

static RC doLoadCompleteWork1 (double *t, long *inputOutput);
static RC doLoadSingleWork (int dbsize, int requestsNumber, int numPages, double *t, long *inputOutput);

static RC doLoadCompleteWork2 (double *t, long *inputOutput);
static RC doLoadSingleWork2 (int dbsize, int requestsNumber, int numPages, int insertPerc, int scanFreq, double *t, long *inputOutput);

// helpers
static RC execScan (RM_TableData *table, Expr *sel, Schema *schema);

static Schema *schema1 ();
static Record *firstRecord(Schema *schema, int a, char *b, int c);

static char *randomString (int length);
static int randomInt (int min, int max);

char *testName = "OPTIONAL";

int main (void) {
  double getCurrentTime = 0;
  long inputOutput = 0;

  TEST_CHECK(doLoadCompleteWork1(&getCurrentTime, &inputOutput));
  printf("\n-------------------------------\n-- COMPLETE WORKLOAD FIRST - TOTAL: %f sec, I/Os: %ld\n\n", getCurrentTime, inputOutput);

  getCurrentTime = 0;
  inputOutput = 0;
  TEST_CHECK(doLoadCompleteWork2(&getCurrentTime, &inputOutput));
  printf("\n-------------------------------\n-- COMPLETEWORKLOAD TWO - TOTAL: %f, I/Os: %ld\n\n", getCurrentTime, inputOutput);

  return 0;
}

RC doLoadCompleteWork1(double *t, long *inputOutput) 
{
  TEST_CHECK(doLoadSingleWork (1000, 10000, 10000, t, inputOutput));
  return RC_OK;
}

RC doLoadSingleWork (int size, int requestsNumber, int numPages, double *t, long *inputOutput) 
{
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
  Schema *schema = schema1();
  time_t startTime, measureEndTime;
  Record *r;
  int i;
  Expr *sel, *left, *right;
  // set up
  srand(0);
  TEST_CHECK(initiateOptional(numPages));

  // create table and insert data
  TEST_CHECK(createTable("test_table_r",schema));
  TEST_CHECK(openTable(table, "test_table_r"));
  
  // insert rows into table
  for(i = 0; i < size; i++)
    {
      r = firstRecord(schema, i, randomString(4), randomInt(0, size / 100));
      TEST_CHECK(insertRecord(table,r));
      freeRecord(r);
    }

  // get start time
  startTime = time(NULL);

  // read requests
  for (i = 0; i < requestsNumber; i++)
  {    
    if (rand() % 10 == 9)
    {
      Value *c;
      MAKE_VALUE(c, DT_INT, rand() % (size / 100));
      MAKE_CONS(left, c);
      MAKE_ATTRREF(right, 2);
      MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);

      TEST_CHECK(execScan(table, sel, schema));
      
      freeExpr(sel);
      printf ("(");
	 }
    else
    {
      Value *c;
      MAKE_VALUE(c, DT_INT, rand() % size);
      MAKE_CONS(left, c);
      MAKE_ATTRREF(right, 0);
      MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);

      TEST_CHECK(execScan(table, sel, schema));
      
      freeExpr(sel);
      printf ("$");
	  }
  }

  // measure time
  measureEndTime = time(NULL);
  (*t) += difftime(measureEndTime,startTime);
  (*inputOutput) += getOptionalInputOutput();
  printf("\nWORKLOAD FIRST - N(R)=<%i>, #scans=<%i>, M=<%i>: %f sec, %ld I/Os\n", size, requestsNumber, numPages, *t, *inputOutput);
  
  // clean up
  TEST_CHECK(closeTable(table));
  TEST_CHECK(deleteTable("test_table_r"));
  TEST_CHECK(shutOptional());

  free(table);

  return RC_OK;
}

RC execScan (RM_TableData *table, Expr *sel, Schema *schema)
{
  RM_ScanHandle *sc = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
  Record *r;
  RC rc = RC_OK;
  
  r = firstRecord(schema, -1,"kkkk", -1);
  TEST_CHECK(startScan(table, sc, sel));
  while((rc = next(sc, r)) == RC_OK)
    ;
  if (rc != RC_RM_NO_MORE_TUPLES)
    TEST_CHECK(rc);

  TEST_CHECK(closeScan(sc));
  
  free(sc);
  freeRecord(r);
  
  return RC_OK;
}


RC doLoadCompleteWork2(double *t, long *inputOutput)
{
  // 70 - inserts, every 100th operation is a scan
  TEST_CHECK(doLoadSingleWork2 (10000, 10000, 100, 70, 100, t, inputOutput));
  
  return RC_OK;
}

RC doLoadSingleWork2 (int size, int requestsNumber, int numPages, int percInserts, int scanFreq, double *t, long *inputOutput)
{
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
  Schema *schema = schema1();
  time_t startTime, measureEndTime;
  Record *r;
  int i, inserted;
  Expr *sel, *left, *right;
  RID *rids = (RID *) malloc(sizeof(RID) * (requestsNumber + size));
  // set up
  srand(0);
  TEST_CHECK(initiateOptional(numPages));
  for(i = 0; i < (requestsNumber + size); i++)
    {
      rids[i].page = -1;
      rids[i].slot = -1;
    }

  // create table and insert data
  TEST_CHECK(createTable("test_table_r",schema));
  TEST_CHECK(openTable(table, "test_table_r"));
  
  // insert rows into table
  for(inserted = 0; inserted < size; inserted++)
    {
      r = firstRecord(schema, inserted, randomString(4), randomInt(0, size / 100));
      TEST_CHECK(insertRecord(table,r));
      freeRecord(r);
    }

  // get start time
  startTime = time(NULL);

  // read requests
  for (i = 0; i < requestsNumber; i++){
      // do update or scan? every 20th operation is a scan
      if (rand() % scanFreq != 0){
        // do insert or delete? 70% are inserts
        if (rand() % 100 <= percInserts)
        {
          r = firstRecord(schema, i, randomString(4), randomInt(0, size / 100));
          TEST_CHECK(insertRecord(table,r));
          rids[inserted++] = r->id;
          printf("+");
        } 
        else 
        {
          int delPos;
          // find not yet deleted record
          while(rids[delPos = (rand() % inserted)].page == -1);
          TEST_CHECK(deleteRecord(table, rids[delPos]));
          rids[delPos].page = -1;
          printf("-",delPos);
         }
      	} 
        else 
        {
          if (rand() % 10 == 0) 
          {
            Value *c;
            MAKE_VALUE(c, DT_INT, rand() % (inserted / 100));
            MAKE_CONS(left, c);
            MAKE_ATTRREF(right, 2);
            MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);

            TEST_CHECK(execScan(table, sel, schema));
        
            freeExpr(sel);
            printf ("(");
          } 
          else 
          {
            Value *c;
            MAKE_VALUE(c, DT_INT, rand() % inserted);
            MAKE_CONS(left, c);
            MAKE_ATTRREF(right, 0);
            MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);

            TEST_CHECK(execScan(table, sel, schema));
        
            freeExpr(sel);
            printf ("$");
          }
	      }
    }

  // measure time
  measureEndTime = time(NULL);
  (*t) += difftime(measureEndTime,startTime);
  (*inputOutput) += getOptionalInputOutput();
  printf("\nWORKLOAD TWO - N(R)=<%i>, #scans=<%i>, M=<%i>: %f sec, %ld I/Os\n", size, requestsNumber, numPages, *t, *inputOutput);

  // clean up
  TEST_CHECK(closeTable(table));
  TEST_CHECK(deleteTable("test_table_r"));
  TEST_CHECK(shutOptional());

  free(table);

  return RC_OK;
}

Schema * schema1 (void) 
{
  Schema *result;
  char *initializeNames[] = { "a", "b", "c" };
  DataType dataType[] = { DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 3);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
  int *cpSizes = (int *) malloc(sizeof(int) * 3);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
  {
    cpNames[i] = (char *) malloc(2);
    strcpy(cpNames[i], initializeNames[i]);
  }
  memcpy(cpDt, dataType, sizeof(DataType) * 3);
  memcpy(cpSizes, sizes, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}

Record * firstRecord(Schema *schema, int a, char *b, int c) 
{
  Record *result;
  Value *value;

  TEST_CHECK(createRecord(&result, schema));

  MAKE_VALUE(value, DT_INT, a);
  TEST_CHECK(setAttr(result, schema, 0, value));
  freeVal(value);

  MAKE_STRING_VALUE(value, b);
  TEST_CHECK(setAttr(result, schema, 1, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, c);
  TEST_CHECK(setAttr(result, schema, 2, value));
  freeVal(value);

  return result;
}

int randomInt (int min, int max) 
{
  int range = max - min;
  if (range == 0)
    return min;
  return (rand() % range) + min;
}

char * randomString (int length) 
{
  char *result = (char *) malloc(length);
  int i;
  
  for(i = 0; i < length; result[i++] = (char) rand() % 256)
    ;
      
  return result;
}
