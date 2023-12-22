#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

uint32 numOfFreePages;
const int kmanga_size = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE;
int kmanga_strt;
int kmanga[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE + 1] = { };

int Kva_to_index(void* va) {
	return ((uint32) va - KERNEL_HEAP_START) / PAGE_SIZE;
}

void* index_to_Kva(int index) {
	return (void *) (index * PAGE_SIZE + KERNEL_HEAP_START);
}

void myAlloc_pages(void* virtual_address, int num_of_req_pages)
{
	for (uint32 va = (uint32) virtual_address; num_of_req_pages > 0; va += PAGE_SIZE, --num_of_req_pages) {
		struct FrameInfo *ptr_frame_info;
		int ret = allocate_frame(&ptr_frame_info);
		ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT | PERM_WRITEABLE);
		ptr_frame_info->va = va;
	}
}

void *kmalloc_and_kfree(void* va, uint32 new_size) {
	void * ret = kmalloc(new_size);
	if (ret != NULL)  //allocation succeeded
	{
		kfree(va);
	}
	return ret;
}

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit) {
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");

	if (daStart + initSizeToAllocate <= daLimit) {
		start = daStart;
		brk = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
		brk += daStart;

		hLimit = daLimit;
		numOfFreePages = (KERNEL_HEAP_MAX - hLimit - PAGE_SIZE) / PAGE_SIZE;
		kmanga_strt = (hLimit + PAGE_SIZE - KERNEL_HEAP_START) / PAGE_SIZE;
//		kmanga[kmanga_strt] = -(kmanga_size - kmanga_strt);
//		kmanga[kmanga_size - 1] = kmanga[kmanga_strt];  // at the end of Brothers

		for (uint32 va = daStart; va < brk; va += PAGE_SIZE)
		{
			struct FrameInfo *ptr_frame_info;
			int ret = allocate_frame(&ptr_frame_info);
			ret = map_frame(ptr_page_directory, ptr_frame_info, va,
			PERM_WRITEABLE | PERM_PRESENT);
			ptr_frame_info->va = va;
		}

		initialize_dynamic_allocator(daStart, initSizeToAllocate);
		return 0;
	}

	return E_NO_MEM;
}

void* sbrk(int increment) {
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
	uint32 new_brk = old_brk;
	if (increment > 0)
	{
		new_brk += increment;
		uint32 diff = new_brk - start;

		if (diff % PAGE_SIZE != 0){
			new_brk = ROUNDUP(diff, PAGE_SIZE) + start;
		}

		if (new_brk > hLimit ){
			panic("\nERROR_1 - brk + increment > hLimit\n");
		}
		else {
			brk = new_brk;
		}


		uint32 strt = old_brk;
		diff = strt - start;
		if (diff % PAGE_SIZE != 0)
		{
			strt = ROUNDUP(diff, PAGE_SIZE) + start;
		}
		for (uint32 i = strt; i < brk; i += PAGE_SIZE) // allocate all frames between old_brk & new brk
		{
			struct FrameInfo *ptr_frame_info = NULL;

			int ret = allocate_frame(&ptr_frame_info);
			if (ret == E_NO_MEM)
				panic("\nERROR_2 - cannot allocate frame, no memory\n");

			ret = map_frame(ptr_page_directory, ptr_frame_info, i, PERM_WRITEABLE | PERM_PRESENT);
			ptr_frame_info->va = i;
			if (ret == E_NO_MEM) {
				free_frame(ptr_frame_info);
				panic("\nERROR_3 - cannot map to frame, no memory\n");
			}

			ptr_frame_info->va = i;
		}


		return (void *) old_brk;
	}
	else if (increment < 0)
	{
		//panic("\nERROR_4 - increment < 0 not implemented yet\n");

		//increment = ROUNDDOWN(increment, PAGE_SIZE);

		if (brk + increment < start)
			panic("\nERROR_5 - brk + increment < start\n");

		new_brk += increment;

		uint32 diff = new_brk - start;
		uint32 temp_brk = ROUNDUP(diff, PAGE_SIZE) + start;
		uint32 *ptr_page_table;
		struct FrameInfo* ptr_frame_info;

		while (temp_brk < old_brk) {
			ptr_frame_info = get_frame_info(ptr_page_directory, temp_brk, &ptr_page_table);
			if (ptr_frame_info == NULL)
				panic("\nERROR_6 - cannot find frame to free\n");
			//pf_remove_env_page(curenv, temp_brk); //remove from page file (disk) -- can't see func or curenv
			unmap_frame(ptr_page_directory, temp_brk);
			temp_brk += PAGE_SIZE;
		}

		/*
		struct BlockMetaData *meta_data = LIST_LAST(&MemoryList);
		while ((uint32) meta_data >= (uint32) new_brk) // remove any metaData above or equals new brk
		{
			LIST_REMOVE(&MemoryList, meta_data);
			meta_data = LIST_LAST(&MemoryList);
		}

		meta_data->size = new_brk - (uint32) meta_data; // last metaData under new brk - size equals space in between
		*/

		brk = new_brk;
		return (void *) new_brk;
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
	if (isKHeapPlacementStrategyFIRSTFIT())
	{
		if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			//cprintf("dynamic allocator\n");
			void* ret =  alloc_block_FF(size);
			if(ret == NULL)
				panic(">> Kernel can't allocate memory\n");
			return ret;
		}

		int num_of_req_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		if (num_of_req_pages > numOfFreePages) {
			//cprintf("not enought pages: %d < %d\n",numOfFreePages, num_of_req_pages);
			return NULL;
		}

		void* _1stVa;
		int index = -1;

		// old Art
		/*
		for (int i = kmanga_strt; i < kmanga_size;) {
			if (kmanga[i] < 0)
			{
				if (-kmanga[i] == num_of_req_pages)
				{
					kmanga[i - kmanga[i] - 1] = 0;  // at the end of Brothers
					kmanga[i] = num_of_req_pages;
					index = i;
					break;
				}
				else if (-kmanga[i] > num_of_req_pages)
				{
					kmanga[i + num_of_req_pages] = kmanga[i] + num_of_req_pages;
					kmanga[i + num_of_req_pages - kmanga[i + num_of_req_pages] - 1] =
							kmanga[i + num_of_req_pages];

					kmanga[i] = num_of_req_pages;

					index = i;
					break;
				}
				else {
					i -= kmanga[i];
				}
			}
			else {
				i += kmanga[i];
			}
		}
		*/

		int ctr = 0;
		for(int i = kmanga_strt; i<kmanga_size; ++i)
		{
			if(kmanga[i] > 0)
			{
				ctr = 0;
				i += kmanga[i]-1;
			}
			else
			{
				if(ctr == 0) index = i;
				ctr++;
				if(ctr == num_of_req_pages)
				{
//					cprintf("\n>>in kmalloc num_of_req_pages = %d.. marked pages %d-%d\n\n", num_of_req_pages, index, i);
					break;
				}
			}
		}

		if (ctr != num_of_req_pages) {
//			return NULL;
			panic(">> Kernel can't allocate memory\n");
		}

		kmanga[index] = num_of_req_pages;
		//_1stVa = index*PAGE_SIZE + start;
		_1stVa = index_to_Kva(index);
		numOfFreePages -= num_of_req_pages;
		myAlloc_pages(_1stVa, num_of_req_pages);
		return _1stVa;
	}

	return NULL;
}

void kfree(void* virtual_address) {
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	uint32 *ptr_page_table = NULL;
	struct FrameInfo *ptr_frame_info = virtual_address;

	if ((uint32) virtual_address >= start && (uint32) virtual_address < brk) // block area
	{
		//cprintf(">> in kfree block allocator, va = %x\n", virtual_address);
		return free_block(virtual_address);
	}
	else if ((uint32) virtual_address >= (hLimit + PAGE_SIZE)&& (uint32) virtual_address <= KERNEL_HEAP_MAX - PAGE_SIZE)
	{
//		int index = ((uint32)virtual_address  - KERNEL_HEAP_START) / PAGE_SIZE;
		int index = Kva_to_index(virtual_address);
		//cprintf("index = %d \n" , kmanga[index]);
		uint32 noOfBrothers = kmanga[index];

		if (noOfBrothers == 0)
			return;

		for (uint32 va = (uint32) virtual_address; noOfBrothers > 0; --noOfBrothers, va += PAGE_SIZE) {
			unmap_frame(ptr_page_directory, va);
		}

		numOfFreePages += kmanga[index];
//		kmanga[index] *= -1;
		kmanga[index] = 0;

		// old Art
		/*
		if (kmanga[index - kmanga[index]] < 0) // next are free -> merge
		{
			int i = index - kmanga[index];
			kmanga[index] += kmanga[i];
			kmanga[i] = 0;
		}
		if (kmanga[index - 1] < 0) // prev are free -> merge
		{
			kmanga[index + kmanga[index - 1]] += kmanga[index];
			kmanga[index] = 0;
			int i = kmanga[index - 1];
			kmanga[index - 1] = 0;
			index += i;
		}
		kmanga[index - kmanga[index] - 1] = kmanga[index]; // at the end of Brothers
		*/

	} else {
		panic("Invalid address");
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address) {
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	struct FrameInfo *ptr_frame_info = to_frame_info(physical_address);
	if (ptr_frame_info->references == 0)
		return 0;
	else
	{
		uint32 mask = 0xFFF;
		uint32 offset = physical_address & mask;

		return (ptr_frame_info->va) + offset;
	}
}
unsigned int kheap_physical_address(unsigned int virtual_address) {
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32 *ptr_page_table = NULL;
	unsigned int ret = get_page_table(ptr_page_directory, virtual_address,
			&ptr_page_table);
	if (ret == TABLE_IN_MEMORY) {
		uint32 table_entry = ptr_page_table[PTX(virtual_address)];

		uint32 frame_number = table_entry >> 12;

		uint32 mask = 0xFFF;

		uint32 offset = virtual_address & mask;

		return (frame_number * PAGE_SIZE) + offset;
	}

	return 0;
}

void kfreeall() {
	panic("Not implemented!");

}

void kshrink(uint32 newSize) {
	panic("Not implemented!");
}

void kexpand(uint32 newSize) {
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

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
//	return NULL;
//	panic("krealloc() is not implemented yet...!!");


	if (virtual_address == NULL && new_size == 0) {
		return NULL;
	}
	else if (virtual_address == NULL) {
		return kmalloc(new_size);
	}
	else if (new_size == 0) {
		kfree(virtual_address);
		return NULL;
	}

	int index = Kva_to_index(virtual_address);
	if (kmanga[index] == 0) {  // is free
		return NULL;
	}

	if ((uint32) virtual_address < brk)
	{
		//cprintf("dynamic allocator\n");
		if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			return realloc_block_FF(virtual_address, new_size);
		}
		else
		{
			void* ret = kmalloc(new_size);
			if (ret != NULL) {
				free_block(virtual_address);
				return ret;
			}
			else {
				return virtual_address;
			}
		}
	}
	else
	{
		if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			void * ret = alloc_block_FF(new_size);
			if(ret != NULL)
			{
				kfree(virtual_address);
				return ret;
			}
		}
	}

	int num_of_req_pages = ROUNDUP(new_size, PAGE_SIZE) / PAGE_SIZE;
	if (kmanga[index] == num_of_req_pages){
		return virtual_address;
	}
	else if (kmanga[index] < num_of_req_pages)  //new size is greater
	{
		int nxt_index = index + kmanga[index];int ctr = 0;
		for(int i = nxt_index; i<kmanga_size; ++i)
		{
			if(kmanga[i] > 0 || ctr == num_of_req_pages)
				break;
			ctr++;
		}
		if(ctr == num_of_req_pages) {
			kmanga[index] = num_of_req_pages;
		}
		else
		{
			void *new_va =  kmalloc_and_kfree(virtual_address, new_size);
			if(new_va != NULL)
			{
				// Here we should transfer the data.
				return new_va;
			}
		}
		return virtual_address;
	}
	else if (kmanga[index] > num_of_req_pages)  //new size is smaller
	{
		int remain_pages = kmanga[index] - num_of_req_pages;
		kmanga[index] = num_of_req_pages;
		kmanga[index + num_of_req_pages] = remain_pages;
		kfree(index_to_Kva(index + num_of_req_pages));
		return virtual_address;
	}

	// old Art
	/*
	if (index + kmanga[index] < kmanga_size) //not the last
	{
		if (kmanga[index] < num_of_req_pages)  //new size is greater
		{
			int nxt_index = index + kmanga[index];
			if (kmanga[nxt_index] < 0) // next is free
			{
				if (kmanga[index] + -kmanga[nxt_index] == num_of_req_pages)
				{
					int page_num = num_of_req_pages - kmanga[index];
					myAlloc_pages(index_to_Kva(nxt_index), page_num);

					kmanga[index] = num_of_req_pages;
					kmanga[nxt_index - kmanga[nxt_index] - 1] = 0;
					kmanga[nxt_index] = 0;
					return virtual_address;
				}
				else if (kmanga[index] + -kmanga[nxt_index] > num_of_req_pages)
				{
					int page_num = num_of_req_pages - kmanga[index];
					myAlloc_pages(index_to_Kva(nxt_index), page_num);

					kmanga[index] = num_of_req_pages;
					kmanga[nxt_index - kmanga[nxt_index] - 1] += page_num;
					kmanga[index + num_of_req_pages] = kmanga[nxt_index- kmanga[nxt_index] - 1];
					kmanga[nxt_index] = 0;
					return virtual_address;
				}
				else {
					return kmalloc_and_kfree(virtual_address, new_size);
				}
			}
			else {
				return kmalloc_and_kfree(virtual_address, new_size);
			}
		}
		if (kmanga[index] > num_of_req_pages)  //new size is smaller
		{

			int remain_pages = kmanga[index] - num_of_req_pages;
			kmanga[index] = num_of_req_pages;
			kmanga[index + num_of_req_pages] = remain_pages;
			kfree(index_to_Kva(index + num_of_req_pages));
			return virtual_address;
		}
	}
	else  // last
	{
		if (kmanga[index] < num_of_req_pages) {
			return kmalloc_and_kfree(virtual_address, new_size);
		}
		else {
			int remain_pages = kmanga[index] - num_of_req_pages;
			kmanga[index] = num_of_req_pages;
			kmanga[index + num_of_req_pages] = remain_pages;
			kfree(index_to_Kva(index + num_of_req_pages));
			return virtual_address;
		}
	}
	*/

	return NULL;
}
