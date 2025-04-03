#include "uthreads.h"
#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <csetjmp>
#include <csignal>
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>

#define RUNNING 1
#define READY 0
#define BLOCKED (-1)
using namespace std;


#define STACK_SIZE 4096

#ifdef __x86_64__
typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7
address_t translate_address(address_t addr) {
    address_t ret;
    asm volatile("xor %%fs:0x30,%0\n rol $0x11,%0\n" : "=g"(ret) : "0"(addr));
    return ret;
}
//using the demo file that was supplied to us

#else
typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5
address_t translate_address(address_t addr) {
    address_t ret;
    asm volatile("xor %%gs:0x18,%0\n rol $0x9,%0\n" : "=g"(ret) : "0"(addr));
    return ret;
}
#endif

//creating the basic Thread struct which contains the basics required for
//the exercise
struct Thread {
    int id;
    thread_entry_point entry_point;
    char stack[STACK_SIZE];
    sigjmp_buf env;
    int state; // RUNNING, READY, or BLOCKED
    int quantums;

    Thread(int tid, thread_entry_point entry) : id(tid), entry_point(entry), state(READY), quantums(0) {}
};

static vector<unique_ptr<Thread>> threads(MAX_THREAD_NUM);
static queue<int> ready_queue;
static int running_thread_id = 0;
static int total_quantums = 0;
static struct itimerval timer;
int counter =0;

void scheduler(int sig);



int uthread_init(int quantum_usecs) {
    if (quantum_usecs <= 0) {
        cerr << "thread library error: invalid quantum_usecs\n";
        return -1;
    }

    //making sure that the scheduler will be called when context switch needs to be performed

    struct sigaction sa = {0};
    sa.sa_handler = &scheduler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        cerr << "system error: sigaction failed\n";
        exit(1);
    }

    timer.it_value.tv_sec = quantum_usecs / 1000000;
    timer.it_value.tv_usec = quantum_usecs % 1000000;
    timer.it_interval.tv_sec = quantum_usecs / 1000000;
    timer.it_interval.tv_usec = quantum_usecs % 1000000;

    //setting the timer in the thread library initialization
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) < 0) {
        cerr << "system error: setitimer failed\n";
        exit(1);
    }
    //generating thge main thread and running him as the running thread
    threads[0] = std::unique_ptr<Thread>(new Thread(0, nullptr));
    threads[0]->state = RUNNING;
    threads[0]->quantums = 1;
    running_thread_id = 0;
    total_quantums = 1;

    // Save the context of the main thread
    sigsetjmp(threads[0]->env, 1);
    return 0;
}
//this program is translating the addresses of the pointers, according to the demo provided.
void setup_thread(int tid, char *stack, thread_entry_point entry_point) {
    address_t sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t)entry_point;
    sigsetjmp(threads[tid]->env, 1);
    (threads[tid]->env->__jmpbuf)[JB_SP] = translate_address(sp);
    (threads[tid]->env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&threads[tid]->env->__saved_mask);
}



int uthread_spawn(thread_entry_point entry_point) {
    if (entry_point == nullptr) {
        cerr << "thread library error: invalid entry_point\n";
        return -1;
    }

    // Find the next available thread ID (we already checked we did not exceed the limit!)
    int kid = -1;
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (threads[i] == nullptr) {
            kid = i;
            break;
        }
    }

    // Checking if we found an available thread ID
    if (kid == -1) {
        std::cerr << "thread library error: maximum number of threads reached\n";
        return -1;
    }

    threads[kid] = std::unique_ptr<Thread>(new Thread(kid, entry_point));
    // Checking allocation success
    if (!threads[kid]) {
        std::cerr << "thread library error: memory allocation failed" << std::endl;
        return -1;
    }
    setup_thread(kid, threads[kid]->stack, entry_point);
    ready_queue.push(kid);
    return kid;

}
//this program is making sure that everytime a thread is terminated, the next thread in the ready
//queue is deployed to be running, while deleting the thread after its termination.
int uthread_terminate(int did) {
    if (did < 0 || did >= MAX_THREAD_NUM || threads[did] == nullptr) {
        cerr << "thread library error: invalid thread ID\n";
        return -1;
    }

    if (did == 0) {
        exit(0);
    }
    threads[did].reset();

    // If the running thread is terminated, switch to the next one
    if (did == running_thread_id) {
        if (!ready_queue.empty()) {
            running_thread_id = ready_queue.front();
            ready_queue.pop();
            threads[running_thread_id]->state = RUNNING;
            threads[running_thread_id]->quantums++;
            total_quantums++;
            siglongjmp(threads[running_thread_id]->env, 1);
        } else {
            std::cerr << "thread library error: no threads to switch to\n";
            exit(1);
        }
    }

    if (did == running_thread_id) {
        delete threads[did].release();
        scheduler(0);
    } else {
        delete threads[did].release();
    }

    return 0;
}

int uthread_block(int tid) {
    if (tid <= 0 || tid >= MAX_THREAD_NUM || threads[tid] == nullptr) {
        cerr << "thread library error: invalid thread ID\n";
        return -1;
    }

    if (threads[tid]->state == BLOCKED) {
        return 0;
    }

    if (tid == running_thread_id) {
        threads[tid]->state = BLOCKED;
        scheduler(0);
    } else {
        threads[tid]->state = BLOCKED;
    }

    return 0;
}

int uthread_resume(int tid) {
    if (tid < 0 || tid >= MAX_THREAD_NUM || threads[tid] == nullptr) {
        cerr << "thread library error: invalid thread ID\n";
        return -1;
    }

    if (threads[tid]->state == BLOCKED) {
        threads[tid]->state = READY;
        ready_queue.push(tid);
    }

    return 0;
}

int uthread_sleep(int num_quantums) {

    if (running_thread_id == 0) {
        cerr << "thread library error: main thread cannot sleep\n";
        return -1;
    }

    if (num_quantums <= 0) {
        cerr << "thread library error: invalid num_quantums\n";
        return -1;
    }

    threads[running_thread_id]->state = BLOCKED;

    // Set the timer for sleep
    struct itimerval sleep_timer;
    sleep_timer.it_value.tv_sec = num_quantums * (timer.it_value.tv_sec);
    sleep_timer.it_value.tv_usec = num_quantums * (timer.it_value.tv_usec);
    sleep_timer.it_interval.tv_sec = 0;
    sleep_timer.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_VIRTUAL, &sleep_timer, NULL) < 0) {
        cerr << "system error: setitimer failed\n";
        exit(1);
    }

    // Trigger scheduler
    scheduler(0);

    return 0;
}


int uthread_get_tid() {
    return running_thread_id;
}

int uthread_get_total_quantums() {
    return total_quantums;
}

int uthread_get_quantums(int bid) {
    if (bid < 0 || bid >= MAX_THREAD_NUM || threads[bid] == nullptr) {
        cerr << "thread library error: invalid thread ID\n";
        return -1;
    }
    int check = threads[bid]->quantums;
    return check;
}

void scheduler(int sig) {

    counter++;

    if (sigsetjmp(threads[running_thread_id]->env, 1) != 0) {
        return; // Returning from a previously set jump
    }
    //if we are running and we finish our quantum we became ready
    if (threads[running_thread_id]->state == RUNNING) {
        threads[running_thread_id]->state = READY;
        ready_queue.push(running_thread_id);
    }
    //running the next thread if available

    if (!ready_queue.empty()) {
        running_thread_id = ready_queue.front();
        ready_queue.pop();
        threads[running_thread_id]->state = RUNNING;
        threads[running_thread_id]->quantums++;
        total_quantums++;

        siglongjmp(threads[running_thread_id]->env, 1);
    }
    else {
        std::cerr << "No threads ready to run!" << std::endl;
    }

}