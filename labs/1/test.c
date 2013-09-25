#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
		printf("%s\n", "Error allocating space for a pointer to a list.");
		return NULL;
	}
	newList->type = type;
	newList->head = NULL;

    return newList;
}

//dequeues the PD at the head of list and returns a pointer to it, or else null.
struct PD * DequeueHead( struct LL *list )
{

	if(!list){
		return NULL;
	}

    struct PD *head;

    head = list->head;

	if (head) {
    	list->head = (list->head)->link;
	}
    return head;
}

// destroys list, whose pointer is passed in as an argument. Returns 0 if 
// successful, and -1 otherwise.
RC DestroyList( struct LL *list ) 
{
	if(!list){
		return FAIL;
	}
	if(list->head){
		struct PD* current = list->head;
		while(current){
			struct PD* old_head = DequeueHead(list);
			free(old_head);
			current = list->head;
		}
	}
	if (!list->head) {
		free(list);
		return SUCC;
	} 

	free(list);
	return FAIL;

}

//if list is a priority list, then enqueues pd in its proper location. 
//Returns -1 if list is not a priority list and 0 otherwise.
RC PriorityEnqueue(struct PD *pd, struct LL *list)
{
	struct PD *cur, *prev;

    if(!pd || !list || list->type != L_PRIORITY){
    	return FAIL;
    }

    pd->inlist = list;

    if(!list->head){
    	list->head = pd;
    } else{
    	prev = NULL;
    	cur = list->head;
    	while(cur){
    		if(cur->priority > pd->priority){
    			if(!prev){
    				pd->link = list->head;
    				list->head = pd;
    			} else{
    				pd->link = cur;
    				prev->link = pd;
    			}
    			break;
    		}
    		prev = cur;
    		cur = cur->link;
    	}
    }
    return SUCC;
}


//enqueues pd at the head of list if list is a LIFO list. Returns 0 if 
//OK and -1 otherwise.
RC EnqueueAtHead(struct PD *pd, struct LL *list)
{

	struct PD *cur, *prev;

    if(!pd || !list || list->type != L_LIFO){
    	return FAIL;
    }

    pd->link = list->head;
    list->head = pd;
    pd->inlist = list;

    return SUCC;

}

void waitDiff(struct PD *pd, int diff) {
	struct PD *hld;
	hld = pd;
	while (hld != NULL) {
		hld->waittime -= diff;
		hld = hld->link;
	}
}

// If list is a waiting list, then inserts pd in its correct 
// position assuming it should wait for waittime. The waittime 
// values of the other elements in the list should be properly 
// adjusted. Return -1 if list is not a waiting list and 0 otherwise.
RC WaitlistEnqueue(struct PD *pd, int waittime, struct LL *list)
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

// The PD before the PD with process id pid in list, 
// which has at least two nodes.
struct PD * FindPrevPD(ProcessId pid, struct LL* list){
	struct PD* prev = NULL;
	struct PD* cur = list->head;
	
	while(cur){
		if(cur->pid == pid){
			return prev;
		}
		prev = cur;
		cur = cur->link;
	}
	return prev;
}


// Searches list for a PD with a process id pid, and 
// returns a pointer to it or null otherwise.
struct PD * FindPD(ProcessId pid, struct LL *list){
	
	if(!list || !list->head){
		return NULL;
	}

	struct PD *prev;

	if (list->head->pid == pid) {
		return list->head;
	} else {
		prev = FindPrevPD(pid, list);
		return prev->link;
	}

	return NULL;

}

// Dequeues pd from whatever list it might be in, 
// if it is in one.
void DequeuePD(struct PD* pd){

	if(!pd){
		return;
	}

	if (pd->inlist) { // inlist is not empty 
		if(pd->inlist->head == pd){ // inlist only has one element
			pd->inlist->head = pd->link;
		} else { // inlist has at least two elements
			struct PD *prev, *next; 
		
			prev = FindPrevPD(pd->pid, pd->inlist);
			next = prev->link->link;
			prev->link = next;
		}
	}
}	


typedef enum {False = 0, True = 1} bool;

// Creates a new PD.
struct PD* createPD(ProcessId pid, int priority, int waittime){
	struct PD *pd = malloc(sizeof(struct PD));
	if(!pd){
		printf("Error allocating space for a pd pointer");
		return NULL;
	}
	pd->pid = pid; pd->priority = priority; pd->waittime = waittime;
	return pd;
}

void test(){

	// Some PD structures to test on
	struct PD *pd1 = createPD(1,4,0);
	struct PD *pd2 = createPD(2,5,0);
	struct PD *pd3 = createPD(3,3,0);
	struct PD *pd4 = createPD(4,1,0);
	struct PD *pd5 = createPD(5,2,0);


	// Now we test for CreateList
	struct LL *priority_list = CreateList(L_PRIORITY);
	struct LL *wait_list = CreateList(L_WAITING);
	struct LL *lifo_list = CreateList(L_LIFO);
	struct LL *undef_list = CreateList(UNDEF);

	assert(priority_list != NULL);
	assert(wait_list != NULL);
	assert(lifo_list != NULL);
	assert(undef_list != NULL);

	// PriorityEnqueue test
	// Extreme cases: NULL values, 2^32 PDs (time permitting)
	assert(PriorityEnqueue(NULL,NULL) == FAIL);
	// General case
	assert(PriorityEnqueue(pd1, priority_list) == SUCC);
	assert(PriorityEnqueue(pd2, priority_list) == SUCC);
	assert(PriorityEnqueue(pd3, priority_list) == SUCC);
	assert(PriorityEnqueue(pd4, priority_list) == SUCC);
	assert(PriorityEnqueue(pd5, priority_list) == SUCC);
	assert(PriorityEnqueue(pd1, wait_list) == FAIL);
	assert(PriorityEnqueue(pd1, lifo_list) == FAIL);
	assert(PriorityEnqueue(pd1, undef_list) == FAIL);

	// DequeueHead test
	// Extreme cases: NULL values, 2^32 PDs (time permitting)
	assert(DequeueHead(NULL) == NULL);
	// General case
	assert(DequeueHead(priority_list) == pd4);
	assert(DequeueHead(wait_list) == NULL);
	assert(DequeueHead(lifo_list) == NULL);
	assert(DequeueHead(undef_list) == NULL);

	// Check DestroyList, then EnqueueAtHead then WaitListEnqueue.
	assert(DestroyList(NULL) == FAIL);
	assert(DestroyList(priority_list) == SUCC);
	assert(DestroyList(undef_list) == SUCC);

	// NOTE: Priority list and undef list no longer exist!

	// EnqueueAtHead test
	pd1 = createPD(1,4,0);
	pd2 = createPD(2,5,0);
	pd3 = createPD(3,3,0);
	pd4 = createPD(4,1,0);
	pd5 = createPD(5,2,0);
	assert(EnqueueAtHead(pd1, wait_list) == FAIL);
	assert(EnqueueAtHead(pd1, lifo_list) == SUCC);
	assert(EnqueueAtHead(pd2, lifo_list) == SUCC);
	assert(EnqueueAtHead(pd3, lifo_list) == SUCC);
	assert(EnqueueAtHead(pd4, lifo_list) == SUCC);
	assert(EnqueueAtHead(pd5, lifo_list) == SUCC);

	assert(DequeueHead(lifo_list) == pd5);
	assert(DequeueHead(lifo_list) == pd4);

	// NOTE: pd5 and pd4 are no longer in lifo_list!

	// WaitlistEnqueue test

	// FindPD test
	assert(FindPD(1,NULL) == NULL);
	assert(FindPD(4,lifo_list) == NULL);
	assert(FindPD(1,lifo_list) == pd1);
	assert(FindPD(2,lifo_list) == pd2);
	assert(FindPD(3,lifo_list) == pd3);

	// DequeuePD test
	DequeuePD(NULL);
	DequeuePD(pd1);
	assert(FindPD(1,lifo_list) == NULL);
	assert(FindPD(2,lifo_list) == pd2);

	assert(DestroyList(wait_list) == SUCC);

}


int main(void) {

	test();
	return 0;
}

