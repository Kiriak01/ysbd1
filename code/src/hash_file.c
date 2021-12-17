
#include <stdio.h>
#include <stdlib.h>
int * file; 
int fileCounter = 0; 
#include <string.h>
#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}                            \


typedef struct node1{
  BF_Block * b; 
  int id,gd; 
  struct node1* next;
} directory;       

void pushd(directory * head, BF_Block * b,int i);



HT_ErrorCode HT_Init() {
  file = malloc(MAX_OPEN_FILES*sizeof(int*)); 

  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  directory dir; 
  printf("Created empty directory.\n");                 ///////dhmioyrgia directory mesa sthn create index kai oxi global 
  int i;
  int fd=-1; 
  BF_CreateFile(filename); 
  printf("file desc before is %d\n", fd);
  BF_OpenFile(filename,&fd);
  printf("File desc is %d\n", fd);  
  BF_Block *blockd; 
  BF_Block_Init(&blockd);
  BF_AllocateBlock(fd,blockd);  
  
  char* d; 
  d = BF_Block_GetData(blockd); 
  printf("d is %s\n",d); 
  memcpy(d,&dir,sizeof(directory));   
  printf("d is %s\n", d); 

  for(i=0;i<2;i++){ 
    BF_Block * blockb;
    BF_Block_Init(&blockb);
    BF_AllocateBlock(fd,blockb);
    char* db;
    db = BF_Block_GetData(blockb);
    printf("DB IS %s\n", db);
    memcpy(db,&blockb,sizeof(blockb));  
    printf("DB NOW IS %s\n",db);      //dhmiourgw directory me 4 kelia afou global depth=2 me pointers NULL afou BF_Blocks adeia
    pushd(&dir,NULL,i); 
  }

  BF_CloseFile(fd); 
  

  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  BF_OpenFile(fileName,indexDesc);
  file[fileCounter] = *indexDesc; 
  return HT_OK; 
}

HT_ErrorCode HT_CloseFile(int indexDesc) {   
  //BF_Destroy()     //otan ena file kleinei prepei na vgainei kai apo ton pinaka me ta files kai na apodesmeyetai o xwros toy
  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

  //////////////////////////// directories


  void pushd(directory * head, BF_Block * b,int i) {
    directory * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    // now we can add a new variable 
    current->next = (directory *) malloc(sizeof(directory));
    current->next->b = b;
    current->next->id=i;
    current->next->next = NULL;
  }
   