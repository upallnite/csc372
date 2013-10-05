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
  TD * link;
  ThreadId tid;
  Registers regs;
  int priority;
  int waittime;
  RC returnCode;
  LL * inlist;
};

TD *CreateTD(ThreadId tid);
void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority);

#endif
