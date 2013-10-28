#include "defines.h"
#include "list.h"
#include "user.h"
#include "main.h"

#ifndef NATIVE

#include "kernel.h" 

#endif /* NATIVE */

#include <stdlib.h>
#include <assert.h>

RC SysCall(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2) 
{
  uval32 returnCode;

#ifdef NATIVE  
  uval32 sysMode = SYS_ENTER;  

  // Save context on stack. This is already done in the isr.

  // Load arguments, execute software trap to kernel.
  asm volatile("ldw r8, %0\n\t"
	       "ldw r4, %1\n\t" 
	       "ldw r5, %2\n\t"
	       "ldw r6, %3\n\t"
	       "ldw r7, %4\n\t" 
	       "trap"
	       : : "m" (sysMode), "m" (type), "m" (arg0), "m" (arg1), "m" (arg2)
	       : "r4", "r5", "r6", "r7", "r8");  
#else /* NATIVE */
  myprint("Here\n");
  CreateThread(arg0, arg1, arg2); //Kernel system call - not normally accessible from user space
#endif /* NATIVE */
  
  returnCode = RC_SUCCESS; //Change this code to take the actual return value
  return returnCode; 
} 

void mymain() 
{ 
  RC ret;

  ret = SysCall(SYS_CREATE, 0x1234, 0, 0); 
  assert(ret == RC_SUCCESS);
  
  myprint("DONE\n");

  while(1);
}
