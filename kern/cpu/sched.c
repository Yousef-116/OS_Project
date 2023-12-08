#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>


uint32 isSchedMethodRR(){if(scheduler_method == SCH_RR) return 1; return 0;}
uint32 isSchedMethodMLFQ(){if(scheduler_method == SCH_MLFQ) return 1; return 0;}
uint32 isSchedMethodBSD(){if(scheduler_method == SCH_BSD) return 1; return 0;}

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
	scheduler_status = SCH_STOPPED;
}

//=========================
// [2] Main FOS Scheduler:
//=========================
void fos_scheduler(void)
{
	//	cprintf("inside scheduler\n");

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR)
	{
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL)
		{
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	}
	else if (scheduler_method == SCH_MLFQ)
	{
		next_env = fos_scheduler_MLFQ();
	}
	else if (scheduler_method == SCH_BSD)
	{
		next_env = fos_scheduler_BSD();
	}
	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env ;
	chk2(next_env) ;
	curenv = old_curenv;

	//sched_print_all();

	if(next_env != NULL)
	{
		//		cprintf("\nScheduler select program '%s' [%d]... counter = %d\n", next_env->prog_name, next_env->env_id, kclock_read_cnt0());
		//		cprintf("Q0 = %d, Q1 = %d, Q2 = %d, Q3 = %d\n", queue_size(&(env_ready_queues[0])), queue_size(&(env_ready_queues[1])), queue_size(&(env_ready_queues[2])), queue_size(&(env_ready_queues[3])));
		env_run(next_env);
	}
	else
	{
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

//=============================
// [3] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum)
{

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel)
{
#if USE_KHEAP
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
#endif
}

//===============================
// [5] Initialize BSD Scheduler:
//===============================
void sched_init_BSD(uint8 numOfLevels, uint8 quantum)
{
#if USE_KHEAP
	//TODO: [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");

	cprintf("here==============================\n");
	load_avg = fix_int(0);
//	kclock_set_quantum(quantum);
	*quantums = quantum;
	env_ready_queues = kmalloc(sizeof(struct Env_Queue) * numOfLevels);
	num_of_ready_queues = numOfLevels;

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_BSD;
	//=========================================
	//=========================================
#endif
}


//=========================
// [6] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ()
{
	panic("not implemented");
	return NULL;
}

//=========================
// [7] BSD Scheduler:
//=========================
struct Env* fos_scheduler_BSD()
{
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");

//	cprintf("num_of_ready_queues = %d\n", num_of_ready_queues);
	for (int i = num_of_ready_queues-1; i >= 0; --i)
	{
		if (!LIST_EMPTY(&env_ready_queues[i]))
		{
			return LIST_FIRST(&env_ready_queues[i]);
		}
	}

	return NULL;
}

//========================================
// [8] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
int Seconds = -1;
void clock_interrupt_handler()
{
	cprintf("\n\n>> clock_interrupt_handler()... Seconds = %d... ", Seconds);
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here
	{
		int seconds = ROUNDDOWN(timer_ticks()*(*quantums), 1000)/1000;
		cprintf("quantum = %d,  timer_ticks() = %d,  seconds = %d\n", *quantums, timer_ticks(), seconds);
		int num_of_ready_processes = 0;
		int num_of_not_ready_processes = 0;
		for(int i = 0; i < num_of_ready_queues ; i++)
		{
//			num_of_ready_processes += queue_size(&env_ready_queues[i]);

			struct Env *all_env;
			LIST_FOREACH(all_env, &env_ready_queues[i])
			{
				if(all_env->env_status == ENV_READY)
					num_of_ready_processes++;
				else
					num_of_not_ready_processes++;
			}
		}
		cprintf("num_of_ready_processes = %d... ", num_of_ready_processes);
		cprintf("num_of_not_ready_processes = %d\n", num_of_not_ready_processes);
		//𝑙𝑜𝑎𝑑_𝑎𝑣𝑔  = (59/60) × 𝑙𝑜𝑎𝑑_𝑎𝑣𝑔  + (1/60) × 𝑟𝑒𝑎𝑑𝑦_𝑝𝑟𝑜𝑐𝑒𝑠𝑠𝑒𝑠.
		if(seconds != Seconds)
		{
			Seconds = seconds;
			cprintf(">> one second passed\n");
			//recalculated once per second
			{
				fixed_point_t L1 = fix_int(59);
				L1 = fix_unscale(L1, 60);
				cprintf("59/60=%d\n", L1);
				L1 = fix_mul(L1, load_avg);

				fixed_point_t L2 = fix_int(1);
				L2 = fix_unscale(L2, 60);
				cprintf("1/60=%d\n", L2);
				L2 = fix_scale(L2, num_of_ready_processes);

				cprintf(">> load_avg recalculated... old=%d, ", load_avg);
				load_avg = fix_add(L1, L2);
				cprintf("new=%d\n", load_avg);
			}

			//==================================================================

			//update recent cpu time once per second for every process
			{
				cprintf(">> update recent cpu time once per second for every process\n");
				for(int i = 0; i < num_of_ready_queues ; i++)
				{
					struct Env *all_env;
					LIST_FOREACH(all_env, &env_ready_queues[i])
					{
						if(all_env != NULL)
						{
							fixed_point_t R1 = fix_scale(load_avg, 2);
							fixed_point_t R2 = fix_add(R1, fix_int(1));
							R1 = fix_div(R1, R2);
							R2 = fix_mul(R1, all_env->recent_cpu_time);
							all_env->recent_cpu_time = fix_add(R2, fix_int(all_env->nice_value));
						}
					}
				}
			}
		}

		//𝒓𝒆𝒄𝒆𝒏𝒕_𝒄𝒑𝒖: updated on each timer tick for running process
		//𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢  = (2 × 𝑙𝑜𝑎𝑑_𝑎𝑣𝑔)/(2 × 𝑙𝑜𝑎𝑑_𝑎𝑣𝑔  + 1) × 𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢  + nice.
		cprintf(">> running process id = %d\n", curenv->env_id);
		fixed_point_t p1 = fix_scale(load_avg, 2);
		fixed_point_t p2 = fix_add(p1, fix_int(1));
		p1 = fix_div(p1, p2);
		p2 = fix_mul(p1, curenv->recent_cpu_time);
		curenv->recent_cpu_time = fix_add(p2, fix_int(curenv->nice_value));


		//Priority: recalculated every 4th tick.
		if(timer_ticks()%4 == 0)
		{
			for(int i = 0; i < num_of_ready_queues ; i++)
			{
				struct Env *all_env;
				LIST_FOREACH(all_env, &env_ready_queues[i])
				{
					if(all_env != NULL)
					{
						//priority = PRI_MAX - (𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢  / 4) - (nice × 2).
						fixed_point_t p1 = fix_int(PRI_MAX);                          //PRI_MAX
						fixed_point_t p2 = fix_unscale(all_env->recent_cpu_time ,4);  //(𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢  / 4)
						fixed_point_t PRIORITY = fix_sub(p1, p2);

						fixed_point_t p3 = fix_int(all_env->nice_value * 2);          //(nice × 2)
						PRIORITY = fix_sub(PRIORITY, p3);

						int trunc_priority = fix_trunc(PRIORITY);

						if(trunc_priority > PRI_MAX) trunc_priority = PRI_MAX;
						else if(trunc_priority < PRI_MIN) trunc_priority = PRI_MIN;

						all_env->priority = trunc_priority;
					}
				}
			}
		}
	}


	/********DON'T CHANGE THIS LINE***********/
	ticks++ ;
	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
	{
		update_WS_time_stamps();
	}
	//cprintf("Clock Handler\n") ;
	fos_scheduler();
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		struct WorkingSetElement* wse ;
		{
			int i ;
#if USE_KHEAP
			LIST_FOREACH(wse, &(curr_env_ptr->page_WS_list))
			{
#else
			for (i = 0 ; i < (curr_env_ptr->page_WS_max_size); i++)
			{
				wse = &(curr_env_ptr->ptr_pageWorkingSet[i]);
				if( wse->empty == 1)
					continue;
#endif
				//update the time if the page was referenced
				uint32 page_va = wse->virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr->env_page_directory, page_va) ;
				uint32 oldTimeStamp = wse->time_stamp;

				if (perm & PERM_USED)
				{
					wse->time_stamp = (oldTimeStamp>>2) | 0x80000000;
					pt_set_page_permissions(curr_env_ptr->env_page_directory, page_va, 0 , PERM_USED) ;
				}
				else
				{
					wse->time_stamp = (oldTimeStamp>>2);
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < __TWS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->__ptr_tws[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr->env_page_directory, table_va))
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr->env_page_directory, table_va);
					}
					else
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}

