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

//struct MemBlock_LIST MemoryList;

void setVBlock0(struct BlockMetaData *MetaData)
{
	MetaData->is_free = 0;
	MetaData->size = 0;
	LIST_REMOVE(&MemoryList ,MetaData);
	//free(MetaData);
}

void *allocff_and_free(void* va, uint32 new_size)
{
	void * ret = alloc_block_FF(new_size);
	if(ret != NULL)  //allocation succeeded
	{
		free_block(va);
		return ret;
	}
	else {  // allocation failed
		return (void*)-1;
	}
}

void split_block(struct BlockMetaData *currBlock , uint32 size)
{
	uint32 remSpace = currBlock->size- size - sizeOfMetaData();
	if(remSpace>=sizeOfMetaData())
	{
		currBlock->size -= remSpace;
		struct BlockMetaData *newBlock = (struct BlockMetaData *)((uint32)currBlock + currBlock->size);
		newBlock->is_free = 1;
		newBlock->size = remSpace;
		LIST_INSERT_AFTER(&MemoryList, currBlock, newBlock);
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

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");

	struct BlockMetaData *metaData = (struct BlockMetaData *) daStart;

	metaData->size=initSizeOfAllocatedSpace;
	metaData->is_free=1;
	metaData->prev_next_info.le_next=NULL;
	metaData->prev_next_info.le_prev=NULL;

	LIST_INIT(&MemoryList);
	LIST_INSERT_HEAD(&MemoryList, metaData);
	LIST_LAST(&MemoryList) = metaData;
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

    if (!is_initialized)
    {
    	//cprintf("block allocator initializing\n");
		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32)sbrk(required_size);
		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
    	//cprintf("block allocator initialized\n");
    }


    struct BlockMetaData *currBlock;
    uint32 emptySpace, remSpace;
    LIST_FOREACH(currBlock, &MemoryList)
    {
    	if(currBlock->is_free)
    	{
    		emptySpace = currBlock->size - sizeOfMetaData();
    		if(emptySpace >= size)
    		{
    			currBlock->is_free = 0;
    			split_block(currBlock,size);
    			return (currBlock + 1);
    		}
    	}
    }

    void * ret = sbrk(size);
    if(ret != (void*)-1)
    {
    	//cprintf("\nsbrk called\n");
    	//cprintf(" ...... ........... List last size = %d \n" ,LIST_LAST(&MemoryList)->size);

		struct BlockMetaData *meta_data = (struct BlockMetaData *) (ret);
		meta_data->size = sbrk(0) - ret;
		meta_data->is_free = 1;
		LIST_INSERT_TAIL(&MemoryList, meta_data);

    	free_block(ret + sizeOfMetaData()); //to merge

    	currBlock = LIST_LAST(&MemoryList);
    	currBlock->is_free = 0;
    	split_block(currBlock,size);
    	return (currBlock + 1);

//		return alloc_block_FF(size);
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
	    LIST_FOREACH(currBlock, &MemoryList)
	    {
	    	if(currBlock->is_free)
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
	    }
	    if (bestFitBlock == NULL){
	    	if (sbrk(size) != (void*)-1) {
	    		return alloc_block_BF(size);
	    	}
	    	return NULL;
	    }

	    bestFitBlock->is_free = 0;
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
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	struct BlockMetaData *currBlock = ((struct BlockMetaData *)va - 1);
	struct BlockMetaData *first_element = LIST_FIRST(&MemoryList);
	struct BlockMetaData *last_element = LIST_LAST(&MemoryList);
	struct BlockMetaData *nextBlock = LIST_NEXT(currBlock);
	struct BlockMetaData *prevBlock = LIST_PREV(currBlock);


	if(!(currBlock == first_element || currBlock == last_element))
	{
		if(nextBlock->is_free == 1 && prevBlock->is_free == 1) //next and prev  are empty
		{
			prevBlock->size += currBlock->size + nextBlock->size;
			setVBlock0(nextBlock);
			setVBlock0(currBlock);
		}
		else if (nextBlock->is_free == 0 && prevBlock->is_free == 0)//next and prev  are not empty
		{
			currBlock->is_free = 1;
		}
		else if(nextBlock->is_free == 1 && prevBlock->is_free == 0)// next is empty
		{
			currBlock->size += nextBlock->size;
			currBlock->is_free = 1;
			setVBlock0(nextBlock);
		}
		else if(nextBlock->is_free == 0 && prevBlock->is_free == 1) // prev is empty
		{
			prevBlock->size += currBlock->size;
			setVBlock0(currBlock);
		}
	}
	else if(currBlock == first_element && currBlock == last_element) // there is one block in the list
	{
		currBlock->is_free = 1;
	}
	else if(currBlock == first_element)
	{
		if(nextBlock->is_free == 1)
		{
			currBlock->size += nextBlock->size;
			currBlock->is_free = 1;
			setVBlock0(nextBlock);
		}
		else
		{
			currBlock->is_free = 1;
		}

	}
	else if(currBlock == last_element)
	{
		if(prevBlock->is_free == 1)
		{
			prevBlock->size += currBlock->size;
			setVBlock0(currBlock);
		}
		else
		{
			currBlock->is_free = 1;
		}
	}
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
		//cprintf("\n>>>>>>>>>1 \n");
		return NULL;
	}
	else if(va == NULL )
	{
		//cprintf("\n>>>>>>>>>2 \n");
		void * ret = alloc_block_FF(new_size);
		if(ret != NULL)  //allocation succeeded
		{
			return ret;
		}
		else {  // allocation failed
			return (void*)-1;
		}
	}
	else if( new_size == 0 )
	{
		//cprintf("\n>>>>>>>>>3 \n");
	    free_block(va);
	    return NULL;
	}
	else if(is_free_block(va)){
		return NULL;
	}

	struct BlockMetaData *currBlock = ((struct BlockMetaData *)va - 1);
	uint32 totalFreeSize, remSpace ;
	struct BlockMetaData *nextBlock = LIST_NEXT(currBlock);

	if(nextBlock != NULL)  // if it has a next node
	{
		if(currBlock->size - sizeOfMetaData() < new_size)  // new size is bigger than current size
		{
			if(nextBlock->is_free)   // if next free
			{
				totalFreeSize = currBlock->size + nextBlock->size - sizeOfMetaData();
		    	if(totalFreeSize == new_size)    // if next free and total size == new size
		    	{
		    		//cprintf("\n>>>>>>>>>4 \n");
		    		currBlock->size = new_size + sizeOfMetaData();
		    		setVBlock0(nextBlock);
		    		return va;
		    	}
		    	else if (totalFreeSize > new_size)    // if next free and total size > new size
		    	{
		    		//cprintf("\n>>>>>>>>>5 \n");
		    		currBlock->size= totalFreeSize+sizeOfMetaData();  // split will handle it it's not your business
		    		setVBlock0(nextBlock);
		    		split_block(currBlock,new_size);
					return va;
		    	}
		    	else if (totalFreeSize < new_size)    // if next free and total size < new size call free bloc and after allocate the bloc by FF
		    	{
		    		//cprintf("\n>>>>>>>>>6 \n");
		    		return allocff_and_free(va, new_size);
		    	}
			}
			else {  // next is not free
	    		//cprintf("\n>>>>>>>>>7 \n");
				return allocff_and_free(va, new_size);
			}
		}
		else if ( currBlock->size - sizeOfMetaData() > new_size )  // new size is smaller than current size
		{
			if(nextBlock->is_free == 0)   // next is not free  (full)
			{
	    		//cprintf("\n>>>>>>>>>8 \n");
				split_block(currBlock,new_size);
			}
			else // next is free
			{
	    		remSpace = (currBlock->size - new_size - sizeOfMetaData());
	    		currBlock->size = new_size + sizeOfMetaData();
	    		struct BlockMetaData *newBlock = (struct BlockMetaData *)((uint32)currBlock + currBlock->size);
	    		newBlock->is_free = 1;
	    		newBlock->size = remSpace + nextBlock->size;
	    		LIST_INSERT_AFTER(&MemoryList, currBlock, newBlock);
	    		setVBlock0(nextBlock);
			}
			return va;
		}
		else if ( currBlock->size - sizeOfMetaData() == new_size ) // new size is equal to current size
		{
    		//cprintf("\n>>>>>>>>>10 \n");
			return va;
		}
	}
	else   // if the node is at the tail or at the its just one node and head and tail
	{
		if(currBlock->size - sizeOfMetaData() < new_size)// new size is greater than current (last) size
		{
    		//cprintf("\n>>>>>>>>>11 \n");
			return allocff_and_free(va, new_size);
		}
		else if(currBlock->size - sizeOfMetaData() > new_size)  // new size is smaller than current (last) size
		{
			split_block(currBlock,new_size);
    		//cprintf("\n>>>>>>>>>12 \n");
			return va;
		}
		else {
    		//cprintf("\n>>>>>>>>>13 \n");
			return va;
		}
	}
	//cprintf("\n>>>>>>>>>14 \n");
	return NULL;
}

