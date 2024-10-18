# FOS - Operating System 🖥️

![FOS](https://img.shields.io/badge/Language-C-blue.svg) ![Status](https://img.shields.io/badge/Status-In%20Development-yellow.svg)

Welcome to **FOS**, a minimalistic yet powerful operating system developed in C! This project is designed to explore low-level system programming, dynamic memory management, kernel, and user-space operations. FOS has been built from scratch and is divided into three major milestones, each focusing on key features of operating system development.

## 🌟 Features
- Dynamic memory allocation (First Fit, Best Fit strategies)
- Kernel heap management
- Page allocation and virtual memory mapping
- Efficient O(1) page removal strategy
- Custom system calls
- User heap management

## 🚀 Project Overview
FOS consists of three significant milestones, which gradually introduce system calls, dynamic memory management, kernel heap handling, and user memory allocation. Below is an overview of each milestone and the functionality implemented.

### Milestone 1: Dynamic Memory Management 🧠

In this milestone, we lay the foundation for memory handling in FOS by creating dynamic memory block management techniques.

**Key Functions:**

- `process_command`:
   - Handles and processes various system-level commands.

- **System Calls**:
   - A series of system calls are defined to facilitate interaction between the OS kernel and user programs.

- `Handle memory blocks dynamically`:
   - FOS uses **lists** to manage memory blocks dynamically in the heap. Both the OS and user programs can allocate and free memory as needed.

- `Allocate by First Fit`:
   - Implements **First Fit** strategy for memory allocation, where the first available memory block large enough is used.

- `free_block`:
   - Frees an allocated memory block, making it available for future allocations.

- `realloc_block_FF`:
   - Reallocates memory using the **First Fit** strategy, adjusting block size dynamically.

- `Allocate by Best Fit`:
   - Implements **Best Fit** memory allocation, finding the smallest block that fits the required space.

---

### Milestone 2: Kernel Heap and Page Allocator 🔧

The second milestone focuses on memory management in the **kernel heap** and user-space, introducing advanced memory handling techniques like page allocation and heap expansion.

**Key Functions:**

- `initialize_kheap_dynamic_allocator`:
   - Initializes the kernel heap with a given starting address, size, and limits.

- `sbrk(int increment)`:
   - Expands the heap by increasing or decreasing its size, facilitating dynamic allocation in the kernel.

- **FIRST FIT Strategy**:
   - Continues the implementation of the **First Fit** strategy for memory allocation in the kernel.

- `kmalloc(unsigned int size)`:
   - Allocates memory in the kernel heap with the **First Fit** strategy.

- `kfree(void* virtual_address)`:
   - Frees a memory block in the kernel heap.

- `kheap_physical_address(unsigned int virtual_address)`:
   - Translates a virtual address in the heap to its corresponding physical address.

- `kheap_virtual_address(unsigned int physical_address)`:
   - Translates a physical address back to its virtual address counterpart.

- `krealloc(void* virtual_address, uint32 new_size)`:
   - Reallocates memory dynamically in the kernel heap, adjusting its size as needed.

- `Fault Handler`:
   - Handles page faults efficiently, ensuring the system remains stable during memory accesses.

- **User Heap Management**:
   - The user-space memory is handled with dynamic allocators.

- `initialize_uheap_dynamic_allocator`:
   - Initializes the user heap for a given process with specific address boundaries.

- `sys_sbrk(int increment)`:
   - Expands the user heap size dynamically, mirroring the kernel heap's behavior.

- `allocate_user_mem`:
   - Allocates memory blocks in user-space, ensuring efficient memory utilization.

- `free(void* virtual_address)`:
   - Frees memory in user-space.

- `free_user_mem(struct Env* e, uint32 va, uint32 size)`:
   - Deallocates user memory at a given virtual address and size.

**BONUS**:
- `O(1) removal in free_user_mem`:
   - Implements an **O(1)** removal strategy for pages from the **Working Set (WS)** list, optimizing page removal by avoiding a full list search.

---

### Milestone 3: Page Replacement and Scheduling Algorithms 🔄

In the third milestone, we dive deeper into **page replacement algorithms** and implement a **BSD scheduler**. These features help optimize memory management and CPU scheduling for process execution.

**Key Functions:**

- **Fault handler by FIFO (First In, First Out)**:
   - Handles page faults using the **FIFO** algorithm.

- **Page Replacement Tests**:
   - `tst_page_replacement_alloc.c (tpr1)`:
     - Tests memory and page file allocation after page replacement.
   - `tst_page_replacement_stack.c (tpr2)`:
     - Tests page replacement of stack (new pages: creation, modification, and reading).
   - `tst_page_replacement_FIFO_1.c (tfifo1)`:
     - Tests FIFO page replacement, checking the working set before and after replacements.
   - `tst_page_replacement_FIFO_2.c (tfifo2)`:
     - Tests FIFO page replacement, ensuring proper maintenance of `page_last_WS_element` and the correct FIFO order after `free_user_mem`.

- **LRU (Least Recently Used) Algorithm**:
   - `tst_placement_1.c (tpplru1)`:
     - Tests page faults on stack (new pages) and page placement when the active list is not full.
   - `tst_placement_2.c (tpplru2)`:
     - Tests page faults on stack (new pages) and page placement when the active list is full, but the second active list is not.
   - `tst_placement_3.c (tpplru3)`:
     - Tests page faults on pages already in the second active list when it is not full.
   - `tst_page_replacement_LRU_Lists_1.c (tprlru1)`:
     - Tests memory and page file allocation after page replacement using the LRU algorithm.
   - `tst_page_replacement_LRU_Lists_2.c (tprlru2)`:
     - Tests the LRU algorithm, inspecting list content before and after replacements.
   - `tst_page_replacement_stack_LRU_Lists.c (tprlru3)`:
     - Tests LRU-based page replacement of stack (new pages: creation, modification, and reading).

- **BSD Scheduler**:
   - Implements a scheduling algorithm based on **BSD Unix** principles. The scheduler uses the following equations:
     - **Priority** (recalculated every 4 ticks):
       ```bash
       priority = PRI_MAX - (recentcpu / 4) - (nice * 2)
       ```
     - **CPU Usage** (updated every second):
       ```bash
       recentcpu = (2 * loadavg / (2 * loadavg + 1)) * recentcpu + nice
       ```
     - **Load Average** (updated every second):
       ```bash
       loadavg = (59/60) * loadavg + (1/60) * readyprocesses
       ```
   - `readyprocesses`: The number of processes that are either running or ready at the time of the update.

---


## 🔧 How to Build and Run

1. **Clone the repository**:
    ```bash
    git clone https://github.com/Yousef-116/OS_Project.git
    ```

2. **Navigate to the project directory**:
    ```bash
    cd FOS
    ```

3. **Build the project**:
    ```bash
    make all
    ```

4. **Run the operating system**:
    ```bash
    make run
    ```

## 🤝 Contributions

Contributions are welcome! Please fork the repository and submit a pull request to contribute to FOS.


---

## 🌍 Connect with Us

- Follow us on [GitHub](https://github.com/Yousef-116/) for the latest updates!
- Reach out via email: `yosuefelbeny@gmail.com`

