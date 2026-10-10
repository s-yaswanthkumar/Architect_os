# 🏛️ Architect OS: Modular C11 Systems Shell & OS Runtime Emulator

[![C11 Standard](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Compiler Flags](https://img.shields.io/badge/Compiler%20Flags--Wall%20-Wextra%20-pedantic%20-pthread-success.svg)](https://gcc.gnu.org/)
[![Automated Test Suite](https://img.shields.io/badge/Test%20Suite-24%2F24%20Passing%20(100%25)-brightgreen.svg)](#-automated-verification-test-suite)
[![Valgrind](https://img.shields.io/badge/Valgrind-100%25%20Zero%20Leaks-brightgreen.svg)](#-valgrind-memory-safety-verification)
[![OS Target](https://img.shields.io/badge/OS%20Kernel-Linux%20%2F%20POSIX%20%2F%20WSL2-orange.svg)](https://ubuntu.com/)

> **Course Alignment:** 25CS2104E — Operating Systems and Systems Programming (CO-1 through CO-6)  
> **Architecture:** Low-Level Modular Linux User-Space Systems Software & Runtime Emulator  
> **Core Differentiator:** Real-Time Under-the-Hood Command Visualizer & Explainable Systems Engine

---

## 📖 Table of Contents
1. [Architecture Overview](#-architecture-overview)
2. [Dual Execution Telemetry Modes](#-dual-execution-telemetry-modes)
   - [Mode 1: Simple Summary Mode](#mode-1-simple-summary-mode-mode-summary)
   - [Mode 2: Advanced 6-Step Visualizer](#mode-2-advanced-6-step-visualizer-mode-advanced)
   - [Mode 3: Silent Execution](#mode-3-silent-execution-mode-off)
3. [Course Outcomes (CO-1 to CO-6) Technical Deep-Dive & Verification](#-course-outcomes-co-1-to-co-6-technical-deep-dive--verification)
   - [CO-1: OS Service Layer & Syscall Introspection](#co-1-the-os-as-a-service-layer--syscall-introspection)
   - [CO-2: Process Control & Process Lifecycle](#co-2-process-control--user-level-process-lifecycle)
   - [CO-3: Inter-Process Communication (IPC) & Job Control](#co-3-inter-process-communication-ipc-signals--job-control)
   - [CO-4: Virtual Memory Management & Introspection](#co-4-virtual-memory-management--memory-introspection)
   - [CO-5: Linux Virtual File System (VFS), Inodes & Redirection](#co-5-linux-virtual-file-system-vfs-inodes--redirection)
   - [CO-6: Concurrency, POSIX Threads & Synchronization Hazards](#co-6-concurrency-posix-threads--synchronization-hazards)
4. [Quickstart: Build & Execution](#-quickstart-build--execution)
5. [Automated Verification Test Suite](#-automated-verification-test-suite)
6. [Valgrind Memory Safety Verification](#-valgrind-memory-safety-verification)
7. [Lab Evaluation / Viva Demonstration Cheat Sheet](#-lab-evaluation--viva-demonstration-cheat-sheet)
8. [Project File Tree](#-project-file-tree)

---

## 🏗️ Architecture Overview

Architect OS is decoupled into modular user-space subsystems following professional Linux systems programming conventions:

```
                              +---------------------------------------+
                              |         Interactive REPL Loop         |
                              |   (Prompt, Backspace, History, Sig)   |
                              +-------------------+-------------------+
                                                  |
                                                  v
                              +---------------------------------------+
                              |         Lexer & Vector Parser         |
                              |  (Dynamic Vec, Quotes, Pipeline AST)  |
                              +-------------------+-------------------+
                                                  |
                                                  v
                      +---------------------------+---------------------------+
                      |                                                       |
                      v                                                       v
+---------------------------------------------+       +-----------------------------------------------+
|         Kernel Telemetry Visualizer         |       |              Execution Dispatcher             |
|  * Mode 1: Plain-English Command Summary    |       |  * Built-in Systems Commands (CO-1 -> CO-6)   |
|  * Mode 2: Advanced 6-Step Engine:          | <---> |  * Multi-Stage Anonymous Pipelines (pipe/dup2)|
|    - [Step 1] AST Blueprint                 |       |  * Stream Redirection (<, >, >>, 2>, 2>&1)    |
|    - [Step 2] Ring 3 -> Ring 0 Hardware Trap|       |  * POSIX Job Control Subsystem (jobs, fg, bg) |
|    - [Step 3] PCB Lifecycle Transitions     |       |  * Foreground/Background Process Group Switch |
|    - [Step 4] Virtual Memory Address Space  |       +-----------------------------------------------+
|    - [Step 5] Inode & VFS Delta Telemetry   |
|    - [Step 6] IPC Plumbing & Exit Accounting|
+---------------------------------------------+
```

---

## 🌟 Dual Execution Telemetry Modes

Architect OS features a runtime-switchable telemetry engine that allows switching between **concise conceptual summaries** and **deep low-level kernel introspection**:

```text
architect-os:~$ mode
=== Architect OS Execution Telemetry Modes ===
  Current Active Mode : ADVANCED (Deep 6-Step Visualizer)

  Toggle Commands:
    mode summary   - Concise plain-English summary of what each command does
    mode advanced  - Full 6-step AST, Ring 3->0 trap, PCB, VMM, VFS & IPC dashboard
    mode off       - Disable all telemetry for standard silent execution
```

---

### Mode 1: Simple Summary Mode (`mode summary`)
Designed for clear, rapid explanation. When any command is executed, Architect OS inspects the command AST and outputs a clean 2–3 line summary box explaining the command's real-world action, arguments, and data flow.

#### Example A: Single Utility Command (`ls -la`)
```text
architect-os:~$ mode summary
Execution Mode set to: SUMMARY (Plain-English Command Explanations)

architect-os:~$ ls -la
+-----------------------------------------------------------------------------------------+
|  [Command Summary Mode]                                                                 |
+-----------------------------------------------------------------------------------------+
  Command     : ls -la
  Description : Lists directory contents, file names, permissions, and sizes.
  Arguments   : -la 
+-----------------------------------------------------------------------------------------+
```

#### Example B: Multi-Stage Pipeline (`cat README.md | grep Syllabus`)
```text
architect-os:~$ cat README.md | grep Syllabus
+-----------------------------------------------------------------------------------------+
|  [Command Summary Mode]                                                                 |
+-----------------------------------------------------------------------------------------+
  Pipeline    : cat README.md | grep Syllabus
  Description : Chains 2 commands via kernel anonymous pipes (zero temp files).
  Data Flow   : [cat] ---> (kernel pipe) ---> [grep]
+-----------------------------------------------------------------------------------------+
```

#### Example C: Built-in Subsystem Command (`sysinspect ls`)
```text
architect-os:~$ sysinspect ls
+-----------------------------------------------------------------------------------------+
|  [Command Summary Mode]                                                                 |
+-----------------------------------------------------------------------------------------+
  Command     : sysinspect ls
  Description : [CO-1] Inspects hardware Ring 3->0 trap, intercepts syscalls, and reads /proc/sys limits.
  Arguments   : ls 
+-----------------------------------------------------------------------------------------+
```

---

### Mode 2: Advanced 6-Step Visualizer (`mode advanced`)
Designed for systems debugging, architecture demonstration, and academic evaluation. Every command displays a 6-step telemetry dashboard:

```text
+-----------------------------------------------------------------------------------------+
|  [Step 1: Parsing & AST Blueprint]                                                      |
+-----------------------------------------------------------------------------------------+
  Raw Input   : "cat < input.txt | grep kernel > output.txt"
  Pipeline AST: [input.txt] -> [Command: cat] -> [Pipe Out]  |  [Pipe In] -> [Command: grep] -> [output.txt]
+-----------------------------------------------------------------------------------------+
|  [Step 2: User Space -> Kernel Space Mode Switch (CO-1)]                                |
+-----------------------------------------------------------------------------------------+
  Hardware Trap    : CPU CPL 3 (User Ring 3) --[syscall instruction]--> CPL 0 (Kernel Ring 0)
  Low-Level Entry  : entry_SYSCALL_64 via arch/x86/entry/entry_64.S
  Target Subsystem : sys_clone (Process Fork) -> sys_execve (ELF Binary Loader)
  Register Save    : User %rip -> %rcx, %rflags -> %r11; Kernel TSS switches %rsp to interrupt stack.
+-----------------------------------------------------------------------------------------+
|  [Step 3: Process Lifecycle & PCB Allocation (CO-2)]                                    |
+-----------------------------------------------------------------------------------------+
  Parent Process   : PID 1024 (Architect OS Interactive Shell)
  Spawned Child    : PID 1025 (Task: 'cat')
  State Transition : [NEW PCB allocated] -> [READY in CFS runqueue] -> [RUNNING on CPU Core]
+-----------------------------------------------------------------------------------------+
|  [Step 4: Memory Space Layout Changes (CO-4)]                                           |
+-----------------------------------------------------------------------------------------+
  Target Address Space (PID: 1025):
    * Text Segment     : Loaded ELF PT_LOAD segments (.text executable code)
    * Data / BSS       : Initialized globals and uninitialized zero-fill pages
    * Heap Pointer     : mm->brk initialized; dynamic heap ready via sbrk/mmap
    * User Stack Frame : Initialized with argc, argv pointers, and envp environment block
+-----------------------------------------------------------------------------------------+
|  [Step 5: File & Inode State Delta (CO-5)]                                              |
+-----------------------------------------------------------------------------------------+
  Target Path      : output.txt
  Before Execution : Inode: [None]    | Size: 0 bytes      | Status: Unallocated
  After Execution  : Inode: #4192841  | Size: 240 bytes    | Status: Committed to Disk
  Plain-English    : File 'output.txt' created: Inode #4192841, Size: 240 bytes allocated on filesystem block.
+-----------------------------------------------------------------------------------------+
|  [Step 6: IPC & Exit Code Accounting (CO-2, CO-3)]                                      |
+-----------------------------------------------------------------------------------------+
  IPC Plumbing     : Anonymous Pipeline: 2 processes linked via 1 kernel pipe (dup2 redirected)
  Reaping Status   : WIFEXITED(status) == true -> Exit Code: 0 (Success)
+-----------------------------------------------------------------------------------------+
```

---

### Mode 3: Silent Execution (`mode off`)
Disables all telemetry banners for standard, quiet shell usage.

---

## 🔬 Course Outcomes (CO-1 to CO-6) Technical Deep-Dive & Verification

| Syllabus Outcome | Core Technical Competency | Built-in Commands & Features |
| :--- | :--- | :--- |
| **CO-1** | OS Service Layer & Syscall Introspection | `sysinspect <cmd>`, `mode`, `visualize`, `history` |
| **CO-2** | Process Control & Process Lifecycle | `procstate [pid]`, `zombiedemo`, `orphandemo`, `scheduler-sim [RR\|EDF\|SRTF] [quantum]` |
| **CO-3** | Inter-Process Communication & Job Control | Anonymous Pipelines (`\|`), `fifowrite`, `fiforead`, `socket-daemon`, `socket-client`, `jobs`, `fg`, `bg` |
| **CO-4** | Virtual Memory & Memory Introspection | `vmmaps [pid]`, `cowdemo`, `arch-alloc <bytes>`, `arch-free <ptr>`, `heapinfo` |
| **CO-5** | Linux VFS, Inodes & Stream Redirection | Stream Redirections (`<`, `>`, `>>`, `2>`, `2>&1`), `inodeinfo <path>`, `mmapio <file> [find] [rep]` |
| **CO-6** | POSIX Threads & Synchronization Hazards | `threads-demo <thr> <iter> [sync\|nosync]`, `pc-demo <items> <cap>`, `deadlock-demo [hazard\|prevent]` |

---

### CO-1: The OS as a Service Layer & Syscall Introspection

#### What It Does:
* **Interactive REPL Architecture:** Implements raw/cooked terminal line reading, backspace character processing (`\b \b`), dynamic buffer expansion, and history tracking.
* **Hardware Mode Switch Introspection:** Explains the transition from User Mode (Ring 3, Current Privilege Level CPL=3) to Kernel Mode (Ring 0, CPL=0) via the x86_64 `syscall` instruction (opcode `0x0F 0x05`). Shows hardware register preservation (user `%rip` $\rightarrow$ `%rcx`, `%rflags` $\rightarrow$ `%r11`, stack pointer `%rsp` switched via Task State Segment (TSS) to the kernel interrupt stack).
* **Dynamic Kernel Syscall Tracing:** Executes commands under `ptrace(PTRACE_SYSCALL)` to intercept and reconstruct the exact system call sequence dispatched to `sys_call_table`.
* **Kernel Parameter Inspection:** Directly inspects `/proc/sys` OS service limits:
  * `/proc/sys/kernel/pid_max` (process table capacity)
  * `/proc/sys/fs/file-max` (system-wide VFS file descriptor ceiling)
  * `/proc/sys/fs/pipe-max-size` (IPC buffer limits)
  * `/proc/sys/vm/swappiness` (virtual memory swapping ratio)

#### Verification Command:
```text
architect-os:~$ sysinspect ls -la
```
#### Expected Telemetry:
* Renders the Ring 3 $\rightarrow$ Ring 0 hardware trap diagram.
* Intercepts and logs dynamic system calls (`sys_execve`, `sys_brk`, `sys_mmap`, `sys_openat`, `sys_newfstatat`, `sys_write`, `sys_close`, `sys_exit_group`).
* Prints kernel parameter limits from `/proc/sys`.

---

### CO-2: Process Control & User-Level Process Lifecycle

#### What It Does:
* **Safe Process Control:** Implements safe `fork()`, program transformation via `execvp()`, and deterministic child synchronization using `waitpid()`.
* **Status Macro Accounting:** Decodes `WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`, `WTERMSIG`, `WIFSTOPPED`, `WSTOPSIG`.
* **PCB Lifecycle Introspection (`procstate [pid]`):** Reads `/proc/[pid]/stat` and `/proc/[pid]/status` to decode states (`R`, `S`, `D`, `Z`, `T`), `VmSize`, `VmRSS`, active threads, voluntary context switches (I/O yields), non-voluntary context switches (quantum expirations), and displays an ASCII State Machine diagram.
* **Zombie Prevention & Reaping (`zombiedemo`):** Intentionally spawns a child that exits immediately while the parent delays calling `waitpid()`. Shows the child in state `'Z'` (defunct PCB in process table). The parent then executes `waitpid()` and proves the child entry is purged from the table.
* **Orphan Adoption (`orphandemo`):** Spawns a child whose parent exits; proves the child's PPID changes to PID 1 (`systemd`/`init`).
* **User-Level Scheduler Simulator (`scheduler-sim`):** Simulates tick-by-tick CPU scheduling under three algorithms:
  1. **Preemptive Round-Robin (RR):** Configurable time-slice quantum with runqueue preemption.
  2. **Earliest Deadline First (EDF):** Dynamic priority scheduling based on shortest remaining deadline.
  3. **Shortest Remaining Time First (SRTF):** Preemptive shortest-job scheduler.
  Generates ASCII Gantt charts, per-task finish/waiting/turnaround times, and context switch counts.

#### Verification Commands:
```text
architect-os:~$ procstate
architect-os:~$ zombiedemo
architect-os:~$ orphandemo
architect-os:~$ scheduler-sim RR 2
architect-os:~$ scheduler-sim EDF
architect-os:~$ scheduler-sim SRTF
```
#### Expected Telemetry:
* `zombiedemo`: Confirms state `'Z'` in Phase 1, followed by clean reaping with exit code 42 in Phase 2.
* `scheduler-sim`: Displays Gantt charts (`| P1 | P1 | P2 | P2 | ... |`), turnaround times, and average waiting times.

---
-
### CO-3: Inter-Process Communication (IPC), Signals & Job Control

#### What It Does:
* **Anonymous Multi-Stage Pipelines:** Supports arbitrary multi-stage pipelines (`cmd1 | cmd2 | cmd3 | ...`). Uses `pipe()` and `dup2()`. Closes all intermediate descriptor ends in both child and parent to avoid leaks and ensure proper EOF delivery.
* **Named Pipes (FIFOs):** Persistent filesystem IPC using `mkfifo()`. Built-in commands `fifowrite <path> <msg>` and `fiforead <path>` with `O_NONBLOCK` support.
* **UNIX Domain Sockets:** Standalone IPC mode (`--daemon` and `--client`) and built-ins (`socket-daemon`, `socket-client`) communicating over `/tmp/architect_os.sock` via `socket(AF_UNIX, SOCK_STREAM, 0)`.
* **POSIX Signal Architecture:**
  * Reliable signal registration via `sigaction()`.
  * Shell ignores `SIGINT` (Ctrl-C) and `SIGTSTP` (Ctrl-Z) while routing them to the active foreground process group via `tcsetpgrp()`.
  * Asynchronous background reaper handles `SIGCHLD` using `waitpid(-1, &status, WNOHANG)` to reap finished background tasks (`&`) without user latency.
* **Job Control Subsystem:**
  * Maintains a `Job` table tracking Job ID, PGID, PID, State (`RUNNING`, `STOPPED`, `DONE`), and Command Line.
  * `jobs`: Displays active jobs table.
  * `fg <job_id>`: Restores terminal control and sends `SIGCONT`.
  * `bg <job_id>`: Resumes stopped job in background with `SIGCONT`.

#### Verification Commands:
```text
# 1. Anonymous multi-stage pipeline
architect-os:~$ echo "operating systems and systems programming" | tr a-z A-Z | rev

# 2. Named Pipes (FIFOs)
architect-os:~$ fifowrite /tmp/demo_fifo "Hello Architect OS FIFO"
architect-os:~$ fiforead /tmp/demo_fifo

# 3. Job Control (Background, Stop, Foreground, Resume)
architect-os:~$ sleep 50 &
architect-os:~$ jobs
architect-os:~$ fg 1
# (Press Ctrl-Z to stop)
architect-os:~$ bg 1
```

---

### CO-4: Virtual Memory Management & Memory Introspection

#### What It Does:
* **Custom Dynamic Heap Allocator:**
  * Explicit free-list memory allocator using `mmap()` (2 MB arena).
  * Implements `arch_malloc(size_t)` and `arch_free(void *)` with 16-byte alignment, block header metadata (32 bytes, magic cookie `0xAC1984C0`), block splitting, and bidirectional contiguous coalescing to eliminate fragmentation.
  * Built-in commands: `arch-alloc <bytes>`, `arch-free <ptr>`, and `heapinfo` (reports block counts, allocated bytes, free pool %, and fragmentation index).
* **Virtual Memory Address Space Inspector (`vmmaps [pid]`):**
  * Parses `/proc/[pid]/maps` to categorize memory segments:
    * **Text Segment (`r-xp`)**: Executable code loaded from ELF binary.
    * **Data / BSS (`rw-p`)**: Initialized and uninitialized globals.
    * **Heap (`[heap]`)**: Dynamic allocations.
    * **Memory Mapping Segment**: Shared libraries (`libc.so`), anonymous `mmap` regions, `[vdso]`.
    * **User Stack (`[stack]`)**: Stack frames, local variables, return addresses.
* **Copy-on-Write (COW) Demonstrator (`cowdemo`):**
  * Allocates a page-aligned buffer using `mmap()`, populates it, and inspects physical Page Frame Numbers (PFNs) via `/proc/self/pagemap`.
  * Forks a child: both processes initially share the same physical RAM page (marked read-only in page table).
  * Child triggers a write: the hardware MMU raises **Page Fault #14**, prompting the Linux VM subsystem to allocate a brand new physical frame, duplicate data, update PTE permissions, and flush the TLB. Shows physical PFNs diverging while virtual addresses remain identical.
* **Valgrind Compliance:** 100% clean memory deallocation (`0 errors from 0 contexts, 0 bytes leaked`).

#### Verification Commands:
```text
architect-os:~$ vmmaps
architect-os:~$ cowdemo
architect-os:~$ arch-alloc 256
architect-os:~$ arch-alloc 512
architect-os:~$ heapinfo
architect-os:~$ arch-free <payload_address_from_table>
architect-os:~$ heapinfo
```
#### Expected Telemetry:
* `heapinfo`: Displays block addresses, payload sizes, allocated vs free states, and fragmentation index (0.0000).
* `cowdemo`: Displays physical frame allocation divergence upon write.

---

### CO-5: Linux Virtual File System (VFS), Inodes & Redirection

#### What It Does:
* **Stream Redirection Operators:**
  * `<` : stdin input redirection (`O_RDONLY`).
  * `>` : stdout truncated output redirection (`O_WRONLY | O_CREAT | O_TRUNC`).
  * `>>`: stdout append redirection (`O_WRONLY | O_CREAT | O_APPEND`).
  * `2>`: stderr redirection to file.
  * `2>&1`: stderr duplicated to stdout file descriptor.
* **Differential VFS Inode Telemetry:** Automatically takes snapshots before and after execution (`vfs_take_snapshot` and `vfs_calculate_delta`), calculating inode allocation, byte size deltas, and filesystem block commits.
* **VFS Inode Inspector (`inodeinfo <path>`):** Queries `lstat()` to display:
  * Inode number and Device ID (Major/Minor numbers).
  * Hard link count (`st_nlink`).
  * Octal and rwx permission bits (`st_mode`).
  * Owner UID / GID.
  * Byte size and optimal I/O block transfer size (`st_blksize`).
  * Physical 512-byte blocks allocated on disk (`st_blocks`).
  * Access, modification, and change timestamps.
* **Memory-Mapped File I/O Engine (`mmapio <file> [find] [replace]`):**
  * Opens file and maps it into process virtual address space via `mmap(..., PROT_READ | PROT_WRITE, MAP_SHARED, ...)`.
  * Performs in-place text replacement directly via memory pointer dereference.
  * Calls `msync(MS_SYNC)` to write dirty pages back to storage.
  * Benchmarks nanosecond/microsecond zero-copy latency against standard buffered `fread`/`fwrite`.

#### Verification Commands:
```text
architect-os:~$ echo "Operating Systems Lab" > /tmp/demo_vfs.txt
architect-os:~$ cat < /tmp/demo_vfs.txt
architect-os:~$ echo "Appended Line" >> /tmp/demo_vfs.txt
architect-os:~$ ls /nonexistent_dir 2> /tmp/demo_err.txt
architect-os:~$ inodeinfo /tmp/demo_vfs.txt
architect-os:~$ mmapio /tmp/demo_vfs.txt "Operating" "LowLevel-"
```
#### Expected Telemetry:
* Step 5 shows file creation with inode number and byte delta.
* `inodeinfo`: Prints detailed VFS inode metadata.
* `mmapio`: Displays microsecond zero-copy I/O benchmark vs buffered I/O.

---

### CO-6: Concurrency, POSIX Threads & Synchronization Hazards

#### Goal:
Demonstrate multithreaded data race conditions, mutual exclusion via mutexes, thread-safe bounded buffers, and deadlock resolution via resource hierarchy.

#### What It Does:
* **Multithreaded Data Race vs Mutex Synchronization (`threads-demo`):**
  * Spawns worker threads via `pthread_create()` and coordinates via `pthread_join()`.
  * `nosync` mode: Unprotected concurrent execution of `counter++` causes read-modify-write CPU data race. Displays missed increments and percentage data loss.
  * `sync` mode: Protects the critical section with `pthread_mutex_t` (`lock`/`unlock`). Proves **Race Delta = 0 (100% thread safe)**.
* **Producer-Consumer Bounded Queue (`pc-demo <items> <capacity>`):**
  * Thread-safe circular bounded buffer coordinated by POSIX counting semaphores (`sem_empty`, `sem_full`) and a mutex lock.
  * Visualizes live buffer occupancy bars: `[#..] (1/3)`, `[##.] (2/3)`, `[###] (3/3)`.
  * Proves producer blocks on buffer full, and consumer blocks on buffer empty (zero underflow/overflow).
* **Deadlock Detection & Resource Hierarchy Resolution (`deadlock-demo`):**
  * `hazard` mode: Simulates circular wait (Thread 1: Lock A $\rightarrow$ Lock B; Thread 2: Lock B $\rightarrow$ Lock A). Analyzes all 4 Coffman conditions:
    1. Mutual Exclusion
    2. Hold and Wait
    3. No Preemption
    4. **Circular Wait (ACTIVE)**
    Employs timed recovery to detect the deadlock safely without hanging the shell.
  * `prevent` mode: Enforces strict **Resource Hierarchy / Lock Ordering** (all threads must acquire Lock A before Lock B). Proves **Coffman Condition #4 is broken**, guaranteeing deterministic completion.

#### Verification Commands:
```text
# 1. Unsynchronized Data Race (Missed increments observed)
architect-os:~$ threads-demo 4 100000 nosync

# 2. Synchronized Execution (0 missed increments)
architect-os:~$ threads-demo 4 100000 sync

# 3. Producer-Consumer Bounded Queue
architect-os:~$ pc-demo 8 3

# 4. Deadlock Simulation (Hazard)
architect-os:~$ deadlock-demo hazard

# 5. Deadlock Prevention (Lock Ordering)
architect-os:~$ deadlock-demo prevent
```

---

## ⚡ Quickstart: Build & Execution

### Prerequisites
* GCC (`>= 9.0`) with C11 support
* Make (`>= 4.0`)
* Linux or WSL2 (Ubuntu / Debian)
* Valgrind (for memory verification)

### 1. Build the Binary
```bash
wsl make clean && wsl make
```
*Builds with `-Wall -Wextra -pedantic -std=c11 -pthread -Iinclude -D_GNU_SOURCE` with **0 compiler warnings and 0 errors**.*

### 2. Launch Interactive Shell
```bash
wsl ./architect-os
```
Inside the shell, type `help` to view all commands, or `mode` to switch between `advanced`, `summary`, and `off`.

---

## 🧪 Automated Verification Test Suite

Architect OS includes an automated test runner (`tests/run_tests.sh`) executing 24 comprehensive tests covering all Course Outcomes (CO-1 through CO-6):

```bash
wsl make test
```

### Test Suite Execution Output:
```text
======================================================================
   ARCHITECT OS: COURSE OUTCOMES (CO-1 -> CO-6) AUTOMATED TEST RUNNER   
======================================================================

[TEST 1] Running: CO-1: System Call Introspection & Ring 3->Ring 0 Mode Switch (sysinspect)...
>>> PASS: CO-1: System Call Introspection & Ring 3->Ring 0 Mode Switch (sysinspect)

[TEST 2] Running: CO-2: Process State Introspection (/proc/[pid]/stat & status)...
>>> PASS: CO-2: Process State Introspection (/proc/[pid]/stat & status)

[TEST 3] Running: CO-2: Zombie Process Creation, Detection ('Z'), and Reaping...
>>> PASS: CO-2: Zombie Process Creation, Detection ('Z'), and Reaping

[TEST 4] Running: CO-2: Orphan Process Reparenting to PID 1...
>>> PASS: CO-2: Orphan Process Reparenting to PID 1

[TEST 5] Running: CO-2: Preemptive Round-Robin (RR) Scheduler Simulator...
>>> PASS: CO-2: Preemptive Round-Robin (RR) Scheduler Simulator

[TEST 6] Running: CO-2: Earliest Deadline First (EDF) Scheduler Simulator...
>>> PASS: CO-2: Earliest Deadline First (EDF) Scheduler Simulator

[TEST 7] Running: CO-2: Shortest Remaining Time First (SRTF) Scheduler Simulator...
>>> PASS: CO-2: Shortest Remaining Time First (SRTF) Scheduler Simulator

[TEST 8] Running: CO-3: Multi-Stage Anonymous Pipeline (cmd1 | cmd2 | cmd3)...
>>> PASS: CO-3: Multi-Stage Anonymous Pipeline (cmd1 | cmd2 | cmd3)

[TEST 9] Running: CO-3: Named Pipe (FIFO) Non-blocking Write and Read...
>>> PASS: CO-3: Named Pipe (FIFO) Non-blocking Write and Read

[TEST 10] Running: CO-3: UNIX Domain Sockets IPC Daemon & Client Handshake...
>>> PASS: CO-3: UNIX Domain Sockets IPC Daemon & Client Handshake

[TEST 11] Running: CO-3: POSIX Job Control Table Introspection...
>>> PASS: CO-3: POSIX Job Control Table Introspection

[TEST 12] Running: CO-4: Virtual Memory Maps Inspector (/proc/self/maps)...
>>> PASS: CO-4: Virtual Memory Maps Inspector (/proc/self/maps)

[TEST 13] Running: CO-4: Copy-on-Write (COW) Memory Demonstrator with Page Frame Verification...
>>> PASS: CO-4: Copy-on-Write (COW) Memory Demonstrator with Page Frame Verification

[TEST 14] Running: CO-4: Custom Dynamic Heap Allocator (arch-alloc, arch-free, heapinfo)...
>>> PASS: CO-4: Custom Dynamic Heap Allocator (arch-alloc, arch-free, heapinfo)

[TEST 15] Running: CO-5: Stream Redirection (>, <, >>, 2>, 2>&1)...
>>> PASS: CO-5: Stream Redirection (>, <, >>, 2>, 2>&1)

[TEST 16] Running: CO-5: Linux VFS Inode Telemetry Inspection (inodeinfo)...
>>> PASS: CO-5: Linux VFS Inode Telemetry Inspection (inodeinfo)

[TEST 17] Running: CO-5: Memory-Mapped File I/O Engine (mmapio)...
>>> PASS: CO-5: Memory-Mapped File I/O Engine (mmapio)

[TEST 18] Running: CO-6: Multithreaded Unprotected Shared Counter Data Race (Race Delta Expected)...
>>> PASS: CO-6: Multithreaded Unprotected Shared Counter Data Race (Race Delta Expected)

[TEST 19] Running: CO-6: Multithreaded Synchronized Counter via pthread_mutex_t (0 Race Delta)...
>>> PASS: CO-6: Multithreaded Synchronized Counter via pthread_mutex_t (0 Race Delta)

[TEST 20] Running: CO-6: Thread-Safe Producer-Consumer Bounded Queue (Semaphores & Mutex)...
>>> PASS: CO-6: Thread-Safe Producer-Consumer Bounded Queue (Semaphores & Mutex)

[TEST 21] Running: CO-6: Deadlock Resolution via Lock Ordering (Resource Hierarchy)...
>>> PASS: CO-6: Deadlock Resolution via Lock Ordering (Resource Hierarchy)

[TEST 22] Running: CO-6: Deadlock Detection & Timed Recovery under Circular Wait...
>>> PASS: CO-6: Deadlock Detection & Timed Recovery under Circular Wait

[TEST 23] Running: Visualizer: Mode Summary (Plain-English Command Explanations)...
>>> PASS: Visualizer: Mode Summary (Plain-English Command Explanations)

[TEST 24] Running: Visualizer: Mode Advanced, Summary, and Off Toggling...
>>> PASS: Visualizer: Mode Advanced, Summary, and Off Toggling

======================================================================
ALL 24 / 24 TESTS PASSED SUCCESSFULLY! (100% CO-1 -> CO-6 COMPLIANT)
======================================================================
```

---

## 🛡️ Valgrind Memory Safety Verification

Architect OS guarantees **zero memory leaks and zero invalid reads/writes**:

```bash
wsl make valgrind
```

### Valgrind Verification Report:
```text
======================================================================
   ARCHITECT OS: VALGRIND LEAK CHECK (Targeting 100% Zero Leaks)
======================================================================
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 \
	./architect-os -c "visualize off; arch-alloc 256; heapinfo; scheduler-sim RR 2; pc-demo 6 3; exit"

== HEAP SUMMARY:
==     in use at exit: 0 bytes in 0 blocks
==   total heap usage: 76 allocs, 76 frees, 7,053 bytes allocated
== 
== All heap blocks were freed -- no leaks are possible
== 
== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

---

## 📋 Lab Evaluation / Viva Demonstration Cheat Sheet

Use this quick-reference table during evaluations to quickly demonstrate any outcome on demand:

| Outcome | Command to Run | Key Low-Level Concept to Explain |
| :--- | :--- | :--- |
| **Telemetry** | `mode summary` / `mode advanced` | Explainable systems engine with dual telemetry levels |
| **CO-1** | `sysinspect ls -la` | Ring 3 $\rightarrow$ Ring 0 `syscall` trap, TSS stack switch, `/proc/sys` limits |
| **CO-2** | `procstate` | `/proc/[pid]/stat` PCB fields, voluntary vs nonvoluntary context switches |
| **CO-2** | `zombiedemo` | Defunct PCB in process table, `waitpid()` collection, exit status recovery |
| **CO-2** | `scheduler-sim RR 2` | Preemptive Round-Robin, quantum preemption, Gantt chart, wait time |
| **CO-3** | `echo "hi" \| tr a-z A-Z` | Anonymous pipes, `dup2()`, descriptor closure avoiding pipeline hangs |
| **CO-3** | `fifowrite /tmp/f "msg"` | Persistent FIFO IPC, `mkfifo()`, `open()` with `O_NONBLOCK` |
| **CO-3** | `jobs` / `fg` / `bg` | Shell process groups, `tcsetpgrp()`, `SIGTSTP`, `SIGCONT` |
| **CO-4** | `vmmaps` | ELF memory segments: Text (`r-xp`), Data/BSS, Heap, Stack |
| **CO-4** | `cowdemo` | Copy-on-Write, Page Fault #14, physical PFN divergence via pagemap |
| **CO-4** | `arch-alloc 256` / `heapinfo` | Segregated free-list, 16-byte alignment, contiguous block coalescing |
| **CO-5** | `echo "test" > out.txt` | Stream redirection, differential VFS snapshot, inode allocation delta |
| **CO-5** | `inodeinfo out.txt` | VFS inode attributes: hard links, permissions, disk blocks, device ID |
| **CO-5** | `mmapio out.txt "old" "new"` | Zero-copy `mmap(MAP_SHARED)` file I/O vs buffered `fread`/`fwrite` |
| **CO-6** | `threads-demo 4 100000 nosync` | Interleaved load/increment/store instructions causing missed increments |
| **CO-6** | `threads-demo 4 100000 sync` | `pthread_mutex_lock`/`unlock` ensuring atomic increments (0 race delta) |
| **CO-6** | `pc-demo 8 3` | Bounded buffer queue using counting semaphores (`sem_empty`, `sem_full`) |
| **CO-6** | `deadlock-demo hazard` | Circular Wait deadlock (Coffman conditions 1–4) with timed recovery |
| **CO-6** | `deadlock-demo prevent` | Resource Hierarchy / Lock Ordering breaking Coffman Condition #4 |

---

## 📂 Project File Tree

```
.
├── include/
│   └── architect.h        # Master header, AST structs, Job table, subsystem prototypes
├── src/
│   ├── main.c             # Shell entry point, REPL event loop, history, signal wiring
│   ├── parser.c           # Dynamic vector (Vec), lexer, quote-escaping, AST pipeline parser
│   ├── executor.c         # Pipeline dispatcher, stream redirection (dup2), command visualizer
│   ├── jobs.c             # POSIX Job control manager (jobs, fg, bg), terminal switching
│   ├── memory_vmm.c       # Virtual memory maps (/proc/maps), custom heap allocator, COW demo
│   ├── vfs_inspect.c      # Inode differential telemetry, mmap file I/O, FIFOs, sockets, scheduler
│   └── concurrency.c      # Multithreaded race-condition demo, bounded queue, deadlock resolver
├── tests/
│   └── run_tests.sh       # Comprehensive 24-test automated verification suite
├── Makefile               # Strict C11 compiler targets, clean, test, and valgrind targets
└── README.md              # Complete course outcome mapping, architecture, and verification
```

---

## 📄 License & Attribution
Developed for academic excellence in **25CS2104E: Operating Systems and Systems Programming**. Engineered to adhere strictly to low-level Linux POSIX architecture and ISO C11 systems design standards.
