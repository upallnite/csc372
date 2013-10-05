#include "defines.h"
#include "list.h"
#include "main.h"

#include <stdlib.h>

TD *CreateTD(ThreadId tid)
{
  TD *thread = (TD *)malloc(sizeof(TD));

  if(thread != NULL) {
    thread->link = NULL;
    thread->tid = tid;
    thread->priority = 0;
    thread->waittime = 0;
    thread->inlist = NULL;
    thread->returnCode = 0;

    thread->regs.pc = 0;
    thread->regs.sp = 0;
    thread->regs.sr = 0;
  } else {
    myprint("Failed to allocate new thread\n");
  }

  return thread;
}

void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority) 
{ 
  if(td != NULL) {
    td->regs.pc  = pc; 
    td->regs.sp = sp; 
    td->regs.sr  = DEFAULT_THREAD_SR; 
    td->priority = priority; 
  } else {
    myprint("Tried to initialize NULL pointer\n");
  }
} 
