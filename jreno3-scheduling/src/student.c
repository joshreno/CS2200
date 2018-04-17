/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "os-sim.h"

#include <string.h>

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

void enqueue(pcb_t *proc_to_add);
pcb_t* dequeue(void);
pcb_t* other_dequeue(void);

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */

typedef enum {
    FIFO,
    ROUND_ROBIN,
    SRTF
} scheduler;
static pcb_t **current;
static pthread_mutex_t current_mutex;
static pcb_t *queue_head;
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static scheduler algorithm;
static int time_slice;
static unsigned int srtf;

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running process indexed by the cpu id. 
 *	See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t* pcb;
    if (algorithm != SRTF) {
        pcb = dequeue();
    } else {
        pcb = other_dequeue();
    }
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = pcb;
    pthread_mutex_unlock(&current_mutex);
    if (pcb) {
    	pcb->state = PROCESS_RUNNING;
        context_switch(cpu_id, pcb, time_slice);
    } else {
        context_switch(cpu_id, NULL, time_slice);
    }   
}

/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&mutex);
    while(!queue_head) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    schedule(cpu_id);
}

/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_READY;
    enqueue(current[cpu_id]);
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is SRTF, wake_up() may need
 *      to preempt the CPU with the highest remaining time left to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a lower remaining time left than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    process->state = PROCESS_READY;
    enqueue(process);
    if (algorithm == SRTF) {
        int cpus = 0;
        pthread_mutex_lock(&current_mutex);
        for (unsigned int i = 0; i < srtf; i++) {
            if (!current[i]) {cpus++;}
        }
        pthread_mutex_unlock(&current_mutex);
        if (cpus == 0) {
            int x = -1;
            pthread_mutex_lock(&current_mutex);
            for (unsigned int i = 0; i < srtf; i++) {
                if (current[i]->time_remaining > process->time_remaining) {
                    if (x == -1 || current[i]->time_remaining > current[x]->time_remaining) {
                        x = (int) i;
                    }
                }
            }
            pthread_mutex_unlock(&current_mutex); 
            if (x != -1) {
                force_preempt((unsigned int)x);
            }
        }
    }
}

/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -s command-line parameters.
 */
int main(int argc, char *argv[])
{
    /* Parse command-line arguments */
    unsigned int cpu_count = strtoul(argv[1], NULL, 0);
    srtf = cpu_count;
    if (argc < 2 || argc > 5)
    {
        fprintf(stderr, "CS 2200 OS Sim -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -s ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -s : Shortest Remaining Time First Scheduler\n\n");
        return -1;
    } else if (argc == 3) {
    	if (strcmp(argv[2], "-s") == 0) {
            algorithm = SRTF;
            time_slice = -1;
        }
    } else if (argc == 4) {
    	if (strcmp(argv[2], "-r") == 0) {
            algorithm = ROUND_ROBIN;
            time_slice = (int) strtol(argv[3], NULL, 10);
        }
    } else if (argc == 2) {
    	algorithm = FIFO;
        time_slice = -1;
    }
    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    /* Start the simulator in the library */
    start_simulator(cpu_count);
    return 0;
}

void enqueue(pcb_t *proc_to_add) {
    pthread_mutex_lock(&mutex);
    pcb_t *head = queue_head;
    if (head) {
    	while(head->next != NULL) {
            head = head->next;
        }
        head->next = proc_to_add;
    } else {
        queue_head = proc_to_add;
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

pcb_t* dequeue() {
    pthread_mutex_lock(&mutex);
    pcb_t *head = queue_head;
    if (queue_head) {
        queue_head = head->next;
        head->next = NULL;
    }
    pthread_mutex_unlock(&mutex);
    return head;
}

pcb_t* other_dequeue() {
    pthread_mutex_lock(&mutex);
    pcb_t *first = queue_head;
    pcb_t *second = queue_head;
    if (first != NULL) {
        while (second != NULL) {
            if (second->time_remaining < first->time_remaining) {
                first = second;
            }
            second = second->next;
        }
        second = queue_head;
        if (second == first) {
            queue_head = first->next;
            first->next = NULL;
        } else {
            while (second->next != first) {
                second = second->next;
            }
            second->next = second->next->next;
            first->next = NULL;
        }
    }
    pthread_mutex_unlock(&mutex);
    return first;
}