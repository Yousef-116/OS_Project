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

void setVBlock0(struct BlockMetaData *currMetaData)
{
	currMetaData->is_free = 0;
	currMetaData->size = 0;
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

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
struct MemBlock_LIST MemoryList;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");
	//cprintf("init size=%d\n metaDataSize=%d\n", initSizeOfAllocatedSpace, sizeOfMetaData());

	struct BlockMetaData *metaData = (struct BlockMetaData *) daStart;
	//cprintf("init address=%p +sizeOfMetaData=%p\n", metaData, metaData+1);

	metaData->size=initSizeOfAllocatedSpace;
	metaData->is_free=1;
	metaData->prev_next_info.le_next=NULL;
	metaData->prev_next_info.le_prev=NULL;

	//struct MemBlock_LIST head;
//	head.lh_first=metaData;
//	head.lh_last=metaData;
//	head.___ptr_next=NULL;
//	head.size=1;

	LIST_INIT(&MemoryList);
	LIST_INSERT_HEAD(&MemoryList, metaData);
	LIST_LAST(&MemoryList) = metaData;
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
int done = 1;
void *alloc_block_FF(uint32 size)
{
	//cprintf("called.. size=%d\n", size);
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
    //panic("alloc_block_FF is not implemented yet");
    if(size==0) return NULL;

    struct BlockMetaData *currMetaData;
    uint32 emptySpace, remSpace;
   // int ctr = 1;
    LIST_FOREACH(currMetaData, &MemoryList)
    {
    	//cprintf("B%d BlockSize=%u\n", ctr++, currMetaData->size);
    	// found suitable size
    	if(currMetaData->is_free)
    	{
    		emptySpace = currMetaData->size - sizeOfMetaData();
    		if(emptySpace >= size)
    		{
    			currMetaData->is_free = 0;
    			remSpace = emptySpace - size;
    			//cprintf("remSpace=%u\n", remSpace);

    			if(remSpace > sizeOfMetaData()) // there is a space for another MetaData
    			{
    				currMetaData->size -= remSpace;

    				struct BlockMetaData *newMetaData = (struct BlockMetaData *)((uint32)currMetaData + size + sizeOfMetaData());
    				newMetaData->is_free = 1;
    				newMetaData->size = remSpace;
    				LIST_INSERT_AFTER(&MemoryList, currMetaData, newMetaData);

    				//cprintf("newBlockSize=%u\n", newMetaData->size);
    				//cprintf("done%d\n", done++);
    			}

    			//cprintf("address=%p +sizeOfMetaData=%p\n\n", currMetaData, currMetaData+1);
    			return (currMetaData + 1);
    		}
    	}
    }

    if(sbrk(size) != (void*)-1){
    	//cprintf("sbrk\n");
    	return alloc_block_FF(size);
    }
    //cprintf("failed\n");

    return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	panic("alloc_block_BF is not implemented yet");
	return NULL;
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

	struct BlockMetaData *currMetaData = ((struct BlockMetaData *)va - 1);
	struct BlockMetaData *first_element = LIST_FIRST(&MemoryList);
	struct BlockMetaData *last_element = LIST_LAST(&MemoryList);


	if(!(currMetaData == first_element || currMetaData == last_element))
	{
		if(currMetaData->prev_next_info.le_next->is_free == 1 && currMetaData->prev_next_info.le_prev->is_free == 1) //next and prev  are empty
		{
			currMetaData->prev_next_info.le_prev->size += currMetaData->size + currMetaData->prev_next_info.le_next->size;
			setVBlock0(currMetaData->prev_next_info.le_next);
			setVBlock0(currMetaData);
			//LIST_REMOVE(&MemoryList ,currMetaData->prev_next_info.le_next);
			//LIST_REMOVE(&MemoryList ,currMetaData);
		}
		else if(currMetaData->prev_next_info.le_next->is_free == 0 && currMetaData->prev_next_info.le_prev->is_free == 0)//next and prev  are not empty
		{
			currMetaData->is_free = 1;
		}else if(currMetaData->prev_next_info.le_next->is_free == 1 && currMetaData->prev_next_info.le_prev->is_free == 0)// next is empty
		{
			currMetaData->size += currMetaData->prev_next_info.le_next->size;
			currMetaData->is_free = 1;
			setVBlock0(currMetaData->prev_next_info.le_next);
			//LIST_REMOVE(&MemoryList ,currMetaData->prev_next_info.le_next);
		}else if(currMetaData->prev_next_info.le_next->is_free == 0 && currMetaData->prev_next_info.le_prev->is_free == 1) // prev is empty
		{
			currMetaData->prev_next_info.le_prev->size += currMetaData->size;
			setVBlock0(currMetaData);

			//LIST_REMOVE(&MemoryList ,currMetaData);
		}
	}
	else if(currMetaData == first_element)
	{
		if(currMetaData->prev_next_info.le_next->is_free == 1)
		{
			currMetaData->size += currMetaData->prev_next_info.le_next->size;
			currMetaData->is_free = 1;
			setVBlock0(currMetaData->prev_next_info.le_next);
		}
		else
		{
			currMetaData->is_free = 1;
		}

	}
	else if(currMetaData == last_element)
	{
		if(currMetaData->prev_next_info.le_prev->is_free == 1)
		{
			currMetaData->prev_next_info.le_prev->size += currMetaData->size;
			setVBlock0(currMetaData);
		}
		else
		{
			currMetaData->is_free = 1;
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
		return NULL;
	}
	else if(va == NULL )
	{
		return alloc_block_FF(new_size);
	}
	else if( new_size == 0 )
	{
	    free_block(va);
	    return NULL;
	}

	struct BlockMetaData *currMetaData = ((struct BlockMetaData *)va - 1);
	uint32 totalSize = 0 ;
	struct BlockMetaData *NextMetaData = LIST_NEXT(currMetaData);
	while(NextMetaData != NULL && NextMetaData->size == 0)  // get the really next node
	{
		NextMetaData = LIST_NEXT(NextMetaData);
	}

	if(NextMetaData != NULL)  // if it has a next node
	{
		if(currMetaData->size < new_size)  // new size is bigger than current size
		{
			if(NextMetaData->is_free)   // if next free
			{
				totalSize = currMetaData->size + NextMetaData->size ;
		    	if(totalSize == new_size)    // if next free and total size == new size
		    	{
		    		currMetaData->size = new_size;
		    		setVBlock0(NextMetaData);
		    		return va;
		    	}
		    	else if (totalSize > new_size)    // if next free and total size > new size
		    	{
		    		/*
		    		currMetaData->size = new_size + sizeOfMetaData();
		    		NextMetaData->size = totalSize - new_size;
		    		NextMetaData = (struct BlockMetaData *)((uint32)currMetaData + currMetaData->size);
		    		//NextMetaData = (struct BlockMetaData *)((uint32)currMetaData + new_size + sizeOfMetaData());
		    		return va;
		    		*/

		    		currMetaData->size = new_size + sizeOfMetaData();
		    		setVBlock0(NextMetaData);
					struct BlockMetaData *newMetaData = (struct BlockMetaData *)((uint32)currMetaData + currMetaData->size);
					newMetaData->is_free = 1;
					newMetaData->size = totalSize - new_size;
					LIST_INSERT_AFTER(&MemoryList, currMetaData, newMetaData);
					return va;
		    	}
		    	else if (totalSize < new_size)    // if next free and total size < new size call free bloc and after allocate the bloc by FF
		    	{
		    		free_block(va);
		    		return alloc_block_FF(new_size);
		    	}

			}
			else {  // next is not free
				free_block(va);
				return alloc_block_FF(new_size);
			}
		}
		else if ( currMetaData->size > new_size )  // new size is smaller than current size
		{
			if(NextMetaData->is_free == 0)   // next is not free  (full)
			{
				struct BlockMetaData *newMetaData = (struct BlockMetaData *)((uint32)currMetaData + new_size + sizeOfMetaData());
				newMetaData->is_free = 1;
				newMetaData->size = currMetaData->size - new_size- sizeOfMetaData();
				currMetaData->size = new_size + sizeOfMetaData();
				LIST_INSERT_AFTER(&MemoryList, currMetaData, newMetaData);
			}
			else // next is free
			{
				NextMetaData->size += (currMetaData->size - new_size);
				currMetaData->size = new_size + sizeOfMetaData();
				NextMetaData = (struct BlockMetaData *)((uint32)currMetaData + currMetaData->size);
			}

			return va;
		}
		else if ( currMetaData->size > new_size ) // new size is equal to current size
		{
			return va;
		}

	}
	else   // if the node is at the tail or at the its just one node and head and tail
	{
		free_block(va);
		return alloc_block_FF(new_size);
	}

	return NULL;
}

