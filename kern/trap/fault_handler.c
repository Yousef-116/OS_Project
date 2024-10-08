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

bool alloc_and_read_from_file(uint32 fault_va)
{
	// allocate and map fault_va
	struct FrameInfo * targetPageFrame;
	allocate_frame(&targetPageFrame);
	map_frame(curenv->env_page_directory, targetPageFrame, fault_va, PERM_USER | PERM_WRITEABLE | PERM_PRESENT);
	targetPageFrame->va = fault_va;

	// read from disk
	int ret = pf_read_env_page(curenv, (void*)fault_va);
	if(ret == E_PAGE_NOT_EXIST_IN_PF)
	{
//		cprintf(">> Not in disk...");
		if(!(fault_va >= USTACKBOTTOM && fault_va < USTACKTOP) && !(fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX))
		{
//			cprintf(" not stack nor heap... so kill\n");
			unmap_frame(curenv->env_page_directory , fault_va);
			sched_kill_env(curenv->env_id);
			return 0;
		}
//		cprintf(">> but it's stack or heap... so it's ok\n");
	}
	return 1;
}

void shift_Active_list()
{
	// remove the last WSE from Active list and insert it to Second list
	struct WorkingSetElement* shift_Elm = LIST_LAST(&curenv->ActiveList);
	LIST_REMOVE(&curenv->ActiveList, shift_Elm);
	LIST_INSERT_HEAD(&curenv->SecondList, shift_Elm);
	pt_set_page_permissions(curenv->env_page_directory, shift_Elm->virtual_address, 0x000, PERM_PRESENT);
}


void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif

	fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
//	cprintf(">> fault_va = %x\n", fault_va);

	if(isPageReplacmentAlgorithmFIFO())
	{
		//FIFO Placement
		if(alloc_and_read_from_file(fault_va) == 0)
			return;

		if(wsSize < (curenv->page_WS_max_size))
		{
			struct WorkingSetElement* WSElem = env_page_ws_list_create_element(curenv, fault_va);
			LIST_INSERT_TAIL(&(curenv->page_WS_list), WSElem);

			// update page_last_WS_element for FIFO and clock algorithm
			if(LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
			{
				curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);
			}
		}
		//FIFO Replacement
		else
		{
			//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
			//refer to the project presentation and documentation for details
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");

			uint32 victim_va = curenv->page_last_WS_element->virtual_address;

#if USE_INV_O1
			uint32 *ptr_page_table = NULL;
			struct FrameInfo * fault_va_frame = get_frame_info(curenv->env_page_directory, fault_va, &ptr_page_table);
			fault_va_frame->element = curenv->page_last_WS_element;

			ptr_page_table =  NULL;
			struct FrameInfo * victim_va_frame = get_frame_info(curenv->env_page_directory, victim_va, &ptr_page_table);
			victim_va_frame->element = NULL;
#endif

			// update virtual_address of the WS
			curenv->page_last_WS_element->virtual_address = fault_va;

			// disk managing
			int perms = pt_get_page_permissions(curenv->env_page_directory, victim_va);
			if((perms & PERM_MODIFIED) || (victim_va >= USTACKBOTTOM && victim_va < USTACKTOP) || (victim_va >= USER_HEAP_START && victim_va <= USER_HEAP_MAX))
			{
//				cprintf(">> victim page is modified or stack or heap... ");
				uint32 *ptr_page_table;
				struct FrameInfo * modified_page_frame_info = get_frame_info(curenv->env_page_directory, victim_va, &ptr_page_table);
				pf_update_env_page(curenv, victim_va, modified_page_frame_info);
//				cprintf("updated in disk\n");
			}
//			else cprintf("victim page is not modified and not stack nor heap\n");

			// unmap the victim va
			unmap_frame(curenv->env_page_directory, victim_va);

			// update page_last_WS_element for FIFO and clock algorithm
			if(curenv->page_last_WS_element == LIST_LAST(&curenv->page_WS_list))
				curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);
			else
				curenv->page_last_WS_element = LIST_NEXT(curenv->page_last_WS_element);
		}
	}
	else if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

		struct WorkingSetElement* WSElem = NULL;
#if USE_INV_O1
		//search in the second list O(1)
		uint32 *ptr_page_table = NULL;
		struct FrameInfo * fault_va_frame = get_frame_info(curenv->env_page_directory, fault_va, &ptr_page_table);
		if(fault_va_frame != 0){
			WSElem= fault_va_frame->element;
		}
#else
		WSElem = get_WSE_from_list(&curenv->SecondList, fault_va);
#endif

		if(WSElem != NULL || LIST_SIZE(&curenv->ActiveList) + LIST_SIZE(&curenv->SecondList) < (curenv->page_WS_max_size))
		{
			// LRU placement
			if(WSElem != NULL) // exist in Second List
			{
//				cprintf(">> WS found in Second List... so move\n");

				// remove the last WS from Active list and insert it to Second list
				shift_Active_list();

				// remove the desired WS from Second list and insert in Active list
				LIST_REMOVE(&curenv->SecondList, WSElem);
				LIST_INSERT_HEAD(&curenv->ActiveList, WSElem);
				pt_set_page_permissions(curenv->env_page_directory, WSElem->virtual_address, PERM_PRESENT, 0x000);
			}
			else
			{
//				cprintf(">> WS not found in Second List... create new one\n");
				if(alloc_and_read_from_file(fault_va) == 0)
					return;

				if(LIST_SIZE(&curenv->ActiveList) == curenv->ActiveListSize)
				{
//					cprintf(">> Active list reach max... so shift\n");
					shift_Active_list();
				}

				WSElem = env_page_ws_list_create_element(curenv, fault_va);
				LIST_INSERT_HEAD(&curenv->ActiveList, WSElem);
			}
		}
		else
		{
			// LRU Replacement
			// not exist in second list.
//			cprintf("LRU replacement >> WS not found in Second List\n");

			if(alloc_and_read_from_file(fault_va) == 0)
				return;

			struct WorkingSetElement* victim = LIST_LAST(&curenv->SecondList);
			uint32 victim_va = victim->virtual_address;

#if USE_INV_O1
			if(fault_va_frame == 0){
				fault_va_frame = get_frame_info(curenv->env_page_directory, fault_va, &ptr_page_table);
			}
			fault_va_frame->element = victim;

			ptr_page_table =  NULL;
			struct FrameInfo * victim_va_frame = get_frame_info(curenv->env_page_directory, victim_va, &ptr_page_table);
			victim_va_frame->element = NULL;
#endif

			LIST_REMOVE(&curenv->SecondList, victim);

			uint32 perm = pt_get_page_permissions(curenv->env_page_directory, victim_va);
			if((perm & PERM_MODIFIED) || (victim_va >= USTACKBOTTOM && victim_va < USTACKTOP) || (victim_va >= USER_HEAP_START && victim_va <= USER_HEAP_MAX))
			{
//				cprintf(">> victim page is modified or stack or heap... ");
				uint32 *page_table;
				struct FrameInfo * modified_page_frame_info = get_frame_info(curenv->env_page_directory, victim_va, &page_table);
				pf_update_env_page(curenv, victim_va, modified_page_frame_info);
//				cprintf("updated in disk\n");
			}
//			else cprintf("victim page is not modified and not stack nor heap\n");

			// unmap the victim va
			unmap_frame(curenv->env_page_directory, victim_va);

			// remove the last WS from Active list and insert it to Second list
			shift_Active_list();

			victim->virtual_address = fault_va;
			LIST_INSERT_HEAD(&curenv->ActiveList, victim);
			pt_set_page_permissions(curenv->env_page_directory, victim->virtual_address, PERM_PRESENT, 0x000);
		}

		//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
	}

	// print WS list
//	cprintf(">> list after\n");
//	env_page_ws_print(curenv);
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



