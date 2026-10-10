# 🏛️ Architect OS: Modular C11 Systems Shell & OS Runtime Emulator

[![C11 Standard](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Compiler Flags](https://img.shields.io/badge/Compiler%20Flags--Wall%20-Wextra%20-pedantic%20-pthread-success.svg)](https://gcc.gnu.org/)
[![Automated Test Suite](https://img.shields.io/badge/Test%20Suite-24%2F24%20Passing%20(100%25)-brightgreen.svg)](#-automated-verification-test-suite)
[![Valgrind](https://img.shields.io/badge/Valgrind-100%25%20Zero%20Leaks-brightgreen.svg)](#-valgrind-memory-safety-verification)
[![OS Target](https://img.shields.io/badge/Kernel-Linux%20%2F%20POSIX%20%2F%20WSL2-orange.svg)](https://ubuntu.com/)
[![Syllabus Alignment](https://img.shields.io/badge/Curriculum-25CS2104E%20(CO--1%20to%20CO--6)-purple.svg)](#-curriculum--course-outcome-co-mapping)

**Architect OS** is a high-performance, modular POSIX interactive systems shell and low-level operating system runtime emulator written in pure C11. 

Unlike traditional production shells (`bash`, `zsh`) that operate as opaque black boxes, Architect OS is engineered as an **Explainable Systems Engine**. It lifts the veil on the Linux kernel boundary by exposing hardware privilege transitions (Ring 3 $\rightarrow$ Ring 0), dynamic system call dispatching, Process Control Block (PCB) state transitions, virtual memory segment allocations, filesystem inode mutations, and concurrent thread synchronization hazards in real time.

---

## 📑 Table of Contents

1. [Project Overview & Philosophy](#-project-overview--philosophy)
2. [System Architecture](#-system-architecture)
3. [How to Run Architect OS](#-how-to-run-architect-os)
   - [Prerequisites](#prerequisites)
   - [Running on Windows via WSL2 (Recommended)](#running-on-windows-via-wsl2-recommended)
   - [Running on Native Linux](#running-on-native-linux)
   - [Invocation Modes & CLI Flags](#invocation-modes--cli-flags)
   - [Interactive Shell Quickstart](#interactive-shell-quickstart)
4. [Dual Execution Telemetry Engine](#-dual-execution-telemetry-engine)
   - [Mode 1: Simple Summary Mode (`mode summary`)](#mode-1-simple-summary-mode-mode-summary)
   - [Mode 2: Advanced 6-Step Visualizer (`mode advanced`)](#mode-2-advanced-6-step-visualizer-mode-advanced)
   - [Mode 3: Silent Execution (`mode off`)](#mode-3-silent-execution-mode-off)
5. [Core Subsystems & Technical Features](#-core-subsystems--technical-features)
   - [1. System Call Introspection & Hardware Traps](#1-system-call-introspection--hardware-traps)
   - [2. Process Control, PCB State Accounting & Scheduling Simulator](#2-process-control-pcb-state-accounting--scheduling-simulator)
   - [3. Inter-Process Communication (IPC), Signals & Job Control](#3-inter-process-communication-ipc-signals--job-control)
   - [4. Virtual Memory Analyzer, Heap Allocator & Copy-on-Write](#4-virtual-memory-analyzer-heap-allocator--copy-on-write)
   - [5. Virtual File System (VFS), Inode Telemetry & Memory-Mapped I/O](#5-virtual-file-system-vfs-inode-telemetry--memory-mapped-io)
   - [6. Multithreading, Synchronization Hazards & Deadlock Resolution](#6-multithreading-synchronization-hazards--deadlock-resolution)
6. [Curriculum & Course Outcome (CO) Mapping](#-curriculum--course-outcome-co-mapping)
7. [Automated Verification Test Suite](#-automated-verification-test-suite)
8. [Valgrind Memory Safety Verification](#-valgrind-memory-safety-verification)
9. [Project File Structure](#-project-file-structure)

---

## 🎯 Project Overview & Philosophy

Modern computer science curricula often present operating systems concepts—such as context switching, virtual page translation, file descriptor tables, and thread contention—as abstract diagrams. When students execute commands in a standard terminal, none of these low-level mechanics are visible.

**Architect OS bridges this conceptual gap.** Every command typed into the shell can be inspected through two pedagogical lenses:
- **Concise Conceptual Mode (`mode summary`):** Translates complex pipelines and utilities into clear, plain-English summaries of what the command achieves and how data flows through system abstractions.
- **Deep Kernel Telemetry Mode (`mode advanced`):** Renders a structured, 6-step ASCII telemetry dashboard detailing the hardware trap, PCB allocation, ELF memory layout, inode delta, and process exit status.

### Key Highlights
- **Zero-Warning Strict Build:** Compiles with `-Wall -Wextra -pedantic -std=c11 -pthread` under GCC and Clang with zero warnings.
- **Zero Memory Leaks:** 100% clean Valgrind profile (`0 errors from 0 contexts, 0 bytes leaked`).
- **Complete Self-Contained Built-ins:** Includes custom implementations of dynamic heap allocators, CPU scheduling algorithms, IPC daemons, COW demonstrators, and deadlock solvers without external high-level libraries.

---

## 🏗️ System Architecture

Architect OS is decoupled into modular, high-cohesion user-space subsystems following professional Linux systems programming patterns:

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
    |         Kernel Telemetry Engine             |       |              Execution Dispatcher             |
    |  * Mode 1: Plain-English Command Summary    |       |  * Built-in Subsystems Engine                 |
    |  * Mode 2: Advanced 6-Step Visualizer:      | <---> |  * Multi-Stage Anonymous Pipelines (pipe/dup2)|
    |    - [Step 1] AST Blueprint                 |       |  * Stream Redirection (<, >, >>, 2>, 2>&1)    |
    |    - [Step 2] Ring 3 -> Ring 0 Hardware Trap|       |  * POSIX Job Control Subsystem (jobs, fg, bg) |
    |    - [Step 3] PCB Lifecycle Transitions     |       |  * Foreground/Background Process Group Switch |
    |    - [Step 4] Virtual Memory Address Space  |       +-----------------------------------------------+
    |    - [Step 5] Inode & VFS Delta Telemetry   |
    |    - [Step 6] IPC Plumbing & Exit Accounting|
    +---------------------------------------------+
```

---

## 🚀 How to Run Architect OS

Architect OS is a native Linux application utilizing low-level POSIX kernel interfaces (`fork`, `execve`, `mmap`, `pipe`, `ptrace`, `/proc`). On Windows systems, it runs seamlessly via **Windows Subsystem for Linux (WSL / WSL2)**.

### Prerequisites

| Component | Minimum Requirement | Notes |
| :--- | :--- | :--- |
| **Operating System** | Linux (Ubuntu 20.04+, Debian 11+, Arch) or Windows 10/11 with WSL2 | Requires POSIX kernel APIs |
| **Compiler** | GCC $\ge$ 9.0 or Clang $\ge$ 10.0 | ISO C11 standard support (`-std=c11`) |
| **Build System** | GNU Make $\ge$ 4.0 | Included in `build-essential` |
| **Threading** | POSIX Threads (`libpthread`) | Included in GNU libc |
| **Debugger & Profiler** | Valgrind (Optional) | For memory leak verification |

To install dependencies on Debian/Ubuntu/WSL:
```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc make valgrind
```

---

### Running on Windows via WSL2 (Recommended)

If you are using Windows, you can compile and launch Architect OS directly through WSL:

#### Step 1: Open Terminal
Open **PowerShell**, **Windows Terminal**, or **Command Prompt**.

#### Step 2: Navigate to Project Directory
```powershell
# Change into the project directory
cd "c:\Users\sukha\OneDrive\Desktop\ossp project"
```

#### Step 3: Compile the Project
```powershell
wsl make clean && wsl make
```
*Output: All object files compile with zero warnings, producing the `architect-os` executable.*

#### Step 4: Launch Architect OS Interactive Shell
```powershell
wsl ./architect-os
```
*Or, switch directly into the WSL Linux shell first:*
```powershell
wsl
cd "/mnt/c/Users/sukha/OneDrive/Desktop/ossp project"
./architect-os
```

You will be greeted by the interactive shell prompt:
```text
======================================================================
     ARCHITECT OS: MODULAR C11 SYSTEMS SHELL & RUNTIME EMULATOR       
======================================================================
 * Academic Standard : 25CS2104E (CO-1 through CO-6)
 * Telemetry Engine  : ACTIVE (Mode: ADVANCED 6-Step Visualizer)
 * Type 'help' for built-in systems commands, or 'mode' to toggle.
----------------------------------------------------------------------
architect-os:~$ 
```

---

### Running on Native Linux

On any Linux distribution (Ubuntu, Debian, Fedora, Arch):

```bash
# 1. Clone or navigate to the project directory
cd "/path/to/ossp project"

# 2. Build the project
make clean && make

# 3. Launch the shell
./architect-os
```

---

### Invocation Modes & CLI Flags

Architect OS supports multiple execution modes from the command line:

```bash
# 1. Interactive Shell REPL (Default)
./architect-os

# 2. Batch Execution Flag (-c)
# Executes a single command string or pipeline and exits cleanly
./architect-os -c "ls -la"
./architect-os -c "mode summary; echo 'Hello systems'; exit"
./architect-os -c "scheduler-sim RR 2"

# 3. Dedicated IPC Socket Daemon Mode (--daemon)
# Launches a background UNIX domain socket server on /tmp/architect_os.sock
./architect-os --daemon

# 4. Dedicated IPC Socket Client Mode (--client)
# Connects to the active UNIX domain socket server, sends a ping, and receives a pong
./architect-os --client

# 5. Automated Verification Test Suite
make test

# 6. Full Valgrind Memory Verification
make valgrind
```

---

### Interactive Shell Quickstart

Once inside `architect-os:~$`:

1. **Toggle Telemetry Detail:**
   - `mode summary` — Switch to concise plain-English explanations.
   - `mode advanced` — Switch to low-level 6-step kernel telemetry dashboard.
   - `mode off` — Silent mode for standard shell execution.
2. **Execute Any Standard Linux Command:**
   - `ls -la`, `pwd`, `cat Makefile | grep CC`, `whoami`
3. **Inspect Subsystem Built-ins:**
   - `sysinspect ls` — Trace system call dispatching and Ring 3 $\rightarrow$ 0 trap.
   - `procstate` — Read Process Control Block stats from `/proc/self/stat`.
   - `scheduler-sim RR 2` — Simulate CPU round-robin scheduling with Gantt charts.
   - `cowdemo` — Observe Copy-on-Write physical page divergence.
   - `threads-demo 4 100000 nosync` — Trigger an uncoordinated CPU data race.
   - `deadlock-demo hazard` — Simulate and safely detect a circular wait condition.
4. **Exit the Shell:**
   - Type `exit` or press `Ctrl+D`.

---

## 🌟 Dual Execution Telemetry Engine

Architect OS features a runtime-switchable telemetry engine. Users can inspect the mechanics of any command at their chosen level of detail:

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

Designed for intuitive, high-level understanding. When any command is executed, Architect OS analyzes the command AST and outputs a concise summary explaining its real-world function, arguments, and data flow.

#### Example A: Directory Listing (`ls -la`)
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
total 468
drwxrwxrwx 1 user user   4096 Oct 10 14:15 .
drwxrwxrwx 1 user user   4096 Oct 10 13:40 ..
-rwxrwxrwx 1 user user 231552 Oct 10 14:15 architect-os
-rw-rw-rw- 1 user user   2541 Oct 10 14:10 Makefile
...
```

#### Example B: Multi-Stage Pipeline (`cat README.md | grep Architecture`)
```text
architect-os:~$ cat README.md | grep Architecture
+-----------------------------------------------------------------------------------------+
|  [Command Summary Mode]                                                                 |
+-----------------------------------------------------------------------------------------+
  Pipeline    : cat README.md | grep Architecture
  Description : Chains 2 commands via kernel anonymous pipes (zero temp files).
  Data Flow   : [cat] ---> (kernel pipe) ---> [grep]
+-----------------------------------------------------------------------------------------+
## 🏗️ System Architecture
```

#### Example C: Built-in Subsystem Command (`sysinspect ls`)
```text
architect-os:~$ sysinspect ls
+-----------------------------------------------------------------------------------------+
|  [Command Summary Mode]                                                                 |
+-----------------------------------------------------------------------------------------+
  Command     : sysinspect ls
  Description : Inspects hardware Ring 3->0 trap, intercepts syscalls, and reads /proc/sys limits.
  Arguments   : ls 
+-----------------------------------------------------------------------------------------+
```

---

### Mode 2: Advanced 6-Step Visualizer (`mode advanced`)

Designed for deep systems programming inspection and architecture walkthroughs. Every command renders a complete 6-stage telemetry dashboard:

```text
architect-os:~$ cat < input.txt | grep kernel > output.txt
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

Disables telemetry banners completely, returning Architect OS to a silent, production-style POSIX shell.

---

## ⚙️ Core Subsystems & Technical Features

Architect OS is built on six decoupled systems engineering subsystems:

### 1. System Call Introspection & Hardware Traps

**File:** `src/main.c`, `src/executor.c` | **Command:** `sysinspect <command>`

- **Hardware Privilege Boundary Introspection:** Analyzes the transition between User Mode (Current Privilege Level `CPL=3`) and Kernel Mode (`CPL=0`) invoked by the x86_64 `syscall` instruction (opcode `0x0F 0x05`). Demonstrates how `%rip` is saved to `%rcx`, `%rflags` to `%r11`, and the stack pointer `%rsp` is switched to the kernel interrupt stack via the Task State Segment (TSS).
- **Dynamic Syscall Interception:** Uses POSIX `ptrace(PTRACE_SYSCALL)` to intercept and display the sequence of system calls (`sys_clone`, `sys_execve`, `sys_brk`, `sys_mmap`, `sys_openat`, `sys_newfstatat`, `sys_write`, `sys_close`, `sys_exit_group`) dispatched to the kernel's `sys_call_table`.
- **Kernel Parameter Inspection:** Directly queries `/proc/sys` OS service limits:
  - `/proc/sys/kernel/pid_max` (process table capacity)
  - `/proc/sys/fs/file-max` (system-wide VFS file descriptor limit)
  - `/proc/sys/fs/pipe-max-size` (IPC buffer limits)
  - `/proc/sys/vm/swappiness` (virtual memory swapping ratio)

```text
architect-os:~$ sysinspect ls
=== [CO-1: System Call Introspection & Mode Switch Telemetry] ===
Command: 'ls'
1. Hardware Mode Switch Mechanics:
   - User Ring 3 (CPL=3) -> Kernel Ring 0 (CPL=0) via 'syscall' instruction
   - Hardware register preservation: %rip -> %rcx, %rflags -> %r11
   - TSS switches %rsp to kernel interrupt stack
2. System Call Sequence Intercepted via ptrace:
   [1] sys_execve("/bin/ls", ["ls"], [env]) -> return 0
   [2] sys_brk(NULL)                        -> return 0x55d28b000
   [3] sys_mmap(NULL, 8192, PROT_READ|WRITE)-> return 0x7f920c000
   [4] sys_openat(AT_FDCWD, ".", O_RDONLY)   -> fd 3
   ...
```

---

### 2. Process Control, PCB State Accounting & Scheduling Simulator

**File:** `src/executor.c`, `src/vfs_inspect.c` | **Commands:** `procstate`, `zombiedemo`, `orphandemo`, `scheduler-sim`

- **Process Control & Synchronization:** Implements safe `fork()`, address space replacement via `execvp()`, and parent synchronization using `waitpid()`. Analyzes termination states using macros: `WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`, `WTERMSIG`, `WIFSTOPPED`.
- **Live PCB State Inspection (`procstate [pid]`):** Reads `/proc/[pid]/stat` and `/proc/[pid]/status` to report Process Control Block state (`R`: Running, `S`: Interruptible Sleep, `D`: Disk Sleep, `Z`: Zombie, `T`: Stopped), virtual memory (`VmSize`, `VmRSS`), thread count, voluntary context switches (I/O blocks), and non-voluntary context switches (time-slice expirations).
- **Zombie Process Lifecycle (`zombiedemo`):** Demonstrates child termination while parent delays `waitpid()`. Shows the child enter state `'Z'` (defunct PCB retaining exit code in process table), followed by parent reaping and table eviction.
- **Orphan Process Adoption (`orphandemo`):** Demonstrates child parent termination, proving immediate reparenting to PID 1 (`init` / `systemd`).
- **Tick-by-Tick CPU Scheduling Simulator (`scheduler-sim`):** Simulates CPU scheduling algorithms with ASCII Gantt charts, per-task turnaround times, and average waiting times:
  - **Round-Robin (RR):** Preemptive scheduling with configurable time-slice quantum.
  - **Earliest Deadline First (EDF):** Dynamic priority scheduling based on shortest deadline.
  - **Shortest Remaining Time First (SRTF):** Preemptive shortest-job scheduler.

```text
architect-os:~$ scheduler-sim RR 2
=== [CO-2: CPU Scheduler Simulator: Round-Robin (Quantum: 2)] ===
Simulating 3 Tasks:
  * P1: Burst=5, Priority=1
  * P2: Burst=3, Priority=2
  * P3: Burst=4, Priority=3

--- Execution Timeline (Gantt Chart) ---
[0-2]: P1 | [2-4]: P2 | [4-6]: P3 | [6-8]: P1 | [8-9]: P2 (DONE) | [9-11]: P3 (DONE) | [11-12]: P1 (DONE)

--- Performance Metrics ---
Task P1: Finish=12, Turnaround=12, Waiting=7
Task P2: Finish=9,  Turnaround=9,  Waiting=6
Task P3: Finish=11, Turnaround=11, Waiting=7
Total Context Switches: 6
Average Turnaround Time: 10.67 units
Average Waiting Time   : 6.67 units
```

---

### 3. Inter-Process Communication (IPC), Signals & Job Control

**File:** `src/executor.c`, `src/jobs.c`, `src/vfs_inspect.c` | **Commands:** `fifowrite`, `fiforead`, `socket-daemon`, `socket-client`, `jobs`, `fg`, `bg`

- **Multi-Stage Anonymous Pipelines:** Chains arbitrary commands (`cmd1 | cmd2 | cmd3 | ...`) using `pipe()` and `dup2()`. Properly closes all intermediate file descriptor read/write ends in parent and children to prevent descriptor leaks and pipeline deadlocks.
- **Named Pipes (FIFOs):** Persistent filesystem IPC using `mkfifo()`. Demonstrates non-blocking I/O with `open(..., O_NONBLOCK)` via `fifowrite` and `fiforead`.
- **UNIX Domain Sockets:** Standalone client-server architecture communicating over `/tmp/architect_os.sock` via `socket(AF_UNIX, SOCK_STREAM, 0)`.
- **POSIX Signal Architecture:**
  - Registers signal handlers using POSIX `sigaction()`.
  - Protects the shell from `SIGINT` (Ctrl+C) and `SIGTSTP` (Ctrl+Z) while forwarding signals to the active foreground process group.
  - Asynchronous background child reaper via `SIGCHLD` using non-blocking `waitpid(-1, &status, WNOHANG)` to reap finished background tasks without shell latency.
- **Job Control Subsystem:**
  - Maintains an active `Job` table tracking Job ID, PGID, PID, State (`RUNNING`, `STOPPED`, `DONE`), and command line.
  - Supports `jobs` (list active jobs), `fg <job_id>` (bring job to foreground with `tcsetpgrp()` and `SIGCONT`), and `bg <job_id>` (resume stopped job in background).

```text
architect-os:~$ sleep 60 &
[1] Spawned background job PID: 4091

architect-os:~$ jobs
=== Active POSIX Job Table ===
[1]  PID: 4091  PGID: 4091  State: RUNNING    Command: 'sleep 60'

architect-os:~$ fg 1
Brought Job [1] (PID 4091) to foreground.
^Z
[1]+ Stopped                 sleep 60

architect-os:~$ bg 1
Job [1] resumed in background.
```

---

### 4. Virtual Memory Analyzer, Heap Allocator & Copy-on-Write

**File:** `src/memory_vmm.c` | **Commands:** `vmmaps`, `cowdemo`, `arch-alloc`, `arch-free`, `heapinfo`

- **Custom Explicit Free-List Heap Allocator:**
  - Independent 2 MB memory arena managed via `mmap(MAP_PRIVATE | MAP_ANONYMOUS)`.
  - Implements `arch_malloc(size_t)` and `arch_free(void *)` featuring 16-byte alignment, 32-byte chunk metadata headers with magic canary verification (`0xAC1984C0`), block splitting on oversized allocations, and bidirectional contiguous block coalescing to eliminate external fragmentation.
  - Telemetry command `heapinfo` reports allocated chunks, free chunks, pool utilization percentage, and fragmentation index.
- **Virtual Memory Address Space Inspector (`vmmaps [pid]`):**
  - Parses `/proc/[pid]/maps` to categorize memory segments:
    - **Text Segment (`r-xp`)**: Executable code from the ELF binary.
    - **Data / BSS (`rw-p`)**: Initialized globals and uninitialized zero-fill pages.
    - **Heap (`[heap]`)**: Dynamic heap break managed via `brk`/`sbrk`.
    - **Memory Mapping Segment**: Dynamic shared libraries (`libc.so`), anonymous `mmap` regions, `[vdso]`.
    - **User Stack (`[stack]`)**: Thread execution frames, local variables, return pointers.
- **Copy-on-Write (COW) Demonstrator (`cowdemo`):**
  - Allocates a page-aligned buffer, populates it, and inspects physical Page Frame Numbers (PFNs) via `/proc/self/pagemap`.
  - Forks a child: both parent and child initially share the exact same physical RAM frame (marked read-only in page table).
  - Child writes to the buffer: the hardware MMU raises **Page Fault #14**, prompting the kernel to allocate a new physical frame, duplicate data, update PTE permissions, and flush the TLB. Demonstrates physical PFN divergence while virtual addresses remain identical.

```text
architect-os:~$ cowdemo
=== [CO-4: Copy-on-Write (COW) Physical Page Frame Demonstration] ===
Allocated page-aligned buffer at virtual address: 0x7fa2b000
Populating buffer with initial data...
Inspecting physical Page Frame Number (PFN) via /proc/self/pagemap...
Parent: Virtual Address: 0x7fa2b000 -> Physical PFN: 0x140e2a

[Phase 1: Forking child process...]
Child:  Virtual Address: 0x7fa2b000 -> Physical PFN: 0x140e2a
>>> Parent & Child share IDENTICAL physical RAM frame (COW active, 0 bytes duplicated)

[Phase 2: Child modifying buffer...]
Child writing new data to buffer...
>>> Hardware MMU triggered Page Fault #14! Kernel allocated new physical frame.
Child:  Virtual Address: 0x7fa2b000 -> Physical PFN: 0x19bb4f
Parent: Virtual Address: 0x7fa2b000 -> Physical PFN: 0x140e2a
>>> Physical frames DIVERGED! Virtual addresses remain identical.
```

---

### 5. Virtual File System (VFS), Inode Telemetry & Memory-Mapped I/O

**File:** `src/vfs_inspect.c` | **Commands:** `inodeinfo`, `mmapio`, Stream Redirection (`<`, `>`, `>>`, `2>`, `2>&1`)

- **POSIX Stream Redirection Engine:**
  - `<` : Standard input redirection (`O_RDONLY`).
  - `>` : Standard output truncation redirection (`O_WRONLY | O_CREAT | O_TRUNC`).
  - `>>`: Standard output append redirection (`O_WRONLY | O_CREAT | O_APPEND`).
  - `2>`: Standard error redirection to file.
  - `2>&1`: Standard error duplication to standard output descriptor.
- **Differential VFS Inode Telemetry:** Automatically records pre-execution snapshots (`vfs_take_snapshot`) and compares them against post-execution state (`vfs_calculate_delta`), computing inode creation, byte mutations, and disk block allocations.
- **VFS Inode Inspector (`inodeinfo <path>`):** Queries `lstat()` to display:
  - Inode number and Device ID (Major/Minor numbers).
  - Hard link count (`st_nlink`).
  - Permission bits (`st_mode` in octal and rwx format).
  - UID / GID ownership.
  - File byte size, optimal block transfer size (`st_blksize`), and 512-byte disk blocks allocated (`st_blocks`).
  - Access (`atime`), modification (`mtime`), and status change (`ctime`) timestamps.
- **Memory-Mapped File I/O Engine (`mmapio <file> [find] [replace]`):**
  - Maps files directly into process address space via `mmap(..., PROT_READ | PROT_WRITE, MAP_SHARED, ...)`.
  - Performs zero-copy in-place string replacements via direct pointer manipulation.
  - Commits dirty pages back to storage using `msync(MS_SYNC)`.
  - Benchmarks nanosecond/microsecond zero-copy latency against standard buffered `fread`/`fwrite`.

```text
architect-os:~$ inodeinfo Makefile
=== [CO-5: Linux Virtual File System (VFS) Inode Telemetry] ===
Path: Makefile
  * Inode Number : 844424930132332
  * Device ID    : Major: 0, Minor: 50
  * File Mode    : -rw-rw-rw- (Octal: 0666)
  * Links        : 1
  * UID / GID    : 1000 / 1000
  * Size (Bytes) : 2541 bytes (5 physical 512-byte blocks allocated)
  * Optimal I/O  : 4096 bytes per block
  * Last Modify  : Sat Oct 10 14:10:22 2026
```

---

### 6. Multithreading, Synchronization Hazards & Deadlock Resolution

**File:** `src/concurrency.c` | **Commands:** `threads-demo`, `pc-demo`, `deadlock-demo`

- **Multithreaded Data Race vs Mutex Synchronization (`threads-demo`):**
  - Spawns worker threads via `pthread_create()` and synchronizes via `pthread_join()`.
  - `nosync` mode: Unprotected concurrent execution of `counter++` causes read-modify-write CPU instruction interleaving. Displays missed increments and percentage data loss.
  - `sync` mode: Protects the critical section with `pthread_mutex_t` (`lock`/`unlock`). Proves **Race Delta = 0 (100% thread safe)**.
- **Producer-Consumer Bounded Queue (`pc-demo <items> <capacity>`):**
  - Thread-safe circular bounded buffer coordinated with POSIX counting semaphores (`sem_empty`, `sem_full`) and a mutex lock.
  - Visualizes real-time buffer occupancy bars: `[#..] (1/3)`, `[##.] (2/3)`, `[###] (3/3)`.
  - Guarantees zero buffer overflow and zero underflow.
- **Deadlock Detection & Resource Hierarchy Resolution (`deadlock-demo`):**
  - `hazard` mode: Simulates circular wait (Thread 1: Lock A $\rightarrow$ Lock B; Thread 2: Lock B $\rightarrow$ Lock A). Analyzes all 4 Coffman conditions:
    1. Mutual Exclusion
    2. Hold and Wait
    3. No Preemption
    4. **Circular Wait (ACTIVE)**
    Employs a timed deadlock detection recovery mechanism to avoid freezing the shell.
  - `prevent` mode: Enforces strict **Resource Hierarchy / Lock Ordering** (all threads acquire Lock A before Lock B). Proves **Coffman Condition #4 is broken**, guaranteeing deterministic completion.

```text
architect-os:~$ threads-demo 4 100000 nosync
=== [CO-6: Multithreading Data Race Demonstration (Unsynchronized)] ===
Spawning 4 worker threads, each incrementing shared counter 100000 times...
Expected Final Counter : 400000
Actual Observed Counter: 247812
Race Delta (Lost Ops)  : 152188 (38.05% data loss due to concurrent read-modify-write!)

architect-os:~$ threads-demo 4 100000 sync
=== [CO-6: Multithreading Mutex Synchronization (Thread-Safe)] ===
Spawning 4 worker threads with pthread_mutex_t synchronization...
Expected Final Counter : 400000
Actual Observed Counter: 400000
Race Delta (Lost Ops)  : 0 (100% thread safe, zero missed increments)
```

---

## 🎓 Curriculum & Course Outcome (CO) Mapping

Architect OS is designed to systematically fulfill all Course Outcomes for **25CS2104E: Operating Systems and Systems Programming**:

| Course Outcome | Academic Competency Description | Implemented Subsystems & Source Files | Shell Commands & Verification Scenarios |
| :--- | :--- | :--- | :--- |
| **CO-1** | **The OS as a Service Layer & Syscall Introspection**<br>Understand operating system architecture, hardware privilege boundaries, system call interfaces, and kernel abstraction parameters. | `src/main.c`<br>`src/executor.c`<br>`include/architect.h` | `sysinspect <command>`<br>`mode summary`<br>`mode advanced`<br>`history`<br>`visualize on\|off` |
| **CO-2** | **Process Control & User-Level Process Lifecycle**<br>Master process creation, address space transformation, parent-child synchronization, process states, and CPU scheduling. | `src/executor.c`<br>`src/vfs_inspect.c`<br>`include/architect.h` | `procstate [pid]`<br>`zombiedemo`<br>`orphandemo`<br>`scheduler-sim RR <quantum>`<br>`scheduler-sim EDF`<br>`scheduler-sim SRTF` |
| **CO-3** | **Inter-Process Communication (IPC), Signals & Job Control**<br>Design pipeline topologies, FIFO streams, socket communication, signal handlers, and shell job control tables. | `src/executor.c`<br>`src/jobs.c`<br>`src/vfs_inspect.c` | Anonymous Pipelines (`cmd1 \| cmd2 \| cmd3`)<br>`fifowrite <path> <msg>`<br>`fiforead <path>`<br>`socket-daemon` / `socket-client`<br>`jobs`<br>`fg <job_id>`<br>`bg <job_id>` |
| **CO-4** | **Virtual Memory Management & Memory Introspection**<br>Understand address space segmentation, dynamic heap allocators, page translation, and Copy-on-Write (COW) mechanics. | `src/memory_vmm.c`<br>`include/architect.h` | `vmmaps [pid]`<br>`cowdemo`<br>`arch-alloc <bytes>`<br>`arch-free <ptr>`<br>`heapinfo` |
| **CO-5** | **Linux Virtual File System (VFS), Inodes & Redirection**<br>Master I/O stream redirection, inode attributes, filesystem metadata tracking, and memory-mapped file I/O. | `src/vfs_inspect.c`<br>`src/executor.c` | Stream Redirection (`<`, `>`, `>>`, `2>`, `2>&1`)<br>`inodeinfo <path>`<br>`mmapio <file> [find] [replace]` |
| **CO-6** | **Concurrency, POSIX Threads & Synchronization Hazards**<br>Analyze race conditions, implement mutexes and semaphores, design bounded buffers, and prevent deadlocks via lock hierarchy. | `src/concurrency.c`<br>`include/architect.h` | `threads-demo <threads> <iter> nosync`<br>`threads-demo <threads> <iter> sync`<br>`pc-demo <items> <capacity>`<br>`deadlock-demo hazard`<br>`deadlock-demo prevent` |

---

## 🧪 Automated Verification Test Suite

Architect OS includes an automated test runner (`tests/run_tests.sh`) that validates all 24 technical requirements across CO-1 through CO-6:

```bash
wsl make test
# or on native Linux:
make test
```

### Test Suite Execution Summary

```text
======================================================================
   ARCHITECT OS: COURSE OUTCOMES (CO-1 -> CO-6) AUTOMATED TEST RUNNER   
======================================================================

[TEST 1] Running: CO-1: System Call Introspection & Mode Switch (sysinspect)...   >>> PASS
[TEST 2] Running: CO-2: Process State Introspection (/proc/[pid]/stat)...        >>> PASS
[TEST 3] Running: CO-2: Zombie Process Creation, Detection ('Z'), and Reaping... >>> PASS
[TEST 4] Running: CO-2: Orphan Process Reparenting to PID 1...                    >>> PASS
[TEST 5] Running: CO-2: Preemptive Round-Robin (RR) Scheduler Simulator...        >>> PASS
[TEST 6] Running: CO-2: Earliest Deadline First (EDF) Scheduler Simulator...      >>> PASS
[TEST 7] Running: CO-2: Shortest Remaining Time First (SRTF) Simulator...         >>> PASS
[TEST 8] Running: CO-3: Multi-Stage Anonymous Pipeline (cmd1 | cmd2 | cmd3)...    >>> PASS
[TEST 9] Running: CO-3: Named Pipe (FIFO) Non-blocking Write and Read...          >>> PASS
[TEST 10] Running: CO-3: UNIX Domain Sockets IPC Daemon & Client Handshake...     >>> PASS
[TEST 11] Running: CO-3: POSIX Job Control Table Introspection...                 >>> PASS
[TEST 12] Running: CO-4: Virtual Memory Maps Inspector (/proc/self/maps)...       >>> PASS
[TEST 13] Running: CO-4: Copy-on-Write Memory Demonstrator (Page Frames)...      >>> PASS
[TEST 14] Running: CO-4: Custom Dynamic Heap Allocator (alloc/free/heapinfo)...   >>> PASS
[TEST 15] Running: CO-5: Stream Redirection (>, <, >>, 2>, 2>&1)...               >>> PASS
[TEST 16] Running: CO-5: Linux VFS Inode Telemetry Inspection (inodeinfo)...      >>> PASS
[TEST 17] Running: CO-5: Memory-Mapped File I/O Engine (mmapio)...                >>> PASS
[TEST 18] Running: CO-6: Multithreaded Unprotected Shared Counter Data Race...     >>> PASS
[TEST 19] Running: CO-6: Multithreaded Synchronized Counter via pthread_mutex_t.. >>> PASS
[TEST 20] Running: CO-6: Thread-Safe Producer-Consumer Bounded Queue...           >>> PASS
[TEST 21] Running: CO-6: Deadlock Resolution via Lock Ordering Hierarchy...       >>> PASS
[TEST 22] Running: CO-6: Deadlock Detection & Timed Recovery (Circular Wait)...   >>> PASS
[TEST 23] Running: Visualizer: Mode Summary (Plain-English Explanations)...       >>> PASS
[TEST 24] Running: Visualizer: Mode Advanced, Summary, and Off Toggling...        >>> PASS

======================================================================
ALL 24 / 24 TESTS PASSED SUCCESSFULLY! (100% VERIFIED)
======================================================================
```

---

## 🛡️ Valgrind Memory Safety Verification

Architect OS adheres to the highest systems engineering memory safety standards. Every allocated AST node, token vector buffer, and file descriptor is cleanly freed:

```bash
wsl make valgrind
# or on native Linux:
make valgrind
```

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

## 📁 Project File Structure

```
ossp-project/
├── include/
│   └── architect.h        # Master header: AST definitions, Job structs, subsystem prototypes
├── src/
│   ├── main.c             # Shell entry point, interactive REPL loop, history, signal wiring
│   ├── parser.c           # Dynamic vector (Vec), tokenizer, quote handler, pipeline AST generator
│   ├── executor.c         # Pipeline dispatcher, stream redirection (dup2), command visualizer
│   ├── jobs.c             # POSIX Job control manager (jobs, fg, bg), terminal process groups
│   ├── memory_vmm.c       # Virtual memory maps (/proc/maps), custom heap allocator, COW demo
│   ├── vfs_inspect.c      # Inode differential telemetry, mmap file I/O, FIFOs, sockets, scheduler
│   └── concurrency.c      # Multithreaded race-condition demo, bounded queue, deadlock resolver
├── tests/
│   └── run_tests.sh       # Comprehensive 24-test automated verification suite
├── Makefile               # Strict C11 compiler targets, clean, test, and valgrind targets
├── .gitignore             # Ignores tests/, object files, build artifacts, and binaries
└── README.md              # Complete project architecture, feature deep dive, and CO mapping
```

---

## 📜 License & Academic Integrity

This project is authored as a complete reference implementation and educational systems engine for **25CS2104E: Operating Systems and Systems Programming**. Engineered strictly according to ISO C11 and POSIX standards.
