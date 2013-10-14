#include "defines.h"
#include "list.h"
#include "kernel.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>

// Fixed size array of TDs
//TD TD_ARRAY[128];
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

void
InitKernel(void) {
  
  // Initialize actively running thread
  Active = CreateTD(1);
  InitTD(Active, 0, 0, 1);  //Will be set with proper return registers on context switch

  // Initialize kernel's sp, sr and pc of syscall handler.
#ifdef NATIVE
  InitTD(&Kernel, (uval32) SysCallHandler, (uval32) &(KernelStack.stack[STACKSIZE]), 0);
  Kernel.regs.sr = DEFAULT_KERNEL_SR;
#endif /* NATIVE */

  // Initialize lists
  ReadyQ = CreateList(L_PRIORITY);

  BlockedQ = CreateList(UNDEF);

  FreeQ = CreateList(L_LIFO);
}



void K_SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2) 
{ 
#ifdef NATIVE
  asm(".align 4; .global SysCallHandler; SysCallHandler:");
  uval32 sysMode = SYS_EXIT;
#endif

  RC returnCode ; 
  switch( type ) {
    case SYS_CREATE: 
    returnCode = CreateThread( arg0, arg1, arg2 ) ; 
    break ; 
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

RC CreateThread( uval32 pc, uval32 sp, uval32 priority ) 
{ 
  RC sysReturn = RC_SUCCESS;
  // Allocate a stack at the user level
  //Stack user_stack;
  // Create a new thread descriptor to be allocated
  //CreateTD()
  myprint("CreateThread ");
  return sysReturn;
} 

T_RC DestroyThread( ThreadId tid )
{
  TD* td_tid;

  // Remove the thread descriptor from whatever queue it is in.
  // DequeueTD(tid);

  // Add TD identified by tid to the list of free descriptors
  td_tid = CreateTD(tid);
  return OK;

}

void 
Idle() 
{ 
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
