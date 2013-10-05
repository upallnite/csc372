#include "defines.h"
#include "list.h"
#include "kernel.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>

TD *Active, Kernel;
Stack KernelStack;

void
InitKernel(void) {
  Active = CreateTD(1);
  InitTD(Active, 0, 0, 1);  //Will be set with proper return registers on context switch
#ifdef NATIVE
  InitTD(&Kernel, (uval32) SysCallHandler, (uval32) &(KernelStack.stack[STACKSIZE]), 0);
  Kernel.regs.sr = DEFAULT_KERNEL_SR;
#endif /* NATIVE */
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
  myprint("CreateThread ");
  return sysReturn;
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
