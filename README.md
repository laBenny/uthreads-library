<<<<<<< HEAD
# uthreads: User-Level Thread Library in C++

This project implements a user-level threading library in C++, inspired by POSIX ucontext but built using `sigsetjmp`, `siglongjmp`, virtual timers, and signal handling.

##  Features

- Cooperative multithreading using user-space context switching
- Timer-based preemptive scheduling
- Thread creation, blocking, resuming, and termination
- Quantum tracking per thread
- Main thread management (thread 0)
- Basic round-robin scheduling

## Getting Started

### Prerequisites

- A Linux-based environment (tested on Ubuntu)
- C++ compiler (e.g., `g++`)

### Compilation

Use the provided Makefile (or compile manually):

```bash
make
=======
# uthreads: User-Level Thread Library in C++

This project implements a user-level threading library in C++, inspired by POSIX ucontext but built using `sigsetjmp`, `siglongjmp`, virtual timers, and signal handling.

##  Features

- Cooperative multithreading using user-space context switching
- Timer-based preemptive scheduling
- Thread creation, blocking, resuming, and termination
- Quantum tracking per thread
- Main thread management (thread 0)
- Basic round-robin scheduling

## Getting Started

### Prerequisites

- A Linux-based environment (tested on Ubuntu)
- C++ compiler (e.g., `g++`)

### Compilation

Use the provided Makefile (or compile manually):

```bash
make
>>>>>>> ba6179b (Initial commit of uthreads project)
