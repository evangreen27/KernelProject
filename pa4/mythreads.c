/*	User-level thread system
 *
 */

//based on piazza post, including this should be okay?
#include <strings.h>
#include <setjmp.h>

#include "aux.h"
#include "umix.h"
#include "mythreads.h"

#define STACKSIZE	65536		// maximum size of thread stack

static int MyInitThreadsCalled = 0;	// 1 if MyInitThreads called, else 0
static int recent;
static int lastThread;
static int position;
static int runningThreads;
static int currThread;

static struct thread {			// thread table
	int valid;			// 1 if entry is valid, else 0
	jmp_buf env;			// current context
	jmp_buf start;	//the starting env
	int fParam;
	void (*tFunc)();
} thread[MAXTHREADS];


int q[MAXTHREADS-1];	// thread queue

/*	MyInitThreads () initializes the thread package. Must be the first
 *	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	int i;

	if (MyInitThreadsCalled) {		// run only once
		Printf ("InitThreads: should be called only once\n");
		Exit ();
	}

	for (i = 0; i < MAXTHREADS; i++) {	// initialize thread table
		thread[i].valid = 0;
	}

	for (i = 0; i < MAXTHREADS-1; i++) {	// initialize queue
		q[i]=-1;
	}

	thread[0].valid = 1;			// initialize thread 0

	//initializing global variables
	runningThreads = 1;
	currThread = 0;
	recent = 0;
	position = 0;
	lastThread = -1; //there is no thread before the first
	
	MyInitThreadsCalled = 1;

	for(int j = 0; j < MAXTHREADS; j++) {
		char s[(STACKSIZE+1)*j];
		if(setjmp(thread[j].start)!=0) {	// if the env started at anything but 0
			(*(thread[currThread].tFunc)) (thread[currThread].fParam);
			MyExitThread();
		}
	}
}

/*	MyCreateThread (func, param) creates a new thread to execute
 *	func (param), where func is a function with no return value and
 *	param is an integer parameter.  The new thread does not begin
 *	executing until another thread yields to it. 
 */

int MyCreateThread (func, param)
	void (*func)();			// function to be executed
	int param;			// integer parameter
{
	
	int ind = recent+1;
	int qInd=0,safe,k,result;
	
	if (! MyInitThreadsCalled) {
		Printf ("CreateThread: Must call InitThreads first\n");
		Exit ();
	}
	
	if ( runningThreads > MAXTHREADS - 1 ){
		return -1;
	}
	
	position++;
	
	for( k = 0; k < MAXTHREADS; k++) {
		if( !thread[ind].valid ) {
			safe=ind;
			thread[safe].valid=1;
			runningThreads++;
			recent=ind;
			ind++;
			for(qInd; qInd < MAXTHREADS; qInd++) {
				if( q[qInd] == -1 ) {
					q[qInd] = safe;
					qInd = 0;
					break;
			}		
		}
		
		break;
		
		}		
		ind++;
	}

	result = safe;
	//using memcpy since prof said it was okay on piazza
	memcpy( thread[safe].env , thread[safe].start , sizeof(jmp_buf ));
	
	
	//set parameter and func of thread which now has memory allocated
	thread[safe].fParam=param;
	thread[safe].tFunc=func;
	
	
	return result;
}

/*	MyYieldThread (t) causes the running thread, call it T, to yield to
 *	thread t.  Returns the ID of the thread that yielded to the calling
 *	thread T, or -1 if t is an invalid ID. Example: given two threads
 *	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 *	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *	will resume by returning from its call to MyYieldThread (2), which
 *	will return the value 2.
 */

int MyYieldThread (t)
	int t;				// thread being yielded to
{
	int starting = currThread;
	if (! MyInitThreadsCalled) {
		Printf ("YieldThread: Must call InitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("YieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("YieldThread: Thread %d does not exist\n", t);
		return (-1);
	}

	lastThread = currThread;
	
	if(t == starting) {
		return starting;
	}
	
	currThread = t;
	
	int queueIndex,i = 0;

	for(i; i<(MAXTHREADS-1); i++) {
		if(t == q[i]) {
			queueIndex = i;
			for(i; i<(MAXTHREADS-2); i++){
				q[i]=q[i+1];
			}
			q[i+1] = -1;
			break;
		}
	}

    if (thread[lastThread].valid) {
		for(i=0;i<(MAXTHREADS-1);i++)
		{
			if(q[i]==-1)
			{
				q[i]=lastThread;
				break;
			}
		}
	}
	
	if(!setjmp(thread[lastThread].env)) {
		longjmp(thread[currThread].env,1);
	}
	return lastThread;
}

/*	MyGetThread () returns ID of currently running thread.
 */

int MyGetThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("GetThread: Must call InitThreads first\n");
		Exit ();
	}
	
	// return current id
	return currThread;
}

/*	MySchedThread () causes the running thread to simply give up the
 *	CPU and allow another thread to be scheduled.  Selecting which
 *	thread to run is determined here. Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads).  
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("SchedThread: Must call InitThreads first\n");
		Exit ();
	}
	
	// checks for if there are any valid threads
	if(!runningThreads) 
		Exit();
	if(!thread[q[0]].valid)
		return;
	
	//yield to first thing in queue
	MyYieldThread(q[0]);

}

/*	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("ExitThread: Must call InitThreads first\n");
		Exit ();
	}
	runningThreads--;
	thread[currThread].valid = 0;
	MySchedThread();
}
