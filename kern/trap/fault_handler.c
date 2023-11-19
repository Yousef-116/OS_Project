/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif

	if(wsSize < (curenv->page_WS_max_size))
	{
		//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
		//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");

//		fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
//		struct FrameInfo * targetPageFrame;
//		allocate_frame(&targetPageFrame);
//		map_frame(curenv->env_page_directory, targetPageFrame, fault_va, PERM_USER | PERM_PRESENT | PERM_WRITEABLE);
//		int ret = pf_read_env_page(curenv, (void*)fault_va);
//		if(ret == E_PAGE_NOT_EXIST_IN_PF)
//		{
//			if((fault_va > USTACKBOTTOM && fault_va < USTACKTOP) || (fault_va > USER_HEAP_START && fault_va < USER_HEAP_MAX))
//			{
//				pf_add_empty_env_page(curenv, fault_va, 0);
//			}
//			else sched_kill_env(curenv->env_id);
//		}
//
//		struct WorkingSetElement* WSElem = env_page_ws_list_create_element(curenv, fault_va);
//		cprintf("SIZE BEFORE: %d \n", LIST_SIZE(&(curenv->page_WS_list)));
//		LIST_INSERT_TAIL(&(curenv->page_WS_list), WSElem);
//		cprintf("SIZE AFTER: %d \n", LIST_SIZE(&(curenv->page_WS_list)));
//		cprintf("MAX SIZE: %d \n", curenv->page_WS_max_size);
//		if(LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
//		{ // full
//			//curenv->page_last_WS_element++;
//		}
//		else curenv->page_last_WS_element = NULL;
//		env_page_ws_print(curenv);

		bool alloc = 1;
		int ret = pf_read_env_page(curenv, (void*)fault_va);
		if(ret == E_PAGE_NOT_EXIST_IN_PF)
		{
			if(!(fault_va > USTACKBOTTOM && fault_va < USTACKTOP) && !(fault_va > USER_HEAP_START && fault_va < USER_HEAP_MAX))
			{
				alloc = 0;
				sched_kill_env(curenv->env_id);
			}
		}
		if(alloc == 1)
		{
			struct FrameInfo * targetPageFrame;
			allocate_frame(&targetPageFrame);
			map_frame(curenv->env_page_directory, targetPageFrame, fault_va, PERM_USER | PERM_PRESENT | PERM_WRITEABLE);
			struct WorkingSetElement* WSElem = env_page_ws_list_create_element(curenv, fault_va);
			LIST_INSERT_TAIL(&(curenv->page_WS_list), WSElem);
		}


		//refer to the project presentation and documentation for details
	}
	else
	{
		//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
		//refer to the project presentation and documentation for details
		if(isPageReplacmentAlgorithmFIFO())
		{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
			panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
		}
		if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
		{
			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
			// Write your code here, remove the panic and write your code
			panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

			//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
		}
	}
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



