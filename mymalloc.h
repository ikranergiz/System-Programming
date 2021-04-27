#include <stdio.h>
typedef struct Info{
    unsigned int size;
    unsigned int isfree;
}Info;

typedef struct Block{
   Info info;           /*number of  16 byte blocks*/
   struct Block *next;  /*next free*/
   char data[0];        /*start of the allocated memory*/
 }Block;
/*Block: |4byte(size)|4byte(isfree)|8-byte next|0-byte(data)|*/

static Block *free_list = NULL;     /*start of the free list*/
static Block *heap_start = NULL;    /*head of allocated memory from sbrk */

void *mymalloc(size_t size);
void *myfree(void *p);
Block *split(Block *b, size_t size);
void printHeap();

static int first_call = 0;
size_t SIZE = 1024;

static int meta = sizeof(Block); //metadata
static int btag = sizeof(Info); //boundry tag 
/*free listi ekrana basar.*/
void printFreeList(){
  
  printf("Free List:\n");
  printf("----------------------\n");

  Block *iter = free_list;
  while(iter != NULL){

    printf("free: %d\nsize: %d\n",iter->info.isfree,iter->info.size);
    printf("----------------------\n");

    iter = (Block *)(iter->next);
  }
}
/*Verilen bloğun boundry tag'ine gider*/
Info *goBtag(Block *block){

  Info *btag = (Info *)((char *)block + meta + block->info.size);

  return btag;
}
/*Verilen allowed bloğu free block yapar*/
void makeFreeBlock(Block *block){

  block->info.isfree = 1;
  block->next = NULL;
  
  Info *btag = goBtag(block);
  btag->size = block->info.size;
  btag->isfree = 1;

  return;
}
/*Verilen free bloğu allowed block yapar*/
void makeAllowedBlock(Block *block){

  block->info.isfree = 0;
  block->next = NULL;
  
  Info *btag = goBtag(block);
  btag->size = block->info.size;
  btag->isfree = 0;

  return;
}
/*Verilen bloğu free listten siler*/
void deleteFreeList(Block *fblock){

  if(free_list == fblock){

    free_list = NULL;
    makeAllowedBlock(fblock);
    
    return;
  }

  if(free_list->next == fblock){

    free_list->next = NULL;
    makeAllowedBlock(fblock);

    return;
  }

  Block *iter = free_list;

  while(iter->next != NULL && (Block *)(iter->next) != fblock){

    iter = (Block *)iter->next;
  }
  
  iter->next = fblock->next;

  makeAllowedBlock(iter);
}
/*Verilen bloğu free liste ekler*/
void addFreeList(Block *block){

  //free listte eleman yoksa 
  if(free_list == NULL){

    free_list = block;
    free_list->next = NULL;

    return;
  }

  Block *iter = free_list;//free list iterator

  while(iter->next != NULL){

    iter = (Block *)iter->next;
  }

  iter->next = (struct Block *)block;
  block->next = NULL;
}
/*Gelen blocktan bir önceki bloğa gider ve
*bu bloğu retur eder.
*/
Block *prevBlock (Block *block){
  
  block = (Block *)((char *)block - btag - block->info.size - meta);

  if(block < heap_start){
      perror("prev:");
      return NULL;
  }

  return block;
}
/*Gelen blocktan bir sonraki bloğa gider ve
*bu bloğu retur eder.
*/
Block *nextBlock (Block *block){

  block = (Block *)((char *)block + meta + block->info.size + btag);

  Block *heapEnd = (SIZE + heap_start);

  if(block > heapEnd){
      perror("next: ");
      return NULL;
  }

  return block;
}
/*Free listeki size a uygun ilk bloğu döndürür.*/
Block *firstFit(size_t size){

  Block *iter = free_list;//free list iterator

  /*free list boşsa*/
  if(iter == NULL){
    return NULL;
  }

  /*ilk free block uygunsa*/
  if((iter->next != NULL) && (iter->info.size > size)){

    return split(iter,size); // allowed blok dönsün
  }
  if((iter->next != NULL) && (iter->info.size == size)){

    deleteFreeList(iter);
  
    return iter;
  }

  Block *prev = iter; 

  while((iter->info.size < size) && (iter != NULL) ){

    prev = iter;
    iter = (Block *)iter->next;

  }

  if(iter == NULL){//uygun free block yok
    return NULL;
  }
  
  else if(iter->info.size == size){
    
    deleteFreeList(iter);
    
    return iter;
  }
  else{ // free block size istenilen size dan büyükse

    return split(iter,size); //allowed blok döndürür
  }
}
/*Free listeki size a en uygun bloğu döndürür.*/
Block *bestFit(size_t size){

  if(free_list == NULL){
    return NULL;
  }

  //tek eleman varsa
  if(free_list->next == NULL){

    if(free_list->info.size == size){

      free_list = NULL;
      return free_list;
    }
    //split
    else if(free_list->info.size > size){

      return split(free_list,size);
    }

    return NULL;
  }

  Block *iter = free_list;
  Block *next_block = (Block *)iter->next;
  Block *best = NULL;

  int bestSize = 0;

  while ((iter->next != NULL) && !(iter->info.size < size)){

    if(iter->info.size == size){
      return iter;
    }

    if(bestSize == 0){//best size tanımlanmamışsa

      bestSize = iter->info.size;
      best = iter;
    }

    if(iter->info.size < bestSize){//best size değişecek mi?

      bestSize = iter->info.size;
      best = iter;
    }

    iter = (Block *)iter->next;
    next_block = (Block *)iter->next;
  }
  return best;
}
/*Free liste sizedan büyük a en büyük bloğu döndürür.*/
Block *worstFit(size_t size){

    if(free_list == NULL){
    return NULL;
  }

  //tek eleman varsa
  if(free_list->next == NULL){

    if(free_list->info.size == size){

      free_list = NULL;
      return free_list;
    }
    //split
    else if(free_list->info.size > size){

      return split(free_list,size);
    }
    return NULL;
  }

  Block *iter = free_list;
  Block *next_block = (Block *)iter->next;
  Block *best = NULL;

  int maxSize = 0;

  while ((iter->next != NULL) && !(iter->info.size < size)){

    if(iter->info.size == size){
      return iter;
    }

    if(maxSize == 0){
      maxSize = iter->info.size;
      best = iter;
    }

    if(iter->info.size > maxSize){
      maxSize = iter->info.size;
      best = iter;
    }
    
    iter = (Block *)iter->next;
    next_block = (Block *)iter->next;
  }
  return best;
}

