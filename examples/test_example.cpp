#include "uthreads.h"
#include <iostream>
#include <unistd.h> // for sleep

using namespace std;

void thread_func_1() {
    while (true) {
        cout << "[Thread 1] ID: " << uthread_get_tid()
             << " | Quantums: " << uthread_get_quantums(uthread_get_tid())
             << " | Total: " << uthread_get_total_quantums() << endl;
        usleep(100000); // simulate work
    }
}

void thread_func_2() {
    int count = 0;
    while (true) {
        cout << "[Thread 2] ID: " << uthread_get_tid()
             << " | Count: " << ++count
             << " | Quantums: " << uthread_get_quantums(uthread_get_tid())
             << endl;

        if (count == 5) {
            cout << "[Thread 2] Going to sleep for 3 quantums" << endl;
            uthread_sleep(3);
        }
        usleep(120000);
    }
}

void thread_func_3() {
    int iterations = 0;
    while (iterations++ < 10) {
        cout << "[Thread 3] Working... Iteration " << iterations << endl;
        if (iterations == 4) {
            cout << "[Thread 3] Blocking Thread 1" << endl;
            uthread_block(1);
        }
        if (iterations == 7) {
            cout << "[Thread 3] Resuming Thread 1" << endl;
            uthread_resume(1);
        }
        usleep(150000);
    }

    cout << "[Thread 3] Done. Terminating itself." << endl;
    uthread_terminate(uthread_get_tid());
}

int main() {
    if (uthread_init(100000) == -1) { // 100ms quantum
        cerr << "Failed to initialize uthreads." << endl;
        return 1;
    }

    int tid1 = uthread_spawn(thread_func_1);
    int tid2 = uthread_spawn(thread_func_2);
    int tid3 = uthread_spawn(thread_func_3);

    if (tid1 == -1 || tid2 == -1 || tid3 == -1) {
        cerr << "Failed to spawn threads." << endl;
        return 1;
    }

    while (true) {
        usleep(500000); // Main thread idle loop
        cout << "[Main] Total Quantums: " << uthread_get_total_quantums() << endl;
    }

    return 0;
}
