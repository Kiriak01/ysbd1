// PANAGIOTIS KONTOEIDIS AM 1115201900266
//STYLIANOS DIMITRIADIS AM 1115201900050
//GEORGIOS KURIAKOPOULOS AM 1115201900092

#include <stdio.h>
#include <stdlib.h>
int * file; 
int fileCounter = 0; 
#include <string.h>
#include "bf.h"
#include "math.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20
#define MAX_DEPTH 7
#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}                            \

char * convert(int i) {             //calculate binary number
      static char bits[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
      int bits_index = 15;                                                                
      while ( i > 0 ) {
         bits[bits_index--] = (i & 1) + '0';
         i = ( i >> 1);
      }
      return bits;
      
   }
char * hash_func(int i,int globald) {    //calculate binary number i and return number of globald first bits
      static char bits[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
      int bits_index = 15;
      while ( i > 0 ) {
         bits[bits_index--] = (i & 1) + '0';
         i = ( i >> 1);
      }
      static char sigbits[16];
      strncpy(sigbits,bits,globald);
      return sigbits;
   } 

char * hash_index(int i,int globald) {     //calculate binary  number i and return number of globald last bits 
      static char bits[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
      int bits_index = 15;
      while ( i > 0 ) {
         bits[bits_index--] = (i & 1) + '0';                               
         i = ( i >> 1);
      } 
      static char sigbits[16];
      strncpy(sigbits,&bits[16-globald],globald);
      return sigbits;
   }

int ppow(int x,int n)    //calculate  x^n
{
    int i; 
    int number = 1;

    for (i = 0; i < n; ++i){
        number *= x;
    }
    return(number);
}

int counter(BF_Block * bl){         //calculate number of records inside block
  char * data;
  int i;
  int count=0;
  data=BF_Block_GetData(bl);
  int temp;      
  memcpy(&temp,data + sizeof(int),sizeof(int));
  return temp; 
}

void increaseBlockcnt(BF_Block * b) {                 //increase the total number of blocks
  char* temp;
  temp = BF_Block_GetData(b); 
  int c;
  memcpy(&c,temp + sizeof(int), sizeof(int));
  c++; 
  memcpy(temp + sizeof(int), &c, sizeof(int)); 
}



void rehashing(int indexDesc,int globaldepth,int blocknumdir){     //rehash directory indexes 
  int i=0;
  char * data;
  char index[16];
  BF_Block * b;
  BF_Block_Init(&b);
  for(i=0;i<ppow(2,globaldepth);i++){
    BF_GetBlock(indexDesc,blocknumdir,b);
    data=BF_Block_GetData(b);
    strcpy(index,hash_index(i,globaldepth));
    memcpy(data,index,sizeof(index));
  }
}


int findfreeblock(int indexDesc,int firstblock){              //when a bucket is splitted return the first free block with no records
  int i;
  int freeblock;
  int freeblockfound=firstblock+ppow(2,MAX_DEPTH) + 1; 
  BF_Block *b;
  BF_Block_Init(&b);
  char * data;
  for(i=0;i<ppow(2,MAX_DEPTH);i++){
    BF_GetBlock(indexDesc,freeblockfound,b); 
    data=BF_Block_GetData(b);
    memcpy(&freeblock , data + sizeof(int), sizeof(int)); 
    if(freeblock==0){
      return freeblockfound;
    }
    else{
      freeblockfound++;
    }
  }
}

void redirect(int indexDesc, int first_block, int global_depth) {             //we create an array of records for the records already inserted in the dictionary
  int first_bucket = first_block + ppow(2,MAX_DEPTH) + 1;                           //then we store all the records to this array 
  int counter;                                                                      //in order to re-insert them for the rehashing  
  int next_bucket = first_bucket;
  int temp_counter;
  BF_Block* b;
  BF_Block_Init(&b);
  char* data;
  
  for (int i = 0; i < ppow(2,MAX_DEPTH); i++) {
    BF_GetBlock(indexDesc, next_bucket, b);
    data = BF_Block_GetData(b);
    memcpy(&temp_counter, data+sizeof(int), sizeof(int));
    counter+=temp_counter;
    next_bucket++;
  }
  Record *record_array;
  record_array= (Record*)malloc(sizeof(Record)*counter);
  int j;
  next_bucket = first_bucket;
  int temp_counterr =0;
  int val =0;
  for(j=0; j<ppow(2,MAX_DEPTH);j++){
    BF_GetBlock(indexDesc, next_bucket, b);
    data = BF_Block_GetData(b);
    memcpy(&temp_counter, data+sizeof(int), sizeof(int));
    memcpy(data+sizeof(int), &val, sizeof(int));
    for(int k=0;k<temp_counter;k++){
      memcpy(&record_array[temp_counterr], data+sizeof(int)+(k+1)*sizeof(Record), sizeof(Record));
      temp_counterr++;
    }
  }
  int first_directory = first_block+1;
  char new_record_index[16];
  char new_directory_index[16];
  int block_pointer;
  int temp_temp;
  int val1=0;
  for(int h=0; h<counter; h++){
    
    strcpy(new_record_index,hash_func(record_array[h].id, global_depth));
    for(int a=0; a<ppow(2, MAX_DEPTH); a++){
      BF_GetBlock(indexDesc, first_directory, b);
      data = BF_Block_GetData(b);
      memcpy(new_directory_index, data, sizeof(new_directory_index));
      if(strcmp(new_record_index, new_directory_index)){
        memcpy(&block_pointer, data+sizeof(int), sizeof(int));
        BF_GetBlock(indexDesc, block_pointer, b);
        data = BF_Block_GetData(b);
        memcpy(&temp_temp, data+sizeof(int), sizeof(int));
        for(int a =0; a< temp_temp; a++){
          memcpy(data, &record_array[h].id+sizeof(int)+(a+1)*sizeof(Record), sizeof(Record));
          memcpy(data, &temp_temp+sizeof(int), sizeof(int));     
        }
      }
    }
  }
}


HT_ErrorCode HT_Init() {    
  file = malloc(MAX_OPEN_FILES*sizeof(int*)); 
  return HT_OK;
}


HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  int i;
  int fd=-1; 
  int locald=depth; 
  BF_CreateFile(filename);       
  BF_OpenFile(filename,&fd); 
  BF_Block *blockd; 
  BF_Block_Init(&blockd);
  CALL_BF(BF_AllocateBlock(fd,blockd));  
  char* d; 
  d = BF_Block_GetData(blockd); 
  memcpy(d,&depth,sizeof(int)); 
  BF_Block_SetDirty(blockd);
  BF_UnpinBlock(blockd);
  for(i=0;i<ppow(2,MAX_DEPTH);i++){                              //we create 2^MAX DEPTH blocks that contain directory indexes for our directory 
    int blockc;
    int nextdir;
    BF_GetBlockCounter(fd,&blockc);
    BF_GetBlockCounter(fd,&nextdir);
    blockc=blockc+ppow(2,MAX_DEPTH);       //fix pointer blockc to point to correct block
    if(i==ppow(2,MAX_DEPTH)-1){
      nextdir=0;                          //if we reach last directory index block dont point to next directory because there isnt one
    }
    else{
      nextdir=nextdir+1;                    //else go to next directory index
    }                             
    BF_Block * blockb;                          
    BF_Block_Init(&blockb);
    CALL_BF(BF_AllocateBlock(fd,blockb));
    char* db;
    db = BF_Block_GetData(blockb);
    if(i<ppow(2,depth)){                               //out of these 2^MAX DEPTH directory indexes we need at first only  2^global depth directory indexes
      char binary[16];
      strcpy(binary,hash_index(i,depth));
      memcpy(db,binary,sizeof(binary));
      memcpy(db+sizeof(binary),&blockc,sizeof(int));
      memcpy(db+sizeof(binary)+sizeof(int),&nextdir,sizeof(int));
    }
    else{                                                         //all the other  directory index blocks we mark them with -1 so we know they are not being used
      int j=-1;
      memcpy(db,&j,sizeof(int));
      memcpy(db+sizeof(int),&blockc,sizeof(int));
      memcpy(db+sizeof(int)+sizeof(int),&nextdir,sizeof(int));
    }
    BF_Block_SetDirty(blockb);
    BF_UnpinBlock(blockb);        //gia prin memcpy(&dd,db+sizeof(int),sizeof(int)) gia to deutero stoixeio opou dd int
  }
  for(i=ppow(2,MAX_DEPTH)+1;i<2*ppow(2,MAX_DEPTH);i++){                 //we continue creating blocks for the buckets of the directory
    BF_Block * blockd;                                                    //the total number of buckets is 128 
    BF_Block_Init(&blockd);                                                       //one bucket contains one block 
    BF_AllocateBlock(fd,blockd);
    char* d;
    int cnt = 0; 
    d = BF_Block_GetData(blockd);
    memcpy(d,&locald,sizeof(int));
    memcpy(d+sizeof(int), &cnt, sizeof(int)); 
    BF_Block_SetDirty(blockd);
    BF_UnpinBlock(blockd);

  }
  BF_CloseFile(fd); 

  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  if(fileCounter==MAX_OPEN_FILES-1){
     printf("sorry max opened files\n");   
  } 
  else{
    BF_OpenFile(fileName,indexDesc);
    file[fileCounter] = *indexDesc;
    fileCounter++;
    return HT_OK;
  }
}                                                                                                                                  

HT_ErrorCode HT_CloseFile(int indexDesc) {
  BF_CloseFile(indexDesc);   
  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  BF_Block * b;
  BF_Block_Init(&b);
  int utilization= BF_BLOCK_SIZE / sizeof(record);           //utilization number is the maximum record number per bucket  
  int totalrecordsinbucket;                                //how many records are in a bucket (calls counter func)
  char * data;                                                          //get data from blocks
  int localdepth;                                             //local depth of the bucket
  int blocksn,bgd;                                //bgd = number of block that contains global depth
  BF_GetBlockCounter(indexDesc,&blocksn);     //blocksn temporaty number of blocks
  bgd=blocksn-(2*ppow(2,MAX_DEPTH)); //bgd = number of block that contains global depth
  BF_GetBlock(indexDesc,bgd,b);
  char * d=BF_Block_GetData(b);                   //get data from b 
  int gd;                                      //gd=global depth
  memcpy(&gd,d,sizeof(int));            //store global depth in gd
  int dir_num;  
  int j;
  int bdir;                                     //current directory block number
  char hashid[16];
  int freeblock;                                        //first free block when we split buckets
  for (j = 0; j < ppow(2,gd); j++) {
    bdir = bgd+(j+1);                                               //number of current directory index
    BF_GetBlock(indexDesc,bdir,b); 
    char * temp = BF_Block_GetData(b);
    char val[16]; 
    memcpy(val,temp,sizeof(char*));                     // put in val the hashing index of the directory index
    strcpy(hashid,hash_func(record.id,gd));                   
    if(strcmp(hashid,val)){                                 //if directory index==hash_func(record.id) then take block number of the directory index and insert record
      int blockpointer;
      memcpy(&blockpointer,temp+sizeof(val),sizeof(int));       //domh dire char (00,01,klp) meta ena int pou deixnei se kapoio block,kai meta ena int poy deixnei sto epomeno directory
      int blockk;
      BF_GetBlock(indexDesc,blockpointer,b);
      char * temp = BF_Block_GetData(b); 
      memcpy(&dir_num,temp,sizeof(int));
      totalrecordsinbucket=counter(b);                                          //call counter and put total records in bucket 
      if(totalrecordsinbucket<utilization){                                                         //if we dont have overflow we insert the record
        data=BF_Block_GetData(b);
        memcpy(data+sizeof(int) +((totalrecordsinbucket+1)*sizeof(Record)),&record,sizeof(Record));   //we insert the record in  bucket 
        increaseBlockcnt(b);                              //increase the number of records in bucket
      }
    }
    else{
      int blockpointer;                                                           //case where local depth == global depth => expand directory, split bucket, gd++, ld++
      memcpy(&blockpointer,temp+sizeof(val),sizeof(int));
      data=BF_Block_GetData(b);
      memcpy(&localdepth,data,sizeof(int));                                                            //edw ginetai overflow tou bucket
      if(localdepth==gd){                  
        BF_GetBlock(indexDesc,bgd,b);
        d=BF_Block_GetData(b);
        gd++;                                                                         //increase global depth
        memcpy(d,&gd,sizeof(int));
        rehashing(indexDesc,gd,bgd+1);                                                //rehashing the directory indexes 
        BF_GetBlock(indexDesc,blockpointer,b);
        d=BF_Block_GetData(b);
        localdepth++;                                                                 //increase local depth of the bucket 
        memcpy(d,&localdepth,sizeof(int));
        freeblock=findfreeblock(indexDesc,bgd);                                           //find the next bucket we initialise after split for records
        BF_GetBlock(indexDesc,freeblock,b);
        d=BF_Block_GetData(b);
        memcpy(d,&localdepth,sizeof(int));                                              //define local depth for the new bucket
        redirect(indexDesc, bgd, gd);}                                                        //call redirect for rehashing  the directory indexes 
      else{                                                                             //case where global depth > local depth => split bucket, ld++
        int blockpointer;
        memcpy(&blockpointer,temp+sizeof(val),sizeof(int));                                                       //edw ginetai overflow tou bucket                 
        BF_GetBlock(indexDesc,bgd,b);
        d=BF_Block_GetData(b);
        gd++;
        memcpy(d,&gd,sizeof(int));
        BF_GetBlock(indexDesc,blockpointer,b);
        d=BF_Block_GetData(b);
        localdepth++;
        memcpy(d,&localdepth,sizeof(int));
        freeblock=findfreeblock(indexDesc,bgd);
        BF_GetBlock(indexDesc,freeblock,b);
        d=BF_Block_GetData(b);
        memcpy(d,&localdepth,sizeof(int));
        redirect(indexDesc, bgd, gd);
      }
    }                                                                                                                               
  }                                                                                                                              

  return HT_OK;
} 


HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {                 
  BF_Block * b;
  BF_Block_Init(&b);            
  int blocks;
  BF_GetBlockCounter(indexDesc,&blocks);
  int firstblock=blocks-(2*ppow(2,MAX_DEPTH));                          //get number of the first block
  BF_GetBlock(indexDesc,firstblock,b);
  char * data=BF_Block_GetData(b);
  int gd;     
  memcpy(&gd,data,sizeof(int));                                         // get global depth
  int i;
  char val[16];
  for(i=0;i<ppow(2,gd);i++){                                            //run through all directories 
    BF_GetBlock(indexDesc,firstblock+1+i,b);
    data=BF_Block_GetData(b);                                         //for each directory get number of bucket
    int blockpointer;                                             
    memcpy(&blockpointer,data+sizeof(val),sizeof(int));
    BF_GetBlock(indexDesc,blockpointer,b);
    data=BF_Block_GetData(b);
    int counter;
    memcpy(&counter,data+sizeof(int),sizeof(int));                        //for each bucket get number of records 
    int j;                                                        
    for(j=0;j<counter;j++){                                               //for each record get its values
      Record idd;
      memcpy(&idd,data+sizeof(int)+((j+1)*sizeof(Record)),sizeof(Record));
      printf("record id %d\n",idd.id);
      printf("record name %s\n",idd.name);
      printf("record surname %s\n",idd.surname);
      printf("record city %s\n",idd.city);

    }
  }
  return HT_OK;
}

HT_ErrorCode HT_Hash_Statistics(const char* filename) {          //same logic as print_all_entries but we find max records min records 
  int fd;                                                         //average num of records and sum of all record files
  int blocks_num;
  BF_Block *b;
  BF_Block_Init(&b);
  BF_OpenFile(filename,&fd);  
  BF_GetBlockCounter(fd,&blocks_num);  
  printf("File has %d number of blocks.\n", blocks_num); 
  int bgd=blocks_num-(2*ppow(2,MAX_DEPTH));
  BF_GetBlock(fd,bgd,b);
  char * d=BF_Block_GetData(b);
  int gd; //gd=global depth
  memcpy(&gd,d,sizeof(int));
  int i;
  char val[16];
  int maxx=-1;
  int min=10;
  int exists;
  int pl=0;
  int count=0;
  for (int i = 0; i < ppow(2,gd); i++) {                             
    BF_GetBlock(fd,bgd+1+i,b);
    d=BF_Block_GetData(b);
    int blockpointer;
    memcpy(&blockpointer,d+sizeof(val),sizeof(int));
    BF_GetBlock(fd,blockpointer,b);
    d=BF_Block_GetData(b);
    int counter;
    memcpy(&counter,d+sizeof(int),sizeof(int));
    if(counter>maxx){
      maxx=counter;
    }
    if(counter<min){
      min=counter;
    }
    memcpy(&exists,d,sizeof(int));
    if(exists!=-1){
      pl++;
    }
    count=count+counter;
  }
  printf("max number of records is %d\n",maxx);
  printf("min numbers of records is %d\n",min);
  int average=count/pl;
  printf("average number of records is %d\n",average);
  return HT_OK;
}