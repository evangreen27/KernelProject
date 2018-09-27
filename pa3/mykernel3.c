/* mykernel.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mykernel3.h"

#define FALSE 0
#define TRUE 1
/*	A sample semaphore table. You may change this any way you wish.  
 */


static struct {
	int valid;	// Is this a valid entry (was sem allocated)?
	int value;	// value of semaphore
	int proclist[MAXPROCS];
} semtab[MAXSEMS];
/*	InitSem () is called when kernel starts up.  Initialize data


 *	structures (such as the semaphore table) and call any initialization
 *	procedures here. 
 */

void InitSem ()
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {		// mark all sems free
		semtab[s].valid = FALSE;
	}
}

/*	MySeminit (p, v) is called by the kernel whenever the system
 *	call Seminit (v) is called.  The kernel passes the initial
 * 	value v, along with the process ID p of the process that called
 *	Seminit.  MySeminit should allocate a semaphore (find a free entry
 *	in semtab and allocate), initialize that semaphore's value to v,
 *	and then return the ID (i.e., index of the allocated entry). 
 */

int MySeminit (int p, int v)
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {
		if (semtab[s].valid == FALSE) {
			break;
		}
	}
	if (s == MAXSEMS) {
		Printf ("No free semaphores\n");
		return (-1);
	}

	semtab[s].valid = TRUE;
	semtab[s].value = v;

	return (s);
}

/*	MyWait (p, s) is called by the kernel whenever the system call
 *	Wait (s) is called.  
 */

void MyWait (p, s)
	int p;				// process
	int s; 				// semaphore
{
	int i;
	/* modify or add code any way you wish */
	semtab[s].value--;
	if (semtab[s].value < 0){
		for(i=0; i <= MAXPROCS; i++){
			if (semtab[s].proclist[i] == p){
					Printf("process %d already blocked, not blocking it a second time",p);
					return;
			}
		}
		for(i=0; i <= MAXPROCS; i++){
			if (semtab[s].proclist[i] == 0){
				semtab[s].proclist[i] = p;
				Block(p);
				return;
			}
		}

	}
}

/*	MySignal (p, s) is called by the kernel whenever the system call
 *	Signal (s) is called.  
 */

void MySignal (p, s)
	int p;				// process
	int s;				// semaphore
{
	/* modify or add code any way you wish */
	int i;
	semtab[s].value++;
	if(semtab[s].proclist[0] != 0){
		Unblock(semtab[s].proclist[0]); //unblock first waiting process
		for(i=0; i < MAXPROCS; i++){	//shift all processes left one space in queue
				semtab[s].proclist[i] = semtab[s].proclist[i+1];
		}
	}
	semtab[s].proclist[MAXPROCS] = 0; 	//set last process to 0 since it is in second to last spot now

}
