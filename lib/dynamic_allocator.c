/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== OUR DEFINED FUNCTIONS =============================//
//==================================================================================//
//struct MemBlock_LIST free_block_list;
uint8 called_sbrk = 0;
uint32 size_called_sbrk;

void update_call_sbrk(uint32 size)
{
	if(size >= size_called_sbrk)
		called_sbrk = 0;
}

int8 is_phys_last(struct BlockMetaData *currBlock)
{
	return (((uint32)currBlock + currBlock->size) == (uint32)sbrk(0));
}

struct BlockMetaData *phys_next(struct BlockMetaData *currBlock)
{
	if(currBlock == NULL)
		return NULL;

	if(is_phys_last(currBlock))
		return NULL;

	return (struct BlockMetaData *)((uint32)currBlock + currBlock->size);
}

int8 is_phys_next_free(struct BlockMetaData *next)
{
	return (next != NULL && next->is_free)? 1 : 0;
}

int8 is_phys_prev_free(struct BlockMetaData *currBlock)
{
	struct BlockMetaData *prev_free = LIST_PREV(currBlock);
	if(phys_next(prev_free) == currBlock)
		return 1;
	return 0;
}

void free_insert(struct BlockMetaData *newFreeBlock)
{
	if(LIST_EMPTY(&free_block_list))
	{
		LIST_INSERT_HEAD(&free_block_list, newFreeBlock);
		LIST_LAST(&free_block_list) = newFreeBlock;
		return;
	}

	if(newFreeBlock > LIST_LAST(&free_block_list))
	{
		LIST_INSERT_TAIL(&free_block_list, newFreeBlock);
		return;
	}

	struct BlockMetaData *currBlock = NULL;
	LIST_FOREACH(currBlock, &free_block_list)
	{
		if(newFreeBlock < currBlock)
		{
			LIST_INSERT_BEFORE(&free_block_list, currBlock, newFreeBlock);
			return;
		}
	}
}

void setVBlock0(struct BlockMetaData *MetaData)
{
	if(MetaData->is_free){
		MetaData->is_free = 0;
		LIST_REMOVE(&free_block_list ,MetaData);
	}
	MetaData->size = 0;
}

void *allocff_and_free(void* va, uint32 new_size)
{
	void * ret = alloc_block_FF(new_size);
	if(ret != NULL)  //allocation succeeded
	{
		free_block(va);
	}
	return ret;
}

void merge_prev(struct BlockMetaData *currBlock)
{
	if(is_phys_prev_free(currBlock))
	{
		LIST_PREV(currBlock)->size += currBlock->size;
		update_call_sbrk(currBlock->size);
		setVBlock0(currBlock);
	}
}


void split_block(struct BlockMetaData *currBlock , uint32 size)
{
	uint32 remSpace = currBlock->size - size - sizeOfMetaData();
	if(remSpace >= sizeOfMetaData())
	{
		currBlock->size -= remSpace;
		struct BlockMetaData *newBlock = (struct BlockMetaData *)((uint32)currBlock + currBlock->size);
		newBlock->is_free = 1;
		newBlock->size = remSpace;

		if(currBlock->is_free){
			LIST_INSERT_AFTER(&free_block_list, currBlock, newBlock);
		}
		else {
			free_block(newBlock+1);
		}
	}
	if(currBlock->is_free)
	{
		currBlock->is_free = 0;
		LIST_REMOVE(&free_block_list, currBlock);
	}
}


void* call_sbrk(void *old_brk, uint32 size)
{
	called_sbrk = 1;
	size_called_sbrk = size;

	struct BlockMetaData *meta_data = (struct BlockMetaData *)(old_brk);
	meta_data->size = sbrk(0) - old_brk;
	meta_data->is_free = 1;
	LIST_INSERT_TAIL(&free_block_list, meta_data);
	split_block(meta_data, size);

	return (meta_data + 1);
}

void realloc_data(char *old_va, char* end, char *new_va)
{
	for(uint32 i = 0x0; (old_va + i) < end; i += sizeof(char))
	{
		*(new_va + i) = *(old_va + i);
	}
}

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================
void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		//cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
		cprintf("(blk: %x, size: %d)\n", blk, blk->size) ;
	}
	cprintf("=========================================\n");
}

void print_blocks_list_tail(struct MemBlock_LIST list, int num)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	int ctr = 1;
	cprintf("\nDynAlloc Blocks List:\n");
	for(blk = LIST_LAST(&list); ctr <= num-1 ; blk = LIST_PREV(blk), ctr++);
	for(; blk != NULL; blk = LIST_NEXT(blk))
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");
}

//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================

void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;

	is_initialized = 1;
	//=========================================
	//=========================================
	//cprintf(brk);

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");

	struct BlockMetaData *metaData = (struct BlockMetaData *) daStart;

	metaData->size=initSizeOfAllocatedSpace;
	metaData->is_free=1;
	metaData->prev_next_info.le_next=NULL;
	metaData->prev_next_info.le_prev=NULL;

	LIST_INIT(&free_block_list);
	LIST_INSERT_HEAD(&free_block_list, metaData);
	LIST_LAST(&free_block_list) = metaData;
}
//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//cprintf("called.. size=%d\n", size);
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
    //panic("alloc_block_FF is not implemented yet");
    if(size==0) return NULL;

    //cprintf(">> allocate size = %d\n", size+16);
    if (!is_initialized)
    {
//    	cprintf("block allocator initializing\n");
		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32)sbrk(required_size);
		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
    	//cprintf("block allocator initialized\n");
    }

    struct BlockMetaData *currBlock;
    uint32 emptySpace, remSpace;
  //  cprintf("size out : %d crrrr : %d \n",size ,crrrr++ );
    if(called_sbrk == 1 )
  //  if(0)
    {
    	// cprintf("size in : %d \n",size );
    	if(size >= size_called_sbrk)
    	{
    		void* break_line = sbrk(0);
    		void* vaaa = LIST_LAST(&free_block_list);
    		if( vaaa >= break_line)
    		{
    		emptySpace = LIST_LAST(&free_block_list)->size - sizeOfMetaData();
			if(emptySpace >= size && (LIST_LAST(&free_block_list)->is_free == 1))
			{
//    		  	call_sbrk=2;
//				LIST_LAST(&free_block_list)->is_free = 0;
				struct BlockMetaData *list_ll= LIST_LAST(&free_block_list);
				split_block(LIST_LAST(&free_block_list),size);
				if(LIST_LAST(&free_block_list)->size < size)
					called_sbrk = 0;
				return (list_ll + 1);
			}
			else
			{
				void * old_brk = sbrk(size);
				if(old_brk != (void*)-1)
				{
					return call_sbrk(old_brk, size);
				}
				// cprintf("\n======> sbrk called and failed\n");
				return NULL;
			}
    	 }
    	}

    }

    LIST_FOREACH(currBlock, &free_block_list)
    {
    	emptySpace = currBlock->size - sizeOfMetaData();
		if(emptySpace >= size)
		{
//			currBlock->is_free = 0;
			split_block(currBlock,size);
			return (currBlock + 1);
		}
    }

   // cprintf("sbreak called \n");
    void * old_brk = sbrk(size);
    if(old_brk != (void*)-1)
    {
    	return call_sbrk(old_brk, size);
    }
    // cprintf("\n======> sbrk called and failed\n");

    return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//panic("alloc_block_BF is not implemented yet");
	 if(size==0) return NULL;

	struct BlockMetaData *currBlock;
	struct BlockMetaData *bestFitBlock = NULL;

	uint32 emptySpace;
	LIST_FOREACH(currBlock, &free_block_list)
	{
		emptySpace = currBlock->size - sizeOfMetaData();
		if(emptySpace == size)
		{
			currBlock->is_free = 0;
			return currBlock+1;
		}
		if(emptySpace > size)
		{
			if(bestFitBlock == NULL || bestFitBlock->size > currBlock->size){
				bestFitBlock = currBlock;
			}
		}
	}
	if (bestFitBlock == NULL)
	{
		void *old_brk = sbrk(size);
		if (old_brk != (void*)-1) {
			return call_sbrk(old_brk, size);
		}
		return NULL;
	}

//	    bestFitBlock->is_free = 0;
	emptySpace = bestFitBlock->size - sizeOfMetaData();
	split_block(bestFitBlock,size);
	return bestFitBlock+1;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//cprintf(" address in free .... = %x \n\n" , va);
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	struct BlockMetaData *currBlock = ((struct BlockMetaData *)va - 1);
	currBlock->is_free = 1;
	struct BlockMetaData *next = phys_next(currBlock);

	if(is_phys_next_free(next))
	{
		currBlock->size += next->size;
		update_call_sbrk(currBlock->size);
		LIST_INSERT_BEFORE(&free_block_list, next, currBlock);
		setVBlock0(next);
		merge_prev(currBlock);
		return;
	}

	free_insert(currBlock);
	merge_prev(currBlock);
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================

void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	//panic("realloc_block_FF is not implemented yet");

	if(va == NULL && new_size == 0)
	{
		return NULL;
	}
	else if(va == NULL)
	{
		void * ret = alloc_block_FF(new_size);
		if(ret != NULL)  //allocation succeeded
		{
			return ret;
		}
		else
		{  // allocation failed
			return va;
		}
	}
	else if(new_size == 0)
	{
	    free_block(va);
	    return NULL;
	}
	else if(is_free_block(va)){
		return NULL;
	}

	/*-----------------------------------------------------------------------------
	 * new size > old size
	 * 					- next is free
	 * 					 			a- total size >= new size ==> split next
	 * 					 			b- total size < new size  ==> alloc and free
	 * 					c- next is not free ==> alloc and free
	 * 					d- has no next ==> alloc and free
	 *
	 * a ==> split next
	 * b,c,d ==> alloc and free
	 * -----------------------------------------------------------------------------
	 *
	 * new size < old size
	 * 					e- next is free ==> split and merge
	 * 					f- next is not free ==> split and free
	 * 					g- has no next ==> split and free
	 *
	 * e ==> split and merge
	 * f,g ==> split and free
	 * -----------------------------------------------------------------------------
	 */

	struct BlockMetaData *currBlock = ((struct BlockMetaData *)va - 1);
	uint32 old_size = currBlock->size - sizeOfMetaData();

	struct BlockMetaData *nextBlock = phys_next(currBlock);
//	cprintf(">> currBlock = %x, nextBlock = %x\n", currBlock, nextBlock);
//	cprintf(">> currBlock->size = %d, new_size = %d\n", currBlock->size, new_size);
//	cprintf("list before :\n");
//	print_blocks_list(free_block_list);

	if(new_size == old_size){
//		cprintf("1\n");
		return va;
	}
	if(new_size > old_size)
	{
//		cprintf("2\n");
		if(is_phys_next_free(nextBlock) && nextBlock->size + old_size >= new_size)
		{
//			cprintf("3\n");
			//a ==> split next
			split_block(nextBlock, new_size - old_size - sizeOfMetaData());
			currBlock->size += nextBlock->size;
			setVBlock0(nextBlock);
		}
		else
		{
//			cprintf("4\n");
			//b,c,d ==> alloc and free
			void *new_va =  allocff_and_free(va, new_size);
			if(new_va != NULL)
			{
				realloc_data(va, (void *)nextBlock, new_va);
				return new_va;
			}
		}
		return va;
	}
	else if(new_size < old_size)
	{
		split_block(currBlock, new_size);
		return va;
	}


	return NULL;
}

