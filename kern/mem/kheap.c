#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

uint32 numOfFreePages;
const int manga_size = (KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE;
int manga_strt;
int manga[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE + 1] = {};

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");


	if(daStart + initSizeToAllocate <= daLimit)
	{
		start = daStart;
		brk = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
		brk += daStart;

		hLimit = daLimit;
		numOfFreePages = (KERNEL_HEAP_MAX - hLimit - PAGE_SIZE)/PAGE_SIZE;
		manga_strt = (hLimit + PAGE_SIZE - KERNEL_HEAP_START) / PAGE_SIZE;
		manga[manga_strt] = manga_strt - manga_size;
		manga[manga_size - 1] = manga[manga_strt];  // at the end of Brothers

		for(uint32 va = daStart; va <= brk; va += PAGE_SIZE)
		{
			struct FrameInfo *ptr_frame_info;
			int ret = allocate_frame(&ptr_frame_info);
			ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
			ptr_frame_info->va = va;
		}

		initialize_dynamic_allocator(daStart, initSizeToAllocate);
//		cprintf("KheapMax - hLimit - 4K = %x\n", KERNEL_HEAP_MAX - hLimit - PAGE_SIZE);
//		cprintf("(KheapMax - hLimit - 4K)/PAGE_SIZE = %d\n", (KERNEL_HEAP_MAX - hLimit - PAGE_SIZE)/PAGE_SIZE);
//		cprintf("NUM_OF_KHEAP_PAGES = %d\n", NUM_OF_KHEAP_PAGES);
		return 0;
	}

	return E_NO_MEM;
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//return (void*)-1 ;
	//panic("not implemented yet");

	uint32 old_brk = brk;
	if (increment > 0)
	{
		increment = ROUNDUP(increment, PAGE_SIZE);

		if (brk + increment >= hLimit)
			panic("\nERROR_1 - brk + increment >= hLimit\n");

		brk += increment;

		struct FrameInfo *ptr_frame_info = NULL;
		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM)
			panic("\nERROR_2 - cannot allocate frame, no memory\n");

		ret = map_frame(ptr_page_directory, ptr_frame_info, old_brk, PERM_WRITEABLE);
		if (ret == E_NO_MEM)
		{
			free_frame(ptr_frame_info);
			panic("\nERROR_3 - cannot map to frame, no memory\n");
		}

		ptr_frame_info->va = old_brk;
		struct BlockMetaData *meta_data = (struct BlockMetaData *) (old_brk);
		meta_data->size = PAGE_SIZE;
		meta_data->is_free = 1;
		LIST_INSERT_TAIL(&MemoryList, meta_data);
		return (void *) old_brk;
	}
	else if (increment < 0)
	{
		//panic("\nERROR_4 - increment < 0 not implemented yet\n");

		increment = ROUNDDOWN(increment, PAGE_SIZE);

		if (brk + increment <= start)
			panic("\nERROR_5 - brk + increment <= start\n");

		brk += increment;

		struct FrameInfo *ptr_frame_info = to_frame_info(brk);
		if (ptr_frame_info == NULL)
			panic("\nERROR_6 - cannot find frame to free\n");

		struct BlockMetaData *meta_data = LIST_LAST(&MemoryList);

		if ((uint32)meta_data == (uint32)brk)
		{
			LIST_REMOVE(&MemoryList, meta_data);
		}
		else if (meta_data->is_free == 1)
		{
			meta_data->size -= PAGE_SIZE;
		}

		unmap_frame(ptr_page_directory, brk);
		free_frame(ptr_frame_info);

		return (void *) old_brk;

	}
	else // increment == 0
	{
		return (void *) old_brk;
	}

}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//cprintf("size = %u\n", size);
	if(isKHeapPlacementStrategyFIRSTFIT())
	{
		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			//cprintf("dynamic allocator\n");
			return alloc_block_FF(size);
		}

		uint32 _size = size;
		int num_of_req_pages = ROUNDUP(_size, PAGE_SIZE) / PAGE_SIZE;
		if(num_of_req_pages > numOfFreePages){
			//cprintf("not enought pages: %d < %d\n",numOfFreePages, num_of_req_pages);
			return NULL;
		}

		uint32 *ptr_page_table = NULL;
		uint32 _1stVa;
		int index = -1;
		for(int i = manga_strt; i < manga_size; )
		{
			if(manga[i] < 0)
			{
				if(-manga[i] == num_of_req_pages)
				{
					manga[i - manga[i] - 1] = 0;  // at the end of Brothers
					manga[i] = num_of_req_pages;
					index = i;
					break;
				}
				else if(-manga[i] > num_of_req_pages)
				{
					manga[i + num_of_req_pages] = manga[i] + num_of_req_pages;
					manga[i + num_of_req_pages - manga[i + num_of_req_pages] - 1] = manga[i + num_of_req_pages];

					manga[i] = num_of_req_pages;

					index = i;
					break;
				}
				else
				{
					i -= manga[i];
				}
			}
			else {
				i += manga[i];
			}
		}

		if(index == -1){
			return NULL;
		}

		manga[index] = num_of_req_pages;
		_1stVa = index*PAGE_SIZE + start;
		numOfFreePages -= num_of_req_pages;

		for(uint32 va = _1stVa; num_of_req_pages > 0; va += PAGE_SIZE, --num_of_req_pages)
		{
			struct FrameInfo *ptr_frame_info;
			int ret = allocate_frame(&ptr_frame_info);
			ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
			ptr_frame_info->va = va;
		}
		return (void *)(_1stVa);

	}

	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	uint32 *ptr_page_table = NULL;
	struct FrameInfo *ptr_frame_info = virtual_address;

	if((uint32) virtual_address >= start && (uint32) virtual_address <= brk) // block area
	{
		return free_block(virtual_address);
	}
	else if((uint32) virtual_address >= (hLimit + PAGE_SIZE) && (uint32) virtual_address <= KERNEL_HEAP_MAX - PAGE_SIZE)
	{
		int index = ((uint32)virtual_address  - KERNEL_HEAP_START) / PAGE_SIZE;
		//cprintf("index = %d \n" , manga[index]);
		uint32 noOfBrothers =  manga[index];

		if(noOfBrothers == 0) return;

		for(uint32 va = (uint32)virtual_address ;noOfBrothers > 0 ; --noOfBrothers , va += PAGE_SIZE)
		{
			unmap_frame(ptr_page_directory ,va);
		}

		numOfFreePages += manga[index];
		manga[index] *= -1;

		if(manga[index - manga[index]] < 0) // next are free -> merge
		{
			int i = index - manga[index];
			manga[index] += manga[i];
			manga[i] = 0;
		}
		if(manga[index - 1] < 0) // prev are free -> merge
		{
			manga[index + manga[index - 1]] += manga[index];
			manga[index] = 0;
			int i = manga[index - 1];
			manga[index - 1] = 0;
			index += i;
		}
		manga[index - manga[index] - 1] = manga[index];  // at the end of Brothers

	}
	else
	{
		panic("Invalid address");
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	struct FrameInfo *ptr_frame_info = to_frame_info(physical_address);

//	cprintf("va  = %x \n" ,ptr_frame_info->va);
//	cprintf("ref  = %x \n" ,ptr_frame_info->references);
	//cprintf("pa actual = %x \n" ,physical_address);

	if(ptr_frame_info->references == 0)	return 0;
	else
	{
		uint32 mask = 0xFFF;

		uint32 offset = physical_address & mask;

//		cprintf("offset = %x \n" , offset);


		//cprintf("virtual = %x \n" ,(ptr_frame_info->va) + offset);

        return (ptr_frame_info->va) + offset;
	}
	//change this "return" according to your answer
	//return 0;
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32 *ptr_page_table = NULL;
	unsigned int ret = get_page_table(ptr_page_directory, virtual_address, &ptr_page_table);
	if (ret == TABLE_IN_MEMORY)
	{
		uint32 table_entry = ptr_page_table[PTX(virtual_address)];

		uint32 frame_number = table_entry >> 12;

		uint32 mask = 0xFFF;

		uint32 offset = virtual_address & mask;

		return (frame_number * PAGE_SIZE) + offset;
	}

	//change this "return" according to your answer
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
