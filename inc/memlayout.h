#ifndef FOS_INC_MEMLAYOUT_H
#define FOS_INC_MEMLAYOUT_H

#ifndef __ASSEMBLER__
#include <inc/types.h>
#include <inc/queue.h>
#include <inc/mmu.h>
//#include <inc/dynamic_allocator.h>

#endif /* not __ASSEMBLER__ */

/*
 * This file contains definitions for memory management in our OS,
 * which are relevant to both the kernel and user-mode software.
 */

/*2016*/
#define USE_KHEAP 1

// Global descriptor numbers
#define GD_KT     0x08     // kernel text
#define GD_KD     0x10     // kernel data
#define GD_UT     0x18     // user text
#define GD_UD     0x20     // user data
#define GD_TSS    0x28     // Task segment selector

/*
 * Virtual memory map:                                Permissions
 *                                                    kernel/user
 *    4 GB --------->  +------------------------------+
 * 					   |      Invalid Memory (*)	  |  PAGE_SIZE
 * KERNEL_HEAP_MAX ->  +------------------------------+
 *                     |     Kernel Heap (KHEAP)      | RW/--
 *                     :              .               :
 *                     :              .               :
 *KERNEL_HEAP_START -> +------------------------------+ 0xf6000000
 *                     :              .               :
 *                     :              .               :
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| RW/--
 *                     |                              | RW/--
 *                     |   Remapped Physical Memory   | RW/--
 *                     |                              | RW/--
 *    KERNEL_BASE -->  +------------------------------+ 0xf0000000
 *                     |  Cur. Page Table (Kern. RW)  | RW/--  PTSIZE
 * VPT,       -------> +------------------------------+ 0xefc00000      --+
 * KERNEL_STACK_TOP    |         Kernel Stack         | RW/--  KERNEL_STACK_SIZE   |
 *                     | - - - - - - - - - - - - - - -|                 PTSIZE
 *                     |      Invalid Memory (*)      | --/--             |
 * USER_LIMIT  ------> +------------------------------+ 0xef800000      --+
 *                     |  Cur. Page Table (User R-)   | R-/R-  	PTSIZE
 *    UVPT      ---->  +------------------------------+ 0xef400000
 *                     |   		  FREE Space	      | 		PTSIZE
 *					   +------------------------------+ 0xef000000
 *                     |           RO ENVS            | R-/R-  PTSIZE
 * USER_TOP,UENVS -->  +------------------------------+ 0xeec00000
 * UXSTACKTOP -/       |     User Exception Stack     | RW/RW  PAGE_SIZE
 *                     +------------------------------+ 0xeebff000
 *                     |       Empty Memory (*)       | --/--  PAGE_SIZE
 *    USTACKTOP  --->  +------------------------------+ 0xeebfe000
 *                     |      Normal User Stack       | RW/RW  PAGE_SIZE
 *                     +------------------------------+ 0xeebfd000
 *                     .                              .
 *                     .                              .
 *                     .                              .
 * USTACKBOTTOM, -->   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * USER_PAGES_WS_MAX   |      					 	  |
 *    				   | User Pages Working Set (Read)|
 * USER_HEAP_MAX,-->   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 0xA0000000 [Original Value 0xC0000000]
 * USER_PAGES_WS_START .                              .
 *                     .                              .
 *                     .    		User Heap         .
 * USER_HEAP_START-->  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	0x80000000
 *                     .                              .
 *                     .                              .
 *                     .                              .
 *                     |     Program Code & Data 	  |
 *    UTEXT -------->  +------------------------------+ 0x00800000
 *    PFTEMP ------->  |       Empty Memory (*)       |        PTSIZE
 *                     |                              |
 *    UTEMP -------->  +------------------------------+ 0x00400000      --+
 *                     |       Empty Memory (*)       |                   |
 *                     | - - - - - - - - - - - - - - -|                   |
 *                     |  User STAB Data (optional)   |                 PTSIZE
 *    USTABDATA ---->  +------------------------------+ 0x00200000        |
 *                     |       Empty Memory (*)       |                   |
 *    0 ------------>  +------------------------------+                 --+
 *
 * (*) Note: The kernel ensures that "Invalid Memory" (USER_LIMIT) is *never*
 *     mapped.  "Empty Memory" is normally unmapped, but user programs may
 *     map pages there if desired.  FOS user programs map pages temporarily
 *     at UTEMP.
 */


// All physical memory mapped at this address
#define	KERNEL_BASE	0xF0000000

// At PHYS_IO_MEM (640K) there is a 384K hole for I/O.  From the kernel,
// PHYS_IO_MEM can be addressed at KERNEL_BASE + PHYS_IO_MEM.  The hole ends
// at physical address PHYS_EXTENDED_MEM.
#define PHYS_IO_MEM	0x0A0000
#define PHYS_EXTENDED_MEM	0x100000

// Virtual page table.  Entry PDX[VPT] in the PD contains a pointer to
// the page directory itself, thereby turning the PD into a page table,
// which maps all the page_table_entries containing the page mappings for the entire
// virtual address space into that 4 Meg region starting at VPT.
#define VPT		(KERNEL_BASE - PTSIZE)
#define KERNEL_STACK_TOP	VPT
#define KERNEL_STACK_SIZE	(8*PAGE_SIZE)   		// size of a kernel stack
#define USER_LIMIT		(KERNEL_STACK_TOP - PTSIZE)

/*
 * User read-only mappings! Anything below here til USER_TOP are readonly to user.
 * They are global pages mapped in at env allocation time.
 */

// Same as VPT but read-only for users
#define UVPT		(USER_LIMIT - PTSIZE)

// Read-only copies of the Frame_Info structures
//2016: READ_ONLY_FRAMES_INFO is not FIT any more in the 4 MB space
//#define READ_ONLY_FRAMES_INFO		(UVPT - PTSIZE)

// Read-only copies of the global env structures
#define UENVS		(UVPT - 2 * PTSIZE)

/*
 * Top of user VM. User can manipulate VA from USER_TOP-1 and down!
 */

// Top of user-accessible VM
#define USER_TOP		UENVS
// Top of one-page user exception stack
#define UXSTACKTOP USER_TOP
// Next page left invalid to guard against exception stack overflow; then:
// Top of normal user stack
#define USTACKTOP	(USER_TOP - 2*PAGE_SIZE)

// Where user programs generally begin
#define UTEXT		(2*PTSIZE)

// Used for temporary page mappings.  Typed 'void*' for convenience
#define UTEMP		((void*) PTSIZE)
// Used for temporary page mappings for the user page-fault handler
// (should not conflict with other temporary page mappings)
#define PFTEMP		(UTEMP + PTSIZE - PAGE_SIZE)
// The location of the user-level STABS data structure
#define USTABDATA	(PTSIZE / 2)

//2016
#define KERNEL_HEAP_START 0xF6000000
#define KERNEL_HEAP_MAX 0xFFFFF000
//KHEAP pages number
#define NUM_OF_KHEAP_PAGES ((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE)

#define USER_HEAP_START 0x80000000
#define USER_HEAP_MAX 0xA0000000
#define NUM_OF_UHEAP_PAGES ((USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE)

#define USER_PAGES_WS_START USER_HEAP_MAX
#define USER_PAGES_WS_MAX (USER_PAGES_WS_START + sizeof(struct WorkingSetElement) * USER_TOP/PAGE_SIZE)

#define USTACKBOTTOM (ROUNDUP(USER_PAGES_WS_MAX, PAGE_SIZE))

//2017
#define KERNEL_SHARES_ARR_INIT_SIZE 0x2000
#define KERNEL_SEMAPHORES_ARR_INIT_SIZE 0x2000

//2022
#define USER_DYN_BLKS_ARRAY 0 //(ROUNDDOWN(USER_HEAP_START - (sizeof(struct MemBlock) * NUM_OF_UHEAP_PAGES), PAGE_SIZE) - PAGE_SIZE)

#ifndef __ASSEMBLER__

/*
 * The page directory entry corresponding to the virtual address range
 * [VPT, VPT + PTSIZE) points to the page directory itself.  Thus, the page
 * directory is treated as a page table as well as a page directory.
 *
 * One result of treating the page directory as a page table is that all page_table_entries
 * can be accessed through a "virtual page table" at virtual address VPT (to
 * which vpt is set in entry.S).  The page_table_entry for page number N is stored in
 * vpt[N].  (It's worth drawing a diagram of this!)
 *
 * A second consequence is that the contents of the current page directory
 * will always be available at virtual address (VPT + (VPT >> PGSHIFT)), to
 * which vpd is set in entry.S.
 */

extern volatile uint32 vpt[];     // VA of "virtual page table"
extern volatile uint32 vpd[];     // VA of current page directory

/*
 * Frame_Info descriptor structures.
 * Read/write to the kernel.
 *
 * Each Frame_Info describes one physical frame.
 * You can map a Frame_Info * to the corresponding physical address
 * with page2pa() in kern/pmap.h.
 */
LIST_HEAD(FrameInfo_List, FrameInfo);
typedef LIST_ENTRY(FrameInfo) Page_LIST_entry_t;

struct FrameInfo {
	Page_LIST_entry_t prev_next_info;	/* free list link */

	// references is the count of pointers (usually in page table entries)
	// to this page, for frames allocated using allocate_frame.
	// frames allocated at boot time using memory_manager.c's
	// boot_allocate_space do not have valid reference count fields.

	uint16 references;
	uint32 va;
	struct Env *environment;
	unsigned char isBuffered;
	struct WorkingSetElement *element;
};

#endif /* !__ASSEMBLER__ */
#endif /* !FOS_INC_MEMLAYOUT_H */
