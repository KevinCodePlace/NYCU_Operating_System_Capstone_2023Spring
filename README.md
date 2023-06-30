# Operating System Capstone, Spring 2023

Welcome to the Operating System Capstone course for Spring 2023! This course focuses on the design and implementation of operating system kernels. Through a series of labs, you will learn both the theoretical concepts and practical implementation aspects.

The hardware platform used in this course is the Raspberry Pi 3 Model B+ (referred to as rpi3). Working with a real machine instead of an emulator allows you to gain hands-on experience.

## Labs
### Lab 0: Environment Setup, [Notes](https://hackmd.io/@OJo2ruXGShKdpuewtwzZcQ/S104l7ZS3)
In Lab 0, you will set up the development environment for future development. This includes installing the target toolchain and using it to build a bootable image for rpi3.

Goals of this lab:

- Set up the development environment.
- Understand the concept of cross-platform development.
- Test your rpi3.

### Lab 1: Hello World, [Notes](https://hackmd.io/@OJo2ruXGShKdpuewtwzZcQ/S104l7ZS3)
Lab 1 focuses on bare metal programming. You will implement a simple shell and establish communication between your host machine and rpi3 using the mini UART.

Goals of this lab:

- Practice bare metal programming.
- Learn how to access rpi3's peripherals.
- Set up the mini UART.
- Set up the mailbox.

### Lab 2: Booting, [Notes](https://hackmd.io/@OJo2ruXGShKdpuewtwzZcQ/Hy6j7lzrn)
Booting is the process of initializing the environment to run various user programs after a computer reset. Lab 2 introduces one method to load your kernel and user programs, as well as how to match devices to drivers on rpi3. Subsequent labs will cover the initialization of remaining subsystems.

Goals of this lab:

- Implement a bootloader that loads kernel images through UART.
- Implement a simple allocator.
- Understand the concept of initial ramdisk.
- Understand the concept of devicetree.

### Lab 3: Exception and Interrupt, [Notes](https://hackmd.io/@OJo2ruXGShKdpuewtwzZcQ/r1WP_BrX3)
In Lab 3, you will explore exceptions and interrupts. Exceptions are events that cause the currently executing program to relinquish the CPU to the corresponding handler. Understanding exception handling and interrupts is crucial for proper error handling and interaction with operating system services.

Goals of this lab:

- Understand the exception levels in Armv8-A.
- Understand exception handling.
- Understand interrupts.
- Learn how rpi3's peripherals interrupt the CPU using interrupt controllers.
- Learn how to multiplex a timer.
- Learn how to concurrently handle I/O devices.

### Lab 4: Allocator
Memory allocation is essential for maintaining the internal states of a kernel and supporting user programs. Lab 4 focuses on implementing memory allocators, which will be utilized in subsequent labs.

Goals of this lab:

- Implement a page frame allocator.
- Implement a dynamic memory allocator.
- Implement a startup allocator.

### Lab 5: Thread and User Process
Multitasking is a fundamental feature of an operating system. In Lab 5, you will learn how to create threads and switch between them to achieve multitasking. Additionally, you will explore how user programs become user processes and access kernel services through system calls.

Goals of this lab:

- Understand how to create threads and user processes.
- Implement a scheduler and context switch.
- Understand preemption.
- Implement POSIX signals.

### Lab 6: Virtual Memory
Virtual memory provides isolated address spaces, enabling each user process to run in its own space without interference from others. Lab 6 focuses on initializing the memory management unit (MMU) and setting up address spaces for the kernel and user processes to achieve process isolation.

Goals of this lab:

- Understand the ARMv8-A virtual memory system architecture.
- Understand how the kernel manages memory for user processes.
- Understand how demand paging works.
- Understand how copy-on-write works.

### Lab 7: Virtual File System
A file system manages data in storage mediums. Each file system has a specific way to store and retrieve the data. Hence, a virtual file system (VFS) is common in general-purpose OS, providing a unified interface for all file systems.

In Lab 7, you will implement a VFS interface for your kernel and a memory-based file system (tmpfs) that mounts as the root file system. Additionally, you will implement a special file for UART and framebuffer.

Goals of this lab:

- Understand how the VFS interface works.
- Understand how to set up a root file system.
- Understand how to operate on files.
- Understand how to mount a file system and look up a file across file systems.
- Understand how special files work.

### Lab 8: Non-volatile Storage
In the previous lab, files were stored in memory, which is volatile. In Lab 8, you will implement the FAT32 file system, which stores data on an SD card.

Goals of this lab:

- Understand how to interact with an SD card.
- Implement the FAT32 file system.
- Understand memory cache for slow storage mediums.
These labs will provide you with a comprehensive understanding of operating system kernels and their various components. Enjoy your learning journey in the Operating System Capstone course!



