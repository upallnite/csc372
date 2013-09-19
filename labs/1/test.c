#include <stdlib.h>
#include <stdio.h>

typedef int ProcessId;

struct PD
{
    struct PD *link;
    ProcessId pid;
    int priority;
    int waittime;
    struct LL *inlist;
};

typedef enum {UNDEF, L_PRIORITY, L_LIFO, L_WAITING} ListType;
typedef enum {SUCC = 0, FAIL = -1} RC;

struct LL
{
    struct PD *head;
    ListType type;
};

// allocates and properly initializes a list structure and returns a pointer to
// it or null.
struct LL *CreateList(ListType type) 
{
	struct LL *newList;

	if ((newList = malloc(sizeof(struct LL))) == NULL) {
		return NULL;
	}
	newList->type = type;
	newList->head = NULL;

    return newList;
}

//dequeues the PD at the head of list and returns a pointer to it, or else null.
struct PD * DequeueHead( struct LL *list )
{
    struct PD *head;

    head = list->head;

	if (head != NULL) {
    	list->head = (list->head)->link;
	}
    return head;
}

// destroys list, whose pointer is passed in as an argument. Returns 0 if 
// successful, and -1 otherwise.
RC DestroyList( struct LL *list ) 
{
	if(list->head){
		struct PD* current = list->head;
		while(current != NULL){
			struct PD* old_head = DequeueHead(list);
			free(old_head);
			current = list->head;
		}
	}
	if (list->head == NULL) {
		return SUCC;
	} 
	return FAIL;
/*
	free(list);
	list = NULL;
	if (list == NULL) {
		return SUCC;
	}
	return FAIL;
	*/
}

//if list is a priority list, then enqueues pd in its proper location. 
//Returns -1 if list is not a priority list and 0 otherwise.
RC PriorityEnqueue(struct PD *pd, struct LL *list)
{
    struct PD *cur, *prev;
	
	if (list->type == L_PRIORITY) {    
		if (list->head == NULL) {
			list->head = pd;
		} else if (pd->priority < (list->head)->priority){
			pd->link = list->head;
			list->head = pd;
		} else {
			prev = list->head;
			cur = prev->link;

			while ((cur != NULL) && (pd->priority > cur->priority))
			{
				printf("hi");
				prev = cur;
  		  		cur = cur->link;
    		}
    		if (cur != NULL) { 
				pd->link = prev->link;
				prev->link = pd;			
			} else {
        		prev->link = pd;
			}
		}
		pd->inlist = list;
		return SUCC;
	} else {
		return FAIL;
	}
}

//enqueues pd at the head of list if list is a LIFO list. Returns 0 if 
//OK and -1 otherwise.
RC EnqueueAtHead(struct PD *pd, struct LL *list)
{
    if (list->type == L_LIFO) {
        pd->link = list->head;
        list->head = pd;
		pd->inlist = list;
        return SUCC;
    } else {
        return FAIL;
    }
}

void waitDiff(struct PD *pd, int diff) {
	struct PD *hld;
	hld = pd;
	while (hld != NULL) {
		hld->waittime -= diff;
		hld = hld->link;
	}
}

RC WaitlistEnqueue( struct PD *pd, int waittime, struct LL *list)
{	
    struct PD *cur, *prev;

	if (list->type == L_WAITING) {    
		if (list->head == NULL) {
			list->head = pd;
		} else if (waittime < (list->head)->waittime){
			pd->link = list->head;
			list->head = pd;
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
				pd->link = prev->link;
				prev->link = pd;			
			} else {
        		prev->link = pd;
			}
		}
		pd->inlist = list;
		pd->waittime = waittime;

		waitDiff(pd->link, waittime);

		return SUCC;
	} else {
		return FAIL;
	}
}

struct PD * FindPrevPD(ProcessId pid, struct LL* list){
	struct PD* prev = NULL;
	struct PD* cur = list->head;
	
	while(cur != NULL){
		//printf("pid: %d\n", cur->pid);
		if(cur->pid == pid){
			return prev;
		}
		prev = cur;
		cur = cur->link;
		//printf("p pid: %d\n", prev->pid);
	}
	return prev;
}

struct PD * FindPD(ProcessId pid, struct LL *list){
	struct PD *prev;

	if (list->head != NULL) { 
		if (list->head->pid == pid) {
			return list->head;
		} else {
			prev = FindPrevPD(pid, list);
			return prev->link;
		}
	}

/*	struct PD* current = list->head;
	while(current != NULL){

		if(current->pid == pid){
			return current;
		}

		current = current->link;
	}
*/	return NULL;
}

void DequeuePD(struct PD* pd){
/*
	// Case 0: pd->inlist refers to an empty list.
	if(!pd->inlist){;}
	
	//Case 1: pd->inlist refers to a singleton list.
	if(!pd->inlist->head->link){
		pd->inlist->head = NULL;
	}
					
	// Case 2: pd->head refers to a list with two or more elements.
	struct PD* prev = FindPrevPd(pd->pid, pd->inlist);
	struct PD* next = prev->link->link;
	free(prev->link);
	prev->link = next;	
*/
	if (pd->inlist != NULL) { // Case 0 
		if(pd->inlist->head == pd){ // Case 1
			pd->inlist->head = pd->link;
		} else { // Case 2
			struct PD *prev, *next; 
		
			prev = FindPrevPD(pd->pid, pd->inlist);
			next = prev->link->link;
			prev->link = next;
		}
	}
}	


typedef enum {False = 0, True = 1} bool;

int main(void) {

	struct PD *pd1 = &(struct PD) {.pid = 1, .priority = 4};
	struct PD *pd2 = &(struct PD) {.pid = 2, .priority = 5};
	struct PD *pd3 = &(struct PD) {.pid = 3, .priority = 3};
	struct PD *pd4 = &(struct PD) {.pid = 4, .priority = 1};
	struct PD *pd5 = &(struct PD) {.pid = 5, .priority = 2};
	
	struct PD *hld1, *hld2, *hld3, *hld4, *hld5;
	struct LL *list;
	bool t1, t2, t3, t4, t5;

	ListType type=L_PRIORITY;
	list = CreateList(type);
	
	t1 = PriorityEnqueue(pd1,list);
	t2 = PriorityEnqueue(pd2,list);
	t3 = PriorityEnqueue(pd3,list);
	t4 = PriorityEnqueue(pd4,list);
	t5 = PriorityEnqueue(pd5,list);

	hld1 = FindPrevPD(1, list);
	hld2 = FindPrevPD(2, list);
	hld3 = FindPrevPD(3, list);
	hld4 = FindPrevPD(4, list);
	hld5 = FindPrevPD(5, list);
	
	printf("pid: %d priority: %d\n", hld1->pid, hld1->priority);
	printf("pid: %d priority: %d\n", hld2->pid, hld2->priority);
	printf("pid: %d priority: %d\n", hld3->pid, hld3->priority);
	//printf("pid: %d priority: %d\n", hld4->pid, hld4->priority); 
	//causes SEGFAULT because FindPrevPD returns NULL
	printf("pid: %d priority: %d\n", hld5->pid, hld5->priority);
		
	DequeuePD(pd1);
	DequeuePD(pd2);
	DequeuePD(pd3);
	DequeuePD(pd4);
	DequeuePD(pd5);

	//dl = DestroyList(list);
	
	printf("t1: %d\nt2: %d\nt3: %d\nt4: %d\nt5: %d\n", t1, t2, t3, t4, t5);

    /*
	list = CreateList(L_WAITING);
	t1 = WaitlistEnqueue( pd, 10, list);
	pd2 = FindPD( 1, list ) ;
	DequeuePD( pd2) ;
	t3=DestroyList(list) ;
	
	printf("t1: %d\nt3: %d\n", t1, t3);*/
	
	return 0;
}

