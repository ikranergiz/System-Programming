#include"mymalloc.h"
#include<unistd.h>

void *mymalloc(size_t size){

    if(0 == first_call){//func ilk çağırış

        heap_start = (void *)sbrk(SIZE);

        //sbrk ile alan alınamadıysa
        if( (void *) -1 == heap_start){
            perror("sbrk: ");
            return NULL;
        }

        /*free block oluşturma*/
        Block *free_block = heap_start;

        //block size 16'nın katı
        free_block->info.size = (SIZE - meta - btag ); // 1024 - 16 - 16 // tüm block size

        makeFreeBlock(free_block);

        addFreeList(free_block);

        first_call = 1;
    }
    Block *allowed = firstFit(size); //allowed olmuş block döndürür.
    
    if(allowed == NULL){
        perror("no memory");
        return NULL;
    }

    return allowed->data;
}
void *myfree(void *p){
    
    Block *block = (Block *)((char *)p - meta);//metadataya gittik
    
    //left coalescing 
    if( !(block <= heap_start)){

        Info *b = (Info *)((char *)block - btag);//bir önceki block btag 
        
        if(b->isfree == 1){//btag free ise

            Block *prev = (Block *)((char *)block - btag - b->size - meta );

            deleteFreeList(prev);
            
            prev->info.size += btag + meta + block->info.size;//size güncelle

            makeFreeBlock(prev);
            addFreeList(prev);
            return NULL;
        }
    }
    if(!(block >(heap_start + SIZE) )){
       
        //right coalescing
        Block *next = (Block *)((char *)block + block->info.size + meta + btag);

        if(next->info.isfree == 1){//next block meta isfree 1 ise
            
            deleteFreeList(next);

            block->info.size += btag + meta + next->info.size;//size güncelle

            makeFreeBlock(block);
            addFreeList(block);
            
            return NULL;
        }
    }
    //coalescing yoksa 
    makeFreeBlock(block);
    addFreeList(block);

}
// |size|remaining|
Block *split(Block *b, size_t size){//split çağırıldığında b size belli olacak

    deleteFreeList(b);//free listten çıkar

    int maxSize = b->info.size;//gelen bloğun size'ını tutar
    
    Block *allowed = b;//ayrılmış block
    allowed->info.size = size; //b nin size ı değişti

    makeAllowedBlock(allowed);

    Block *free = (Block *)((char *)b + meta + size + btag);//free block
    free->info.size = maxSize - meta - btag -size;
    
    makeFreeBlock(free);

    addFreeList(free);

    return allowed;
}
/*
*best fit hep en yakın size da block bulduğundan external
*fragmention daha az olur.
*worst fit hep en büyük bloğu bulup split edeceğinden 
*daha fazla ex. frag. olur.
*en verimli best fit olur.
*/
void printHeap(){

    printf("Heap:\n");
    printf("----------------------\n");

    Block *iter = heap_start;
    Block *end = (Block *)((char *)heap_start + SIZE);

    while(iter != end){

        printf("free: %d\nsize: %d\n",iter->info.isfree,iter->info.size);
        printf("----------------------\n");

        iter = nextBlock(iter);
    }
}
int main(){

    int *a = mymalloc(16);
    void *b = mymalloc(32);
    void *c = mymalloc(36);
    void *d = mymalloc(40);
    void *e = mymalloc(44);

    void *f = mymalloc(44);
    void *g = mymalloc(44);
    void *h = mymalloc(44);
    void *o = mymalloc(44);

    void *i = mymalloc(44);
    void *j = mymalloc(44);

    printFreeList();
    printf("\n\n");
    printHeap();
    printf("\n\n");
    printf("\n\n");

    myfree(b);
    myfree(d);
    myfree(f);
    myfree(h);
    myfree(i);
    myfree(a);
    
    printFreeList();
    printf("\n\n");
    printHeap();
    printf("\n\n");
    printf("\n\n");

    myfree(e);

    return 0;
}

