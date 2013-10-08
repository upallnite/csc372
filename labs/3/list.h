#ifndef _LIST_H_
#define _LIST_H_

#include "defines.h"

typedef enum { UNDEF, L_PRIORITY, L_LIFO, L_WAITING} ListType ;

#define MIN_PRIORITY 100

typedef struct type_LL LL;
typedef struct type_TD TD;
typedef struct type_REGS Registers;

struct type_REGS
{
  uval32 sp;
  uval32 pc;
  uval32 sr;
}; 

struct type_LL
{
  TD *head;
  ListType type;
};

struct type_TD
{
  // Points to the next TD in whatever queue the TD is in
  TD * link;
  // The unique number that can be used to identify a thread once it has 
  // been created.
  ThreadId tid;
  // Structure used for savinfg CPU registers and other CPU state when the 
  // state of the thread needs to be saved.
  Registers regs;
  // Holds the current priority of the thread by convention in systems software 
  // (particularly for UNIX), the higher the number in priority the lower the 
  // importance of the thread.
  int priority;
  // ?
  int waittime;
  // Used to temporarily hold the return value of a system call
  RC returnCode;
  // Identifies the queue that the thread is currently in.
  LL * inlist;
};

TD *CreateTD(ThreadId tid);
void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority);

#endif
