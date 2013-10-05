#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "defines.h"
#include "list.h"

typedef struct type_STACK Stack;

struct type_STACK 
{ 
  uval8 stack[STACKSIZE]; 
};

extern TD *Active, Kernel;

ThreadId CreateThread( uval32 pc, uval32 stackSize, uval32 priority );
void Idle(void);
void InitKernel(void);  

void K_SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2);
extern void SysCallHandler(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2);
#endif
