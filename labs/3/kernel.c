#include "defines.h"
#include "list.h"
#include "kernel.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>

// Fixed size array of TDs
TD TD_ARRAY[MAX_THREADS];
// 

// Contains the actively running thread
TD *Active;
// Contains the kernel's stack pointer, default 
// status register, and program counter of system 
// call handler. Used to enter/exit to/from system calls.
TD Kernel;

Stack KernelStack;

// Contains the TD's of all threads that are ready to run, 
// ordered by priority. When a thread is entered into the list, 
// then it is to be positioned after all TD's with higher or 
// equal priority. 
LL* ReadyQ;

// Contains the TDs of all threads currently blocked. See Suspend()
LL* BlockedQ;

// Contains all TDs that are currently unallocated. You have an array of 
// thread descriptors, and not all of them  will always be used. Any descriptor 
// that is not used should be placed into this queue, so that they are easily 
// accessible when a new descriptor is needed.
LL* FreeQ;

void InitKernel(void) {

	// Initialize actively running thread
	Active = CreateTD(1);
	InitTD(Active, 0, 0, 1); //Will be set with proper return registers on context switch

	// Initialize kernel's sp, sr and pc of syscall handler.
#ifdef NATIVE
	InitTD(&Kernel, (uval32) SysCallHandler, (uval32) &(KernelStack.stack[STACKSIZE]), 0);
	Kernel.regs.sr = DEFAULT_KERNEL_SR;
#endif /* NATIVE */

	// Initialize lists
	ReadyQ = CreateList(L_PRIORITY);

	BlockedQ = CreateList(UNDEF);

	FreeQ = CreateList(L_PRIORITY);
	int *cnt = 128;
	while (*cnt > 0) {
		TID * tid;
		tid->id = cnt;
		tid->link = FreeQ->head;
		FreeQ->head = tid;
	}
}

void K_SysCall(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2) {
#ifdef NATIVE
	asm(".align 4; .global SysCallHandler; SysCallHandler:");
	uval32 sysMode = SYS_EXIT;
#endif

	RC returnCode;
	T_RC err;

	switch (type) {
	case SYS_CREATE:
		returnCode = CreateThread(arg0, arg1, arg2);
		break;
	case SYS_DIST:
		err = DestroyThread(arg0);
		break;
	case SYS_YIELD:
		err = Yield();
		break;
	case SYS_SUSP:
		err = Suspend();
		break;
	case SYS_RESUME:
		err = ResumeThread(arg0);
		break;
	case SYS_CHANGE_PRI:
		err = ChangeThreadPrioirty(arg0, arg1);
		break;
	default:
		myprint("Invalid SysCall type\n");
		returnCode = RC_FAILED;
		break;
	}
#ifdef NATIVE
	asm volatile("ldw r8, %0" : : "m" (sysMode): "r8");
	asm( "trap" );
#endif /* NATIVE */
}

uval32 getTid(){
	TID tid;

	if ((tid = DequeueHeadFreeQ(FreeQ)) == NULL) {
		return 0;
	} else {
		return tid.id;
	}
}

/* 	Creates a new thread that should start executing the procedure pointed to by
 *	pc. Creating a new thread should be done by first allocating a stack at the
 *	user level (using malloc() with a minimum size of 8K). At the kernel level,
 *	this should cause a new thread descriptor to be allocated, its fields to be
 *	initialized and the descriptor to be enqueued in the ReadyQ. If the new
 *	thread has higher priority than the invoking thread then the invoking thread
 *	should yield the processor to the new thread.
 *
 *	Return Value - CreateThread() should return the thread Id of the new thread,
 *	RESOURCE_ERROR of there are no thread descriptors available, STACK_ERRROR
 *	if stackSize is not at least 8K large, and PRIORITY_ERROR if priority is not
 *	in the range of valid priorities.
 */

RC CreateThread(uval32 pc, uval32 sp, uval32 priority) {
	int *ptr, tid;
	TD thread;
	RC sysReturn = RC_SUCCESS;

	if ((priority < 1) & (priority > 128)) {
		return PRIORITY_ERROR;
	} else if ((tid == getTid())< 1) {
		return RESOURCE_ERROR;
	} else if ((ptr = malloc(8000)) == 0) {
		return STACK_ERROR;
	}
	//Stack user_stack;
	// Create a new thread descriptor to be allocated
	thread = CreateTD(tid);

    thread->priority = priority;
    thread->inlist = ReadyQ;
    thread->regs.pc = pc;
    thread->regs.sp = sp;

	myprint("CreateThread ");

	if (priority > currentThreadPriority) {
    	yield();
    }

	return sysReturn;
}

/*	ResumeTread:
 *  Wakes up the thread identified by the tid and makes it ready to run. If
 *  that thread has higher priority than the invoking thread then the invoking
 *  thread should yield the processor.
 *
 *  Return Value - ResumeThread() should return TID_ERROR if there is no thread
 *  with Id tid, NOT_BLOCKED if the target thread is not blocked, and OK
 *  otherwise.
 */

T_RC ResumeThread(ThreadId tid) {
	TD td;
	T_RC err = 0;

	if (!tidExists(tid)) {
		return TID_ERROR;
	} else {
		td = getTD(tid);
	}

	if (td.inlist != BlockedQ) {
		return NOT_BLOCKED;
	} else {
		td.inlist = ReadyQ;
	}

	if (td.priority > currentThreadPriority) {
    	yield();
    }

	return OK;
}

/* ChangePriorityThread:
 * Changes the priority of the target thread identified by tid to newPriority.
 * This can be achieved by setting the priority field of the thread descriptor
 * to newPriority. If the corresponding TD is in the ReadyQ, then remove and
 * re-insert it, so that the ReadyQ remains sorted according to Priority. If
 * the target thread now has higher priority than the invoking thread then the
 * invoking thread should yield the processor to the target thread.
 *
 * Return Value - ChangeThreadPriority() should return TID_ERROR if there is no
 * thread with Id tid, PRIORITY_ERROR if newPriority is not valid, and OK
 * otherwise.
 */

T_RC ChangeThreadPriority(ThreadId tid, int newPriority) {
	TD * td;

	if (!tidExists(tid)) {
		return TID_ERROR;
	} else {
		td = getTD(tid);
	}

	if ((newPriority < 1) & (newPriority > 128)) {
		return PRIORITY_ERROR;
	}

	td.priority = newPriority;

	if (td.inlist == ReadyQ){
		if (dequeue(td, ReadyQ)) {
			priorityEnqueue(td, ReadyQ);
		} else {
			myprint("Dequeue error\n");
		}
	}

	return OK;
}
// Destroy the thread identified by tid.
T_RC DestroyThread(ThreadId tid) {
	TD* td_tid;

	// If tid is 0 or is the same as that of the invoking thread,
	// then the invoking thread should be destroyed.
	// If the Active thread is killed, a new thread should be
	// dispatched.
	if (tid == 0 || tid == Active->tid) {
		// Kill the Active Thread
		Active = NULL;
		// Dispatch the head of the ReadyQ.
		// We know that there is at least one thread here
		// (the idle thread).
		// But what happens if the idle thread is destroyed?
		// Can this happen? Should we replace it if so?
		Active = DequeueHead(ReadyQ);
	}

	// Remove the thread descriptor from whatever queue it is in.

	// First find the TD associated with tid by searching in the
	// three lists.
	if ((td_tid = FindTD(tid, ReadyQ)) == NULL) {
		if ((td_tid = FindTD(tid, BlockedQ)) == NULL) {
			return TID_ERROR;
		}
	}

	// Then dequeue the TD from the list it is in.
	DequeueTD(td_tid);

	// Add TD identified by tid to the list of free descriptors
	td_tid = CreateTD(tid);
	EnqueueAtHead(td_tid, FreeQ);

	return OK;

}

// Allows the invoking thread to yield the processor to the highest 
// priority ready-to-run thread with equal or higher priority.
T_RC Yield() {

	// Enqueue the Active thread onto the ReadyQ behind all threads of
	// the same, or higher, priority.
	PriorityEnqueue(Active, ReadyQ);
	// Dispatch the ready-to-run thread with the highest priority
	Active = DequeueHead(ReadyQ);

	return OK;
}

// Block the invoking thread until it is woken up again.
T_RC Suspend() {

	// Enqueue the Active thread onto BlockedQ
	EnqueueAtHead(Active, BlockedQ);
	// Dispatch the ready-to-run thread with the highest priority
	Active = DequeueHead(ReadyQ);

	return OK;
}

void Idle() {
	/*
	 int i;
	 while( 1 )
	 {
	 myprint( "CPU is idle\n" );
	 for( i = 0; i < MAX_THREADS; i++ )
	 {
	 }
	 Yield();
	 }
	 */
}


/*TO DO:
 * create:
 * 	getTD(tid)
 *	tidExists()
 *
 */
