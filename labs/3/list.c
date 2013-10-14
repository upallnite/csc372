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

// allocates and properly initializes a list structure and returns a pointer to
// it or null.
LL *CreateList(ListType type) 
{
  LL *newList;

  if ((newList = malloc(sizeof(LL))) == NULL) {
    //printf("%s\n", "Error allocating space for a pointer to a list.");
    return NULL;
  }
  newList->type = type;
  newList->head = NULL;

    return newList;
}

//dequeues the TD at the head of list and returns a pointer to it, or else null.
TD * DequeueHead( LL *list )
{

  if(!list){
    return NULL;
  }

    TD *head;

    head = list->head;

  if (head) {
      list->head = (list->head)->link;
  }
    return head;
}

// destroys list, whose pointer is passed in as an argument. Returns 0 if 
// successful, and -1 otherwise.
RC DestroyList( LL *list ) 
{
  if(!list){
    return RC_FAILED;
  }
  if(list->head){
    TD* current = list->head;
    while(current){
      TD* old_head = DequeueHead(list);
      free(old_head);
      current = list->head;
    }
  }
  if (!list->head) {
    free(list);
    return RC_SUCCESS;
  } 

  free(list);
  return RC_FAILED;

}

//if list is a priority list, then enqueues td in its proper location. 
//Returns -1 if list is not a priority list and 0 otherwise.
RC PriorityEnqueue(TD *td, LL *list)
{
  TD *cur, *prev;

    if(!td || !list || list->type != L_PRIORITY){
      return RC_FAILED;
    }

    td->inlist = list;

    if(!list->head){
      list->head = td;
    } else{
      prev = NULL;
      cur = list->head;
      while(cur){
        if(cur->priority > td->priority){
          if(!prev){
            td->link = list->head;
            list->head = td;
          } else{
            td->link = cur;
            prev->link = td;
          }
          break;
        }
        prev = cur;
        cur = cur->link;
      }
    }
    return RC_SUCCESS;
}


//enqueues td at the head of list if list is a LIFO list. Returns 0 if 
//OK and -1 otherwise.
RC EnqueueAtHead(TD *td, LL *list)
{

  //TD *cur, *prev;

    if(!td || !list || list->type != L_LIFO){
      return RC_FAILED;
    }

    td->link = list->head;
    list->head = td;
    td->inlist = list;

    return RC_SUCCESS;

}

void waitDiff(TD *td, int diff) {
  TD *hld;
  hld = td;
  while (hld != NULL) {
    hld->waittime -= diff;
    hld = hld->link;
  }
}

// If list is a waiting list, then inserts td in its correct 
// position assuming it should wait for waittime. The waittime 
// values of the other elements in the list should be properly 
// adjusted. Return -1 if list is not a waiting list and 0 otherwise.
// waittime: total time a process should wait
// td->waittime: amount of time td should wait after the process right before it is done waiting.

// For example, let's say that we have the following list:

// -> {td1, wt:10} -> {td2, wt:10} -> {td3, wt:15}

// This means that td1 will be taken care of in 10 ms, td2 in 10+10=20 ms time, td3 in 10+10+15=35 ms time.

// Let's say I call WaitlistEnqueue(td4, 15, list). 

// So td4 should go after td1, and the waittime field of td4 is 5. 
// We then utdate the waittime field of td2 to 5 ms (since it now comes after td4) and the 
// waittime field of td3 is still 15 ms.
RC WaitlistEnqueue(TD *td, int waittime, LL *list)
{ 
    TD *cur, *prev;

    if(!td || !list){
      return RC_FAILED;
    }

    if(list->type != L_WAITING){
      return RC_FAILED;
    }

    if (list->head == NULL) {
    list->head = td;
  } else if (waittime < (list->head)->waittime){
    td->link = list->head;
    list->head = td;
  } else {      
    prev = list->head;
    cur = prev->link;
    waittime -= prev->waittime;

    while ((cur != NULL) && (waittime > cur->waittime))
    {
      prev = cur;
          cur = cur->link;
      waittime -= prev->waittime;
      }
      if (cur != NULL) { 
      td->link = prev->link;
      prev->link = td;      
    } else {
          prev->link = td;
    }
  }
  td->inlist = list;
  td->waittime = waittime;

  waitDiff(td->link, waittime);

  return RC_SUCCESS;
  
}

// The TD before the TD with process id tid in list, 
// which has at least two nodes.
TD * FindPrevTD(ThreadId tid, LL* list){
  TD* prev = NULL;
  TD* cur = list->head;
  
  while(cur){
    if(cur->tid == tid){
      return prev;
    }
    prev = cur;
    cur = cur->link;
  }
  return prev;
}


// Searches list for a TD with a process id tid, and 
// returns a pointer to it or null otherwise.
TD * FindTD(ThreadId tid, LL* list){
  
  if(!list || !list->head){
    return NULL;
  }

  TD *prev;

  if (list->head->tid == tid) {
    return list->head;
  } else {
    prev = FindPrevTD(tid, list);
    return prev->link;
  }

  return NULL;

}

// Dequeues td from whatever list it might be in, 
// if it is in one.
void DequeueTD(TD* td){

  if(!td){
    return;
  }

  if (td->inlist) { // inlist is not empty 
    if(td->inlist->head == td){ // inlist only has one element
      td->inlist->head = td->link;
    } else { // inlist has at least two elements
      TD *prev, *next; 
    
      prev = FindPrevTD(td->tid, td->inlist);
      next = prev->link->link;
      prev->link = next;
    }
  }
}
