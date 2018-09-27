/* mykernel2.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1	// in ticks (tick = 10 msec)
#define PROPNUM 100000

/*	A sample process table. You may change this any way you wish.  
 */

static struct {
	int valid;		// is this entry valid: 1 = yes, 0 = no
	int pid;		// process ID (as provided by kernel)
	int time;		// the time it was created
	int pass;
	int stride;
	int requested;
} proctab[MAXPROCS];

int clock;

/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h). Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks. 
 */

void InitSched ()
{
	int i;

	/* First, set the scheduling policy.  You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY). 
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not be
	 * called, thus leaving the policy to whatever we chose to test.  
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	// leave as is
		SetSchedPolicy (PROPORTIONAL);		// set policy here
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
	}

	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary). Returns 1 if successful, 0 otherwise.  
 */

int StartingProc (pid)
	int pid;			// process that is starting
{

	
	int nonreq=0; //total processes that don't request any cpu
	int reqqed=0;	//total cpu requested
	int i;
	clock++;
	
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;
			proctab[i].time = clock;
			proctab[i].pass = 0;
			proctab[i].stride = 0;
			proctab[i].requested = 0;
			if (GetSchedPolicy() == LIFO || GetSchedPolicy() == FIFO) {
				DoSched();
			}
			
			if (GetSchedPolicy() == PROPORTIONAL){
				for (int k = 0; k < MAXPROCS; k++){
					if ( proctab[k].valid){
						proctab[k].pass = 0;
						if ( proctab[k].requested == 0 ){
							nonreq++;
						}
						else{
							reqqed += proctab[k].requested;
						}
					}
				}
				for (int h = 0; h < MAXPROCS; h++){
					if ( proctab[h].valid){
						if ( proctab[h].requested == 0 ){
							if ( nonreq > 0){
								if (reqqed != 100){
									proctab[h].stride = (PROPNUM/((100-reqqed)/nonreq));
								}
								else{
									proctab[h].stride = 0;
								}
							}
						}
					}
				}
				if(proctab[i].pid == pid){
					proctab[i].pass = proctab[i].stride;		
				}
				
			}
			return (1);
		}
	}
	
	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc (pid)
	int pid;			// process that is ending
{
	int i;
	int nonreq = 0;
	int reqqed = 0;

	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && proctab[i].pid == pid) {
			proctab[i].valid = 0;
				
			if ( GetSchedPolicy() == PROPORTIONAL){
				for (int k = 0; k < MAXPROCS; k++){
					if ( proctab[k].valid){
						if ( proctab[k].requested == 0){
							nonreq++;
						}
						else{
							reqqed += proctab[k].requested;
						}
					}
				}
			}

			if ( GetSchedPolicy() == PROPORTIONAL){
				for (int h = 0; h < MAXPROCS; h++){
					if ( proctab[h].valid){
						proctab[h].pass = 0;
						if ( proctab[h].requested == 0){
							if ( nonreq > 0){
								if (reqqed != 100){
									proctab[h].stride = (PROPNUM/((100-reqqed)/nonreq));
								}
								else{
									proctab[h].stride = 0;
								}
							}
						}
					}
				}
			}

			/*
			for (int j = 0; j < MAXPROCS - 1; j++) {
				if (proctab[j].valid == 0 && proctab[j + 1].valid == 1) {

					proctab[j].pid = proctab[j + 1].pid;
					proctab[j].valid = proctab[j + 1].valid;
					proctab[j + 1].pid = 0;
					proctab[j + 1].valid = 0;
				}

			*/
			//if (proctab[j + 1].valid == 0) {
			//	proctab[j + 1].pid = pid;
			//}

			return(1);
		}
	}
	


	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy). SchedProc ()
 *	should return a process id, or 0 if there are no processes to run.  
 */

int SchedProc ()
{
	int i;
	int lowest;
	int lowestpid;
	int total;
	int lowestindex;
	int firstPid;
	int firstValid;

	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:
		lowest=0;
		lowestpid=0;
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid){
				if (lowest == 0){
					lowest = proctab[i].time;
					lowestpid = proctab[i].pid;
				}
				if (proctab[i].time < lowest){
					lowest = proctab[i].time;
					lowestpid = proctab[i].pid;
				}
			}
		}
		if (lowest != 0){
			return lowestpid;
		}
		break;

	case LIFO:

		lowest=0;  // not actually lowest, just named that like FIFO.
		lowestpid=0;
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid){
				if (lowest == 0){
					lowest = proctab[i].time;
					lowestpid = proctab[i].pid;
				}
				if (proctab[i].time > lowest){
					lowest = proctab[i].time;
					lowestpid = proctab[i].pid;
				}
			}
		}
		if (lowest != 0){
			return lowestpid;
		}

		break;

	case ROUNDROBIN:



		firstPid = proctab[0].pid;
		firstValid = proctab[0].valid;

		for (i = 0; i < MAXPROCS - 1; i++) {
			if (proctab[i + 1].pid == 0) {
				break;
			}
			proctab[i].pid = proctab[i + 1].pid;
			proctab[i].valid = proctab[i + 1].valid;
		}

		proctab[i].pid = firstPid;
		proctab[i].valid = firstValid;
				
				
		for(int k = 0; k < MAXPROCS; k++){
			if (proctab[0].valid) {
				return (proctab[0].pid);
			}
			else{
			firstPid = proctab[0].pid;
			firstValid = proctab[0].valid;


			for (i = 0; i < MAXPROCS - 1; i++) {
				if (proctab[i + 1].pid == 0) {
					break;
				}
				proctab[i].pid = proctab[i + 1].pid;
				proctab[i].valid = proctab[i + 1].valid;
			}

			proctab[i].pid = firstPid;
			proctab[i].valid = firstValid;
			}
		}
	

		break;

	case PROPORTIONAL:

	
		lowest = -1; 
		lowestindex = -1;
		
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid){
				if (lowest == -1){
					lowest = proctab[i].pass;
					lowestindex = i;
				}
				if (proctab[i].pass < lowest){
					lowest = proctab[i].pass;
					lowestindex = i;
				}
			}
		}
		
		if (proctab[lowestindex].valid){
			proctab[lowestindex].pass = proctab[lowestindex].pass + proctab[lowestindex].stride;
			return proctab[lowestindex].pid;
		}

		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.  
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);

	switch (GetSchedPolicy ()) {	// is policy preemptive?

	case ROUNDROBIN:		// ROUNDROBIN is preemptive
		DoSched();
		break;
	case PROPORTIONAL:		// PROPORTIONAL is preemptive

		DoSched ();		// make scheduling decision
		break;

	default:			// if non-preemptive, do nothing
		break;
	}
}

/*	MyRequestCPUrate (pid, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (n). This is a request for
 *	n% of CPU time, i.e., requesting a CPU whose speed is effectively
 *	n% of the actual CPU speed.  Roughly n out of every 100 quantums
 *	should be allocated to the calling process.  n must be greater than
 *	0 and must be less than or equal to 100. MyRequestCPUrate (pid, n)
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	n < 1 or n > 100).  If MyRequestCPUrate (pid, n) fails, it should
 *	have no effect on scheduling of this or any other process, i.e., AS
 *	IF IT WERE NEVER CALLED.
 */

int MyRequestCPUrate (pid, n)
	int pid;			// process whose rate to change
	int n;				// percent of CPU time
{
	int reqqed=0;
	int nonreq=0;
	
	if (GetSchedPolicy() != PROPORTIONAL) {
		return (-1);
	}
	
	//calculates the total cpu requested 	
	for (int h = 0; h < MAXPROCS; h++){
		if ( proctab[h].valid){
			if ( proctab[h].requested != 0 && proctab[h].pid != pid ){
				reqqed += proctab[h].requested;
			}
		}
	}
	
	//checks if cpu will go over 100
	if (n <= 0 || n > (100 - reqqed) ){
		return (-1);
	}
	

	
	//determines how many processes exist which don't request cpu
	for (int f = 0; f < MAXPROCS; f++){
		if ( proctab[f].valid){
			if ( proctab[f].requested == 0 ){
				if ( proctab[f].pid != pid){
					nonreq++;
				}
			}
		}
	}
	

	//reset strides and passes and update this process
	for (int j = 0; j < MAXPROCS; j++){
		if ( proctab[j].valid){
			proctab[j].pass = 0;
			if (proctab[j].pid == pid) {
				proctab[j].requested = n;
				proctab[j].stride = (PROPNUM/n);
				proctab[j].pass = proctab[j].stride;
			}
			if ( nonreq > 0 ){
				if (proctab[j].requested == 0){
					if((reqqed + n) != 100){
						proctab[j].stride = (PROPNUM/((100 - (reqqed + n))/nonreq));
					}
					else{
						proctab[j].stride = 0;
					}
				}
			}
			
		}
	}
	

	

	return (0);
}
