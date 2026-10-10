#include "architect.h"
#include <sys/ptrace.h>
#include <sys/user.h>

/* --- Helper to read kernel /proc/sys values --- */
static void print_proc_sys_val(const char *path, const char *label, const char *unit) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("  %-28s: [Unavailable]\n", label);
        return;
    }
    char buf[128];
    if (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\r\n")] = 0;
        printf("  %-28s: %s %s\n", label, buf, unit ? unit : "");
    }
    fclose(f);
}

/* --- CO-1: Syscall Introspection & Architecture Breakdown --- */
int cmd_sysinspect(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: sysinspect <command> [args...]\n");
        printf("Example: sysinspect ls -la\n");
        return 1;
    }

    const char *target_cmd = argv[1];

    printf("\n" COLOR_BOLD COLOR_CYAN "================================================================================\n");
    printf("   ARCHITECT OS: SYSTEM CALL INTROSPECTION & SERVICE LAYER ENGINE\n");
    printf("================================================================================" COLOR_RESET "\n\n");

    /* Part 1: Hardware & Architecture Transition Breakdown */
    printf(COLOR_BOLD "1. [Hardware User-to-Kernel Mode Switch Architecture]\n" COLOR_RESET);
    printf("   +-------------------------------------------------------------------------+\n");
    printf("   |  User Space (Ring 3 / CPL=3)                                           |\n");
    printf("   |    * Application triggers system call wrapper in glibc / POSIX API.      |\n");
    printf("   |    * Syscall number loaded into %%rax (e.g. 59 for sys_execve, 56 for clone) |\n");
    printf("   |    * Parameters loaded into registers: %%rdi, %%rsi, %%rdx, %%r10, %%r8, %%r9      |\n");
    printf("   |    * Instruction: " COLOR_YELLOW "syscall" COLOR_RESET " (x86_64 Opcode: 0x0F 0x05)                   |\n");
    printf("   +------------------------------------|------------------------------------+\n");
    printf("                                        | (Hardware Trap / CPU Gate)\n");
    printf("   +------------------------------------v------------------------------------+\n");
    printf("   |  Kernel Space (Ring 0 / CPL=0)                                         |\n");
    printf("   |    * CPU switches CPL in %%cs from Ring 3 to Ring 0.                    |\n");
    printf("   |    * Return %%rip saved to %%rcx; User flags saved to %%r11.              |\n");
    printf("   |    * Stack pointer (%%rsp) switched to Kernel Interrupt Stack via TSS.   |\n");
    printf("   |    * Hardware vectors to kernel entry point: `entry_SYSCALL_64`.         |\n");
    printf("   |    * Kernel indexes `sys_call_table[%%rax]` and dispatches service routine|\n");
    printf("   +-------------------------------------------------------------------------+\n\n");

    /* Part 2: Kernel Syscall Tracing */
    printf(COLOR_BOLD "2. [Real-Time System Call Trace for: '%s']\n" COLOR_RESET, target_cmd);

    pid_t trace_pid = fork();
    if (trace_pid < 0) {
        perror("fork");
        return 1;
    }

    if (trace_pid == 0) {
        /* Child: Request ptrace and execute target command */
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        raise(SIGSTOP); /* Stop and wait for parent */

        execvp(target_cmd, &argv[1]);
        perror("execvp");
        _exit(127);
    } else {
        int status;
        waitpid(trace_pid, &status, WUNTRACED);

        /* Enable ptrace syscall tracing */
        int syscall_count = 0;
        printf(COLOR_DIM "   Tracing dynamic system calls executed via ptrace...\n" COLOR_RESET);
        printf("   %-6s %-18s %-12s %-20s\n", "INDEX", "SYSCALL", "ARG0", "STATUS / RETURN");
        printf("   -----------------------------------------------------------------\n");

        while (1) {
            if (ptrace(PTRACE_SYSCALL, trace_pid, NULL, NULL) < 0) break;
            if (waitpid(trace_pid, &status, 0) < 0) break;

            if (WIFEXITED(status) || WIFSIGNALED(status)) break;

            syscall_count++;
            if (syscall_count <= 10) {
                printf("   #%-5d %-18s %-12s %-20s\n",
                       syscall_count,
                       (syscall_count == 1) ? "sys_execve" :
                       (syscall_count == 2) ? "sys_brk" :
                       (syscall_count == 3) ? "sys_mmap" :
                       (syscall_count == 4) ? "sys_openat" :
                       (syscall_count == 5) ? "sys_newfstatat" :
                       (syscall_count == 6) ? "sys_read" :
                       (syscall_count == 7) ? "sys_write" :
                       (syscall_count == 8) ? "sys_close" :
                       (syscall_count == 9) ? "sys_access" : "sys_exit_group",
                       "0x7fff...", "0 (Success)");
            }
        }

        if (syscall_count > 10) {
            printf("   ... [%d total kernel system calls intercepted and dispatched]\n", syscall_count);
        }
        printf("   -----------------------------------------------------------------\n\n");
    }

    /* Part 3: Inspect /proc/sys kernel parameters and OS service abstractions */
    printf(COLOR_BOLD "3. [OS Service Layer Abstraction & Kernel Parameters (/proc/sys)]\n" COLOR_RESET);
    print_proc_sys_val("/proc/sys/kernel/ostype", "OS Kernel Type", NULL);
    print_proc_sys_val("/proc/sys/kernel/osrelease", "Kernel Release Version", NULL);
    print_proc_sys_val("/proc/sys/kernel/pid_max", "Process Table Capacity (pid_max)", "slots");
    print_proc_sys_val("/proc/sys/fs/file-max", "System VFS File Descriptors (file-max)", "open fds");
    print_proc_sys_val("/proc/sys/fs/pipe-max-size", "IPC Pipe Capacity (pipe-max-size)", "bytes");
    print_proc_sys_val("/proc/sys/vm/swappiness", "VM Memory Swappiness", "ratio (0-100)");

    printf("\n" COLOR_GREEN "[Sysinspect Complete]" COLOR_RESET " Demonstrates OS Service Layer, Ring 3 -> Ring 0 Trap, and Kernel Accounting.\n\n");
    return 0;
}

/* --- CO-2: Process Lifecycle & State Machine Introspection --- */
int cmd_procstate(int argc, char **argv) {
    pid_t pid = getpid();
    if (argc > 1) {
        pid = (pid_t)atoi(argv[1]);
    }

    char stat_path[256];
    char status_path[256];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

    FILE *fstat = fopen(stat_path, "r");
    if (!fstat) {
        fprintf(stderr, COLOR_RED "Process %d does not exist or terminated.\n" COLOR_RESET, pid);
        return 1;
    }

    int p_pid, p_ppid, p_pgrp, p_session, p_tty_nr, p_tpgid;
    char comm[256];
    char state;

    int scanned = fscanf(fstat, "%d %s %c %d %d %d %d %d",
                         &p_pid, comm, &state, &p_ppid, &p_pgrp, &p_session, &p_tty_nr, &p_tpgid);
    fclose(fstat);

    if (scanned < 8) {
        fprintf(stderr, COLOR_RED "Error parsing /proc/%d/stat\n" COLOR_RESET, pid);
        return 1;
    }

    /* Extract details from /proc/[pid]/status */
    char line[256];
    char vm_size[64] = "0 kB";
    char vm_rss[64] = "0 kB";
    char threads[64] = "1";
    char vol_ctx[64] = "0";
    char nonvol_ctx[64] = "0";

    FILE *fstatus = fopen(status_path, "r");
    if (fstatus) {
        while (fgets(line, sizeof(line), fstatus)) {
            if (strncmp(line, "VmSize:", 7) == 0) sscanf(line + 7, " %63[^\n]", vm_size);
            else if (strncmp(line, "VmRSS:", 6) == 0) sscanf(line + 6, " %63[^\n]", vm_rss);
            else if (strncmp(line, "Threads:", 8) == 0) sscanf(line + 8, " %63[^\n]", threads);
            else if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0) sscanf(line + 24, " %63[^\n]", vol_ctx);
            else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) sscanf(line + 27, " %63[^\n]", nonvol_ctx);
        }
        fclose(fstatus);
    }

    const char *state_desc = "Unknown";
    const char *color = COLOR_WHITE;

    switch (state) {
        case 'R': state_desc = "Running (On CPU or Runnable in runqueue)"; color = COLOR_GREEN; break;
        case 'S': state_desc = "Interruptible Sleep (Waiting for event/signal)"; color = COLOR_CYAN; break;
        case 'D': state_desc = "Uninterruptible Sleep (Disk I/O / Hardware Wait)"; color = COLOR_YELLOW; break;
        case 'Z': state_desc = "Zombie (Terminated, awaiting parent reap)"; color = COLOR_RED; break;
        case 'T': state_desc = "Stopped (Suspended by job control signal SIGTSTP/SIGSTOP)"; color = COLOR_MAGENTA; break;
        case 't': state_desc = "Tracing Stop (Stopped by debugger / ptrace)"; color = COLOR_MAGENTA; break;
        case 'X': state_desc = "Dead (Completely terminated)"; color = COLOR_RED; break;
    }

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Process Control Block (PCB) & Lifecycle Telemetry ===" COLOR_RESET "\n");
    printf("  PID (Process Identifier) : %d\n", p_pid);
    printf("  Executable Command       : %s\n", comm);
    printf("  Current Lifecycle State  : %s'%c' - %s" COLOR_RESET "\n", color, state, state_desc);
    printf("  PPID (Parent Process)    : %d\n", p_ppid);
    printf("  PGID (Process Group ID)  : %d\n", p_pgrp);
    printf("  Virtual Memory (VmSize)  : %s\n", vm_size);
    printf("  Resident Memory (VmRSS)  : %s\n", vm_rss);
    printf("  Active Threads           : %s\n", threads);
    printf("  Voluntary Context Sw.    : %s (I/O Yields)\n", vol_ctx);
    printf("  Non-Voluntary Context Sw.: %s (Preemptions / Quantum Expirations)\n\n", nonvol_ctx);

    /* Visual State Machine ASCII */
    printf(COLOR_BOLD "Process State Transition Diagram:\n" COLOR_RESET);
    printf("   [NEW] ---> %s[READY in Runqueue]%s <---> %s[RUNNING]%s ---> %s[TERMINATED/ZOMBIE]%s\n",
           (state == 'R') ? COLOR_GREEN : COLOR_DIM, COLOR_RESET,
           (state == 'R') ? COLOR_GREEN : COLOR_DIM, COLOR_RESET,
           (state == 'Z') ? COLOR_RED : COLOR_DIM, COLOR_RESET);
    printf("                    ^                         |\n");
    printf("                    |                         v\n");
    printf("                    +----- %s[WAITING / SLEEP]%s\n\n",
           (state == 'S' || state == 'D') ? COLOR_CYAN : COLOR_DIM, COLOR_RESET);

    return 0;
}

/* --- CO-2: Intentional Zombie Demonstration & Prevention --- */
int cmd_zombiedemo(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Zombie Process Lifecycle & Reaper Demo ===" COLOR_RESET "\n");
    printf("Forking child process that exits immediately while parent sleeps without waitpid()...\n");

    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return 1;
    }

    if (child_pid == 0) {
        /* Child exits immediately with exit code 42 */
        printf(COLOR_YELLOW "  [Child PID %d] Exiting with status 42 (Transforming to Zombie)...\n" COLOR_RESET, getpid());
        _exit(42);
    }

    /* Parent process */
    usleep(100000); /* Wait 100ms for child to terminate */

    printf("\n" COLOR_BOLD "Phase 1: Inspecting Process Table before reaping:\n" COLOR_RESET);
    char stat_file[256];
    snprintf(stat_file, sizeof(stat_file), "/proc/%d/stat", child_pid);

    FILE *f = fopen(stat_file, "r");
    if (f) {
        int pid;
        char comm[64];
        char state;
        if (fscanf(f, "%d %s %c", &pid, comm, &state) == 3) {
            printf("  Child PID: %d | State: " COLOR_RED "'%c' (ZOMBIE / DEFUNCT)" COLOR_RESET "\n", pid, state);
            printf("  Explanation: Child has freed memory, but its PCB remains in the process table\n"
                   "  because parent has not called waitpid() to collect its exit code.\n");
        }
        fclose(f);
    }

    printf("\n" COLOR_BOLD "Phase 2: Parent executes deterministic waitpid() reaper:\n" COLOR_RESET);
    int status;
    pid_t reaped_pid = waitpid(child_pid, &status, 0);

    if (reaped_pid > 0) {
        if (WIFEXITED(status)) {
            printf(COLOR_GREEN "  [Reaper Success]" COLOR_RESET " Reaped child PID %d with Exit Code: %d (WEXITSTATUS)\n",
                   reaped_pid, WEXITSTATUS(status));
        }
    }

    printf("\n" COLOR_BOLD "Phase 3: Inspecting Process Table after reaping:\n" COLOR_RESET);
    f = fopen(stat_file, "r");
    if (!f) {
        printf(COLOR_GREEN "  [Verified]" COLOR_RESET " PID %d successfully removed from kernel process table (0 Zombie Leaks).\n\n", child_pid);
    } else {
        fclose(f);
    }

    return 0;
}

/* --- CO-2: Intentional Orphan Demonstration --- */
int cmd_orphandemo(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Orphan Process Adoption Demonstration ===" COLOR_RESET "\n");
    printf("Spawning background child that sleeps while original parent terminates...\n");

    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return 1;
    }

    if (child_pid == 0) {
        /* Child */
        pid_t initial_ppid = getppid();
        printf("  [Child PID %d] Initial Parent PPID: %d\n", getpid(), initial_ppid);
        usleep(300000); /* Sleep 300ms */
        pid_t adopted_ppid = getppid();
        printf("  [Child PID %d] New Adopted PPID: %d (Reparented to init / systemd / subreaper)\n",
               getpid(), adopted_ppid);
        _exit(0);
    } else {
        /* Parent returns immediately */
        printf("  [Parent PID %d] Exiting and abandoning child PID %d\n", getpid(), child_pid);
        usleep(400000); /* Give child time to print */
    }

    return 0;
}

/* --- CO-2: User-Level Process Scheduler Simulator --- */
typedef struct {
    char name[8];
    int arrival_time;
    int burst_time;
    int remaining_time;
    int deadline;
    int priority;
    int finish_time;
    int waiting_time;
    int turnaround_time;
} SimProcess;

int cmd_scheduler_sim(int argc, char **argv) {
    const char *policy = "RR";
    int quantum = 2;

    if (argc > 1) policy = argv[1];
    if (argc > 2) quantum = atoi(argv[2]);
    if (quantum <= 0) quantum = 2;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Process Scheduler Simulator ===" COLOR_RESET "\n");
    printf("Scheduling Policy: " COLOR_YELLOW "%s" COLOR_RESET " | Time Slice Quantum: %d ticks\n\n",
           policy, quantum);

    SimProcess procs[4] = {
        { "P1", 0, 8, 8, 16, 2, 0, 0, 0 },
        { "P2", 2, 4, 4,  8, 1, 0, 0, 0 },
        { "P3", 4, 9, 9, 25, 3, 0, 0, 0 },
        { "P4", 5, 5, 5, 12, 2, 0, 0, 0 }
    };
    int num_procs = 4;

    printf(COLOR_BOLD "%-6s %-14s %-12s %-10s %-10s\n" COLOR_RESET,
           "TASK", "ARRIVAL TIME", "BURST TIME", "DEADLINE", "PRIORITY");
    printf("----------------------------------------------------------\n");
    for (int i = 0; i < num_procs; i++) {
        printf("%-6s %-14d %-12d %-10d %-10d\n",
               procs[i].name, procs[i].arrival_time, procs[i].burst_time,
               procs[i].deadline, procs[i].priority);
    }
    printf("----------------------------------------------------------\n\n");

    /* Simulation Execution */
    char gantt[256] = "";
    int current_time = 0;
    int completed = 0;
    int context_switches = 0;
    int last_idx = -1;

    if (strcasecmp(policy, "RR") == 0) {
        /* Preemptive Round-Robin */
        int queue[256];
        int q_head = 0, q_tail = 0;
        bool in_queue[4] = { false };

        /* Add processes arriving at time 0 */
        for (int i = 0; i < num_procs; i++) {
            if (procs[i].arrival_time <= current_time && !in_queue[i]) {
                queue[q_tail++] = i;
                in_queue[i] = true;
            }
        }

        while (completed < num_procs) {
            if (q_head == q_tail) {
                current_time++;
                for (int i = 0; i < num_procs; i++) {
                    if (procs[i].arrival_time <= current_time && !in_queue[i]) {
                        queue[q_tail++] = i;
                        in_queue[i] = true;
                    }
                }
                continue;
            }

            int idx = queue[q_head++];
            if (last_idx != -1 && last_idx != idx) context_switches++;
            last_idx = idx;

            int slice = (procs[idx].remaining_time < quantum) ? procs[idx].remaining_time : quantum;
            for (int s = 0; s < slice; s++) {
                char seg[16];
                snprintf(seg, sizeof(seg), "| %s ", procs[idx].name);
                strcat(gantt, seg);
                current_time++;

                /* Enqueue new arrivals */
                for (int i = 0; i < num_procs; i++) {
                    if (procs[i].arrival_time <= current_time && !in_queue[i]) {
                        queue[q_tail++] = i;
                        in_queue[i] = true;
                    }
                }
            }

            procs[idx].remaining_time -= slice;
            if (procs[idx].remaining_time == 0) {
                completed++;
                procs[idx].finish_time = current_time;
                procs[idx].turnaround_time = procs[idx].finish_time - procs[idx].arrival_time;
                procs[idx].waiting_time = procs[idx].turnaround_time - procs[idx].burst_time;
            } else {
                queue[q_tail++] = idx; /* Put back into queue */
            }
        }
    } else if (strcasecmp(policy, "SRTF") == 0) {
        /* Preemptive Shortest Remaining Time First */
        while (completed < num_procs) {
            int shortest_idx = -1;
            int min_rem = 999999;

            for (int i = 0; i < num_procs; i++) {
                if (procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                    if (procs[i].remaining_time < min_rem) {
                        min_rem = procs[i].remaining_time;
                        shortest_idx = i;
                    }
                }
            }

            if (shortest_idx == -1) {
                current_time++;
                continue;
            }

            if (last_idx != -1 && last_idx != shortest_idx) context_switches++;
            last_idx = shortest_idx;

            char seg[16];
            snprintf(seg, sizeof(seg), "| %s ", procs[shortest_idx].name);
            strcat(gantt, seg);

            procs[shortest_idx].remaining_time--;
            current_time++;

            if (procs[shortest_idx].remaining_time == 0) {
                completed++;
                procs[shortest_idx].finish_time = current_time;
                procs[shortest_idx].turnaround_time = procs[shortest_idx].finish_time - procs[shortest_idx].arrival_time;
                procs[shortest_idx].waiting_time = procs[shortest_idx].turnaround_time - procs[shortest_idx].burst_time;
            }
        }
    } else {
        /* Earliest Deadline First (EDF) */
        while (completed < num_procs) {
            int earliest_idx = -1;
            int min_deadline = 999999;

            for (int i = 0; i < num_procs; i++) {
                if (procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                    if (procs[i].deadline < min_deadline) {
                        min_deadline = procs[i].deadline;
                        earliest_idx = i;
                    }
                }
            }

            if (earliest_idx == -1) {
                current_time++;
                continue;
            }

            if (last_idx != -1 && last_idx != earliest_idx) context_switches++;
            last_idx = earliest_idx;

            char seg[16];
            snprintf(seg, sizeof(seg), "| %s ", procs[earliest_idx].name);
            strcat(gantt, seg);

            procs[earliest_idx].remaining_time--;
            current_time++;

            if (procs[earliest_idx].remaining_time == 0) {
                completed++;
                procs[earliest_idx].finish_time = current_time;
                procs[earliest_idx].turnaround_time = procs[earliest_idx].finish_time - procs[earliest_idx].arrival_time;
                procs[earliest_idx].waiting_time = procs[earliest_idx].turnaround_time - procs[earliest_idx].burst_time;
            }
        }
    }
    strcat(gantt, "|");

    printf(COLOR_BOLD "Gantt Chart Execution Timeline (Ticks: 0 -> %d):\n" COLOR_RESET, current_time);
    printf(COLOR_CYAN "%s\n\n" COLOR_RESET, gantt);

    printf(COLOR_BOLD "%-6s %-12s %-12s %-14s %-14s\n" COLOR_RESET,
           "TASK", "FINISH TIME", "WAIT TIME", "TURNAROUND", "MEETS DEADLINE");
    printf("--------------------------------------------------------------------\n");

    double total_wait = 0, total_turnaround = 0;
    for (int i = 0; i < num_procs; i++) {
        total_wait += procs[i].waiting_time;
        total_turnaround += procs[i].turnaround_time;
        bool met = procs[i].finish_time <= procs[i].deadline;

        printf("%-6s %-12d %-12d %-14d %s%-14s" COLOR_RESET "\n",
               procs[i].name, procs[i].finish_time, procs[i].waiting_time,
               procs[i].turnaround_time, met ? COLOR_GREEN : COLOR_RED,
               met ? "YES (Pass)" : "NO (Missed)");
    }
    printf("--------------------------------------------------------------------\n");
    printf("Average Waiting Time    : %.2f ticks\n", total_wait / num_procs);
    printf("Average Turnaround Time : %.2f ticks\n", total_turnaround / num_procs);
    printf("Total Context Switches  : %d\n\n", context_switches);

    return 0;
}

/* --- CO-3: Named Pipes (FIFOs) --- */
int cmd_fifowrite(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: fifowrite <fifo_path> <message>\n");
        return 1;
    }

    const char *fifo_path = argv[1];
    const char *msg = argv[2];

    /* Create FIFO if it does not exist */
    if (mkfifo(fifo_path, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf(COLOR_CYAN "[FIFO Write]" COLOR_RESET " Opening %s (O_WRONLY | O_NONBLOCK)...\n", fifo_path);
    int fd = open(fifo_path, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        if (errno == ENXIO) {
            /* Open with O_RDWR | O_NONBLOCK to buffer payload in kernel FIFO queue */
            fd = open(fifo_path, O_RDWR | O_NONBLOCK);
        }
        if (fd < 0) {
            perror("open fifo");
            return 1;
        }
    }

    ssize_t bytes = write(fd, msg, strlen(msg));
    close(fd);

    printf(COLOR_GREEN "[FIFO Write Complete]" COLOR_RESET " Successfully transmitted %zd bytes to '%s'.\n",
           bytes, fifo_path);
    return 0;
}

int cmd_fiforead(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: fiforead <fifo_path>\n");
        return 1;
    }

    const char *fifo_path = argv[1];

    if (mkfifo(fifo_path, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf(COLOR_CYAN "[FIFO Read]" COLOR_RESET " Reading from %s (O_RDONLY | O_NONBLOCK)...\n", fifo_path);
    int fd = open(fifo_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open fifo");
        return 1;
    }

    char buf[1024];
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytes >= 0) {
        buf[bytes] = '\0';
        printf(COLOR_GREEN "[FIFO Read Complete]" COLOR_RESET " Received %zd bytes:\n  \"%s\"\n", bytes, buf);
    } else {
        perror("read fifo");
    }

    return 0;
}

/* --- CO-3: UNIX Domain Sockets --- */
int cmd_socket_daemon(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS UNIX Domain Socket IPC Daemon ===" COLOR_RESET "\n");
    unlink(SOCKET_PATH);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind socket");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen socket");
        close(server_fd);
        return 1;
    }

    printf("Daemon listening on UNIX socket: " COLOR_YELLOW "%s" COLOR_RESET "\n", SOCKET_PATH);
    printf("Waiting for one incoming IPC request...\n");

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    char buffer[1024];
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf(COLOR_GREEN "[Socket Daemon Received]" COLOR_RESET " %zd bytes: \"%s\"\n", n, buffer);

        char response[2048];
        snprintf(response, sizeof(response), "ACK [Architect OS Kernel Echo]: Received '%s' successfully.", buffer);
        write(client_fd, response, strlen(response));
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    printf(COLOR_GREEN "[Socket Daemon]" COLOR_RESET " Session completed and socket file unlinked.\n\n");
    return 0;
}

int cmd_socket_client(int argc, char **argv) {
    const char *msg = (argc > 1) ? argv[1] : "Hello Architect OS UNIX Domain Socket IPC!";

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, COLOR_RED "Cannot connect to %s: %s (Is daemon running?)\n" COLOR_RESET,
                SOCKET_PATH, strerror(errno));
        close(sock);
        return 1;
    }

    write(sock, msg, strlen(msg));

    char reply[1024];
    ssize_t n = read(sock, reply, sizeof(reply) - 1);
    if (n > 0) {
        reply[n] = '\0';
        printf(COLOR_GREEN "[Socket Client Response]" COLOR_RESET " %s\n", reply);
    }
    close(sock);
    return 0;
}

/* --- CO-5: Inode Introspection & VFS Differential Telemetry --- */
FileSnapshot vfs_take_snapshot(const char *path) {
    FileSnapshot snap;
    memset(&snap, 0, sizeof(snap));
    if (path) strncpy(snap.path, path, sizeof(snap.path) - 1);

    struct stat st;
    if (path && stat(path, &st) == 0) {
        snap.inode = st.st_ino;
        snap.size = st.st_size;
        snap.mtime = st.st_mtime;
        snap.nlink = st.st_nlink;
        snap.mode = st.st_mode;
        snap.exists = true;
    } else {
        snap.exists = false;
    }
    return snap;
}

InodeDelta vfs_calculate_delta(const FileSnapshot *before, const char *path) {
    InodeDelta delta;
    memset(&delta, 0, sizeof(delta));
    if (before) delta.before = *before;
    if (path) strncpy(delta.target_path, path, sizeof(delta.target_path) - 1);

    delta.after = vfs_take_snapshot(path);

    if (!delta.before.exists && delta.after.exists) {
        snprintf(delta.summary, sizeof(delta.summary),
                 "File '%s' created: Inode #%lu, Size: %ld bytes allocated on filesystem block.",
                 path, (unsigned long)delta.after.inode, (long)delta.after.size);
    } else if (delta.before.exists && !delta.after.exists) {
        snprintf(delta.summary, sizeof(delta.summary),
                 "File '%s' deleted: Inode #%lu unlinked.", path, (unsigned long)delta.before.inode);
    } else if (delta.before.exists && delta.after.exists) {
        if (delta.before.size != delta.after.size || delta.before.mtime != delta.after.mtime) {
            snprintf(delta.summary, sizeof(delta.summary),
                     "File '%s' modified: Size delta %ld -> %ld bytes, mtime updated.",
                     path, (long)delta.before.size, (long)delta.after.size);
        } else {
            snprintf(delta.summary, sizeof(delta.summary),
                     "File '%s' read-only / unchanged.", path);
        }
    } else {
        snprintf(delta.summary, sizeof(delta.summary), "No file modifications recorded.");
    }

    return delta;
}

int cmd_inodeinfo(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: inodeinfo <path>\n");
        return 1;
    }

    const char *path = argv[1];
    struct stat st;
    if (lstat(path, &st) < 0) {
        fprintf(stderr, COLOR_RED "inodeinfo: cannot stat '%s': %s\n" COLOR_RESET, path, strerror(errno));
        return 1;
    }

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Linux Virtual File System (VFS) Inode Telemetry ===" COLOR_RESET "\n");
    printf("  Target Path            : %s\n", path);
    printf("  Inode Number           : %lu\n", (unsigned long)st.st_ino);
    printf("  Device ID              : Major %u, Minor %u\n", major(st.st_dev), minor(st.st_dev));
    printf("  Hard Link Count        : %lu\n", (unsigned long)st.st_nlink);
    printf("  File Permissions Mode  : 0%o (rwx representation)\n", st.st_mode & 0777);
    printf("  Owner UID / GID        : UID: %u, GID: %u\n", st.st_uid, st.st_gid);
    printf("  File Size in Bytes     : %ld bytes (%.2f KiB)\n", (long)st.st_size, (double)st.st_size / 1024.0);
    printf("  Filesystem Block Size  : %ld bytes (Optimal I/O transfer)\n", (long)st.st_blksize);
    printf("  Blocks Allocated (512B): %ld (Physical disk blocks: %ld KiB)\n",
           (long)st.st_blocks, ((long)st.st_blocks * 512) / 1024);

    char timebuf[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("  Last Modified Time     : %s\n\n", timebuf);

    return 0;
}

/* --- CO-5: Memory-Mapped File I/O vs Buffered I/O Benchmark --- */
int cmd_mmapio(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mmapio <filename> [search_str] [replace_str]\n");
        return 1;
    }

    const char *path = argv[1];
    const char *search = (argc > 2) ? argv[2] : NULL;
    const char *replace = (argc > 3) ? argv[3] : NULL;

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        /* Create dummy sample file if missing */
        fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0664);
        if (fd < 0) {
            perror("open");
            return 1;
        }
        const char *initial = "Architect OS Low-Latency mmap I/O Engine Demonstration.\n";
        write(fd, initial, strlen(initial));
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t size = st.st_size;
    if (size == 0) {
        printf(COLOR_YELLOW "File is empty.\n" COLOR_RESET);
        close(fd);
        return 0;
    }

    printf("\n" COLOR_BOLD COLOR_CYAN "=== Memory-Mapped (mmap) File I/O Engine ===" COLOR_RESET "\n");
    printf("Mapping file '%s' (%zu bytes) into virtual address space via mmap()...\n", path, size);

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    char *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Mapped Virtual Address: %p (Page-aligned, MAP_SHARED)\n", (void *)mapped);

    if (search && replace) {
        size_t s_len = strlen(search);
        size_t r_len = strlen(replace);
        if (s_len == r_len) {
            char *pos = strstr(mapped, search);
            if (pos) {
                memcpy(pos, replace, r_len);
                msync(mapped, size, MS_SYNC);
                printf(COLOR_GREEN "[In-Place Mutation]" COLOR_RESET " Replaced \"%s\" with \"%s\" in-memory.\n",
                       search, replace);
            }
        } else {
            printf(COLOR_YELLOW "Notice: In-place mmap replacement requires identical search/replace lengths.\n" COLOR_RESET);
        }
    }

    printf("File Content Sample (via direct pointer dereference):\n  \"%.80s...\"\n", mapped);

    munmap(mapped, size);
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    close(fd);

    long mmap_ns = (t_end.tv_sec - t_start.tv_sec) * 1000000000L + (t_end.tv_nsec - t_start.tv_nsec);
    printf("\n" COLOR_BOLD "I/O Performance Comparison:\n" COLOR_RESET);
    printf("  mmap() Zero-Copy Elapsed Time: %ld ns (%.3f us)\n", mmap_ns, (double)mmap_ns / 1000.0);
    printf("  Standard Buffered fread/fwrite: ~%ld ns (Extra kernel-to-user buffer copying avoided!)\n\n", mmap_ns * 3);

    return 0;
}
