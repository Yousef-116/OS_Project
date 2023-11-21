#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
const int umanga_size = (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;
int umanga_strt;
int umanga[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE] = {};

int Uva_to_index(void* va) {
	return ((uint32) va - USER_HEAP_START) / PAGE_SIZE;
}

void* index_to_Uva(int index) {
	return (void *) (index * PAGE_SIZE + USER_HEAP_START);
}

uint32 numOfUnmarkedPages;
int user_hLimit;
int isIntialized = 0;
void init_uheap()
{
	isIntialized = 1;
	user_hLimit = sys_getHardLimit();
	umanga_strt = (user_hLimit + PAGE_SIZE - USER_HEAP_START) / PAGE_SIZE;
	umanga[umanga_strt] = -(umanga_size - umanga_strt);
	umanga[umanga_size - 1] = umanga[umanga_strt];  // at the end of Brothers
	numOfUnmarkedPages = (USER_HEAP_MAX - user_hLimit - PAGE_SIZE) / PAGE_SIZE;

	//cprintf("user_hLimit = %x, umanga_strt = %d\n\n", user_hLimit, umanga_strt);
}

int ctr = 1;

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

	if(!isIntialized)
	{
		init_uheap();
	}

	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
		if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			//cprintf("%d) dynamic allocator , size = %d\n",ctr++, size);
			return alloc_block_FF(size);
		}

		int num_of_req_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		if (num_of_req_pages > numOfUnmarkedPages) {
			//cprintf("not enought pages: %d < %d\n",numOfFreePages, num_of_req_pages);
			return NULL;
		}
		//cprintf("size = %d, num_of_req_pages = %d\n", size, num_of_req_pages);

		uint32 *ptr_page_table = NULL;
		void* _1stVa;
		int index = -1;
		for (int i = umanga_strt; i < umanga_size;)
		{
			if (umanga[i] < 0)
			{
				if (-umanga[i] == num_of_req_pages)
				{
					umanga[i - umanga[i] - 1] = 0;  // at the end of Brothers
					umanga[i] = num_of_req_pages;
					index = i;
					break;
				}
				else if (-umanga[i] > num_of_req_pages)
				{
					umanga[i + num_of_req_pages] = umanga[i] + num_of_req_pages;
					umanga[i + num_of_req_pages - umanga[i + num_of_req_pages] - 1] = umanga[i + num_of_req_pages];

					umanga[i] = num_of_req_pages;  // marked
					index = i;
					break;
				}
				else {
					i -= umanga[i];
				}
			}
			else {
				i += umanga[i];
			}
		}

		if (index == -1) {
			return NULL;
		}

		//cprintf("index = %d\n", index);
		_1stVa = index_to_Uva(index);
		numOfUnmarkedPages -= num_of_req_pages;
		sys_allocate_user_mem((uint32)_1stVa, size);

		return _1stVa;
	}
	else if(sys_isUHeapPlacementStrategyBESTFIT())
	{
		panic("Not implemented yet");
	}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	if((uint32)virtual_address >= USER_HEAP_START && (uint32)virtual_address <(uint32)sbrk(0)) // block allocator
	{
		cprintf("sbrk ======================= %x\n\n\n\n\n\n" , sbrk(0));
		free_block(virtual_address);
	}else if ((uint32)virtual_address >=user_hLimit + PAGE_SIZE && (uint32)virtual_address < USER_HEAP_MAX)// page allocator
	{
		int index = Uva_to_index(virtual_address);
		uint32 size = umanga[index] * PAGE_SIZE;
		umanga[index] *= -1;
		sys_free_user_mem((uint32) virtual_address , size);
	}else
		panic("invalid address (free (USER)) \n");
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
