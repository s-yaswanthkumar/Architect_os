#include "architect.h"

VisualizerMode g_visualizer_mode = MODE_ADVANCED;
bool g_visualize_enabled = true;
bool g_exit_requested = false;

/* --- Plain-English Command Dictionary for Summary Mode --- */
static const char *get_command_explanation(const char *cmd_name) {
    if (!cmd_name) return "Executes program binary in child process.";

    /* Common Linux Utilities */
    if (strcmp(cmd_name, "ls") == 0) return "Lists directory contents, file names, permissions, and sizes.";
    if (strcmp(cmd_name, "cat") == 0) return "Concatenates file data and streams it directly to standard output.";
    if (strcmp(cmd_name, "grep") == 0) return "Searches text or input streams for patterns matching a query string.";
    if (strcmp(cmd_name, "mkdir") == 0) return "Creates a new directory entry and allocates an inode on the filesystem.";
    if (strcmp(cmd_name, "rm") == 0) return "Unlinks files or directory entries and deallocates filesystem blocks.";
    if (strcmp(cmd_name, "touch") == 0) return "Updates access and modification timestamps or creates an empty file.";
    if (strcmp(cmd_name, "pwd") == 0) return "Prints the absolute path of the current working directory.";
    if (strcmp(cmd_name, "echo") == 0) return "Writes arguments to standard output stream followed by a newline.";
    if (strcmp(cmd_name, "cp") == 0) return "Copies source files or directories to a destination path.";
    if (strcmp(cmd_name, "mv") == 0) return "Moves or renames files/directories within the virtual file system.";
    if (strcmp(cmd_name, "sleep") == 0) return "Suspends process execution for a specified duration of seconds.";
    if (strcmp(cmd_name, "find") == 0) return "Searches the directory hierarchy for files matching criteria.";
    if (strcmp(cmd_name, "wc") == 0) return "Counts lines, words, and byte/character counts in input files.";
    if (strcmp(cmd_name, "head") == 0) return "Outputs the first part (default 10 lines) of files or streams.";
    if (strcmp(cmd_name, "tail") == 0) return "Outputs the last part (default 10 lines) of files or streams.";
    if (strcmp(cmd_name, "ps") == 0) return "Reports snapshot of active processes currently running on the system.";
    if (strcmp(cmd_name, "kill") == 0) return "Sends POSIX termination or control signals to specified process IDs.";
    if (strcmp(cmd_name, "chmod") == 0) return "Modifies file access permission bits (read, write, execute).";
    if (strcmp(cmd_name, "chown") == 0) return "Changes file owner user ID and group ID ownership.";
    if (strcmp(cmd_name, "nano") == 0 || strcmp(cmd_name, "vim") == 0 || strcmp(cmd_name, "vi") == 0)
        return "Interactive full-screen text editor for modifying files.";
    if (strcmp(cmd_name, "date") == 0) return "Displays or sets the system date and time.";
    if (strcmp(cmd_name, "uname") == 0) return "Prints system hardware architecture and operating system kernel details.";
    if (strcmp(cmd_name, "whoami") == 0) return "Displays the effective user name of the current shell session.";
    if (strcmp(cmd_name, "tr") == 0) return "Translates, squeezes, or deletes characters from standard input stream.";
    if (strcmp(cmd_name, "rev") == 0) return "Reverses characters on each line from standard input stream.";
    if (strcmp(cmd_name, "sort") == 0) return "Sorts lines of text input alphabetically or numerically.";
    if (strcmp(cmd_name, "uniq") == 0) return "Filters or omits repeated lines from sorted input stream.";

    /* Architect OS Course Subsystem Built-ins */
    if (strcmp(cmd_name, "sysinspect") == 0)
        return "[CO-1] Inspects hardware Ring 3->0 trap, intercepts syscalls, and reads /proc/sys limits.";
    if (strcmp(cmd_name, "procstate") == 0)
        return "[CO-2] Inspects child process lifecycle state (Ready/Running/Waiting/Zombie) via /proc/[pid]/stat.";
    if (strcmp(cmd_name, "zombiedemo") == 0)
        return "[CO-2] Spawns an intentional zombie process, detects 'Z' state, and demonstrates parent waitpid() reaping.";
    if (strcmp(cmd_name, "orphandemo") == 0)
        return "[CO-2] Demonstrates orphan process reparenting to init/systemd (PID 1) upon parent termination.";
    if (strcmp(cmd_name, "scheduler-sim") == 0)
        return "[CO-2] Simulates CPU scheduling algorithms (Round-Robin, EDF, SRTF) with Gantt chart metrics.";
    if (strcmp(cmd_name, "jobs") == 0)
        return "[CO-3] Lists all active background tasks and stopped jobs in the shell job table.";
    if (strcmp(cmd_name, "fg") == 0)
        return "[CO-3] Moves a background or stopped job to the foreground with terminal control and SIGCONT.";
    if (strcmp(cmd_name, "bg") == 0)
        return "[CO-3] Resumes a stopped job in the background with SIGCONT.";
    if (strcmp(cmd_name, "fifowrite") == 0)
        return "[CO-3] Writes an IPC payload to a persistent named pipe (FIFO) on the filesystem.";
    if (strcmp(cmd_name, "fiforead") == 0)
        return "[CO-3] Reads an IPC payload from a persistent named pipe (FIFO) from the filesystem.";
    if (strcmp(cmd_name, "socket-daemon") == 0)
        return "[CO-3] Starts a UNIX Domain Socket IPC server on /tmp/architect_os.sock.";
    if (strcmp(cmd_name, "socket-client") == 0)
        return "[CO-3] Connects to UNIX Domain Socket IPC server and exchanges message payloads.";
    if (strcmp(cmd_name, "vmmaps") == 0)
        return "[CO-4] Reconstructs process virtual memory address space layout (Text, Data, BSS, Heap, Stack).";
    if (strcmp(cmd_name, "cowdemo") == 0)
        return "[CO-4] Demonstrates Copy-on-Write (COW) memory page sharing and divergence upon write via pagemap.";
    if (strcmp(cmd_name, "arch-alloc") == 0)
        return "[CO-4] Allocates memory block from custom mmap-backed segregated free-list heap allocator.";
    if (strcmp(cmd_name, "arch-free") == 0)
        return "[CO-4] Frees heap block and triggers contiguous bidirectional block coalescing.";
    if (strcmp(cmd_name, "heapinfo") == 0)
        return "[CO-4] Displays custom heap allocator telemetry, block tables, and fragmentation index.";
    if (strcmp(cmd_name, "inodeinfo") == 0)
        return "[CO-5] Queries Linux VFS inode metadata: inode number, device ID, link count, blocks, and timestamps.";
    if (strcmp(cmd_name, "mmapio") == 0)
        return "[CO-5] Maps file into memory (mmap) for zero-copy in-place editing benchmarked against buffered I/O.";
    if (strcmp(cmd_name, "threads-demo") == 0)
        return "[CO-6] Demonstrates unprotected multithreaded data race counter vs mutex-synchronized execution.";
    if (strcmp(cmd_name, "pc-demo") == 0)
        return "[CO-6] Runs Producer-Consumer bounded queue coordinated with POSIX semaphores and mutex.";
    if (strcmp(cmd_name, "deadlock-demo") == 0)
        return "[CO-6] Demonstrates circular-wait deadlock hazard vs prevention via strict lock ordering.";
    if (strcmp(cmd_name, "mode") == 0)
        return "Toggles shell visualizer mode between 'advanced', 'summary', and 'off'.";
    if (strcmp(cmd_name, "visualize") == 0 || strcmp(cmd_name, "visualizer") == 0)
        return "Configures real-time command execution telemetry and visualizer mode.";
    if (strcmp(cmd_name, "summary") == 0)
        return "Switches shell display mode to concise plain-English command summary.";
    if (strcmp(cmd_name, "cd") == 0)
        return "Changes the current working directory of the shell process.";
    if (strcmp(cmd_name, "clear") == 0)
        return "Clears the terminal screen buffer.";
    if (strcmp(cmd_name, "history") == 0)
        return "Displays previously entered shell commands.";
    if (strcmp(cmd_name, "help") == 0)
        return "Prints comprehensive Architect OS command help and Course Outcome index.";
    if (strcmp(cmd_name, "exit") == 0 || strcmp(cmd_name, "quit") == 0)
        return "Shuts down Architect OS shell and frees all allocated subsystem resources.";

    return "Executes external Linux executable binary in a spawned child process.";
}

/* --- 🌟 Simple Summary Mode Visualizer --- */
void visualizer_render_summary(const Pipeline *p) {
    if (g_visualizer_mode != MODE_SUMMARY || !p) return;

    printf("\n" COLOR_BOLD COLOR_GREEN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Command Summary Mode]                                                                 |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");

    if (p->cmd_count == 1) {
        SimpleCommand *cmd = p->commands[0];
        const char *name = (cmd->argc > 0) ? cmd->argv[0] : "empty";
        printf(COLOR_BOLD "  Command     : " COLOR_YELLOW "%s\n" COLOR_RESET, p->raw_cmdline ? p->raw_cmdline : name);
        printf(COLOR_BOLD "  Description : " COLOR_RESET "%s\n", get_command_explanation(name));

        if (cmd->argc > 1) {
            printf(COLOR_BOLD "  Arguments   : " COLOR_RESET);
            for (size_t a = 1; a < cmd->argc; a++) {
                printf("%s ", cmd->argv[a]);
            }
            printf("\n");
        }

        if (cmd->redir_count > 0) {
            printf(COLOR_BOLD "  Redirection : " COLOR_RESET);
            for (size_t r = 0; r < cmd->redir_count; r++) {
                if (cmd->redirs[r].type == REDIR_IN)
                    printf("Reading input from file '%s'; ", cmd->redirs[r].target);
                else if (cmd->redirs[r].type == REDIR_OUT)
                    printf("Writing output to file '%s' (overwrite); ", cmd->redirs[r].target);
                else if (cmd->redirs[r].type == REDIR_APPEND)
                    printf("Appending output to file '%s'; ", cmd->redirs[r].target);
                else if (cmd->redirs[r].type == REDIR_ERR)
                    printf("Redirecting stderr to file '%s'; ", cmd->redirs[r].target);
                else if (cmd->redirs[r].type == REDIR_ERR_OUT)
                    printf("Redirecting stderr into stdout stream; ");
            }
            printf("\n");
        }
    } else {
        printf(COLOR_BOLD "  Pipeline    : " COLOR_YELLOW "%s\n" COLOR_RESET, p->raw_cmdline);
        printf(COLOR_BOLD "  Description : " COLOR_RESET "Chains %zu commands via kernel anonymous pipes (zero temp files).\n", p->cmd_count);
        printf(COLOR_BOLD "  Data Flow   : " COLOR_RESET);
        for (size_t i = 0; i < p->cmd_count; i++) {
            SimpleCommand *cmd = p->commands[i];
            const char *cname = (cmd->argc > 0) ? cmd->argv[0] : "empty";
            printf("[%s]", cname);
            if (i + 1 < p->cmd_count) printf(" ---> (kernel pipe) ---> ");
        }
        printf("\n");
    }

    if (p->is_background) {
        printf(COLOR_BOLD "  Execution   : " COLOR_CYAN "Background task (&) - shell returns prompt immediately.\n" COLOR_RESET);
    }
    printf(COLOR_BOLD COLOR_GREEN "+-----------------------------------------------------------------------------------------+\n\n" COLOR_RESET);
}

/* --- 🌟 Advanced 6-Step Command Visualizer Subsystem --- */
void visualizer_step1_ast(const Pipeline *p) {
    if (g_visualizer_mode != MODE_ADVANCED || !p) return;

    printf("\n" COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 1: Parsing & AST Blueprint]                                                      |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");
    printf(COLOR_BOLD "  Raw Input   : " COLOR_YELLOW "\"%s\"%s\n" COLOR_RESET,
           p->raw_cmdline ? p->raw_cmdline : "",
           p->is_background ? " (Background Mode '&')" : "");

    printf(COLOR_BOLD "  Pipeline AST: " COLOR_RESET);
    for (size_t i = 0; i < p->cmd_count; i++) {
        SimpleCommand *cmd = p->commands[i];
        const char *in_src = (i == 0) ? "fd 0 (stdin)" : "Pipe In";
        const char *out_tgt = (i == p->cmd_count - 1) ? "fd 1 (stdout)" : "Pipe Out";

        for (size_t r = 0; r < cmd->redir_count; r++) {
            if (cmd->redirs[r].type == REDIR_IN) in_src = cmd->redirs[r].target;
            else if (cmd->redirs[r].type == REDIR_OUT) out_tgt = cmd->redirs[r].target;
            else if (cmd->redirs[r].type == REDIR_APPEND) out_tgt = cmd->redirs[r].target;
        }

        printf("[%s] -> [Command: " COLOR_GREEN "%s" COLOR_RESET "] -> [%s]",
               in_src, cmd->argc > 0 ? cmd->argv[0] : "empty", out_tgt);
        if (i + 1 < p->cmd_count) printf("  |  ");
    }
    printf("\n");
}

void visualizer_step2_modeswitch(const char *cmd_name) {
    if (g_visualizer_mode != MODE_ADVANCED) return;

    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 2: User Space -> Kernel Space Mode Switch (CO-1)]                                |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");
    printf("  Hardware Trap    : CPU CPL 3 (User Ring 3) " COLOR_YELLOW "--[syscall instruction]-->" COLOR_RESET " CPL 0 (Kernel Ring 0)\n");
    printf("  Low-Level Entry  : " COLOR_GREEN "entry_SYSCALL_64" COLOR_RESET " via arch/x86/entry/entry_64.S\n");
    printf("  Target Subsystem : sys_clone (Process Fork) -> sys_execve (ELF Binary Loader: '%s')\n", cmd_name ? cmd_name : "builtin");
    printf("  Register Save    : User %%rip -> %%rcx, %%rflags -> %%r11; Kernel TSS switches %%rsp to interrupt stack.\n");
}

void visualizer_step3_pcb(pid_t parent_pid, pid_t child_pid, const char *cmd_name) {
    if (g_visualizer_mode != MODE_ADVANCED) return;

    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 3: Process Lifecycle & PCB Allocation (CO-2)]                                    |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");
    printf("  Parent Process   : PID %d (Architect OS Interactive Shell)\n", parent_pid);
    printf("  Spawned Child    : PID %d (Task: '%s')\n", child_pid, cmd_name ? cmd_name : "builtin");
    printf("  State Transition : " COLOR_BLUE "[NEW PCB allocated]" COLOR_RESET " -> "
           COLOR_YELLOW "[READY in CFS runqueue]" COLOR_RESET " -> "
           COLOR_GREEN "[RUNNING on CPU Core]" COLOR_RESET "\n");
}

void visualizer_step4_vmm(pid_t pid) {
    if (g_visualizer_mode != MODE_ADVANCED) return;

    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 4: Memory Space Layout Changes (CO-4)]                                           |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");
    printf("  Target Address Space (PID: %d):\n", pid);
    printf("    * Text Segment     : Loaded ELF PT_LOAD segments (.text executable code)\n");
    printf("    * Data / BSS       : Initialized globals and uninitialized zero-fill pages\n");
    printf("    * Heap Pointer     : mm->brk initialized; dynamic heap ready via sbrk/mmap\n");
    printf("    * User Stack Frame : Initialized with argc, argv pointers, and envp environment block\n");
}

void visualizer_step5_vfs(const InodeDelta *delta) {
    if (g_visualizer_mode != MODE_ADVANCED || !delta) return;

    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 5: File & Inode State Delta (CO-5)]                                              |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");

    if (delta->target_path[0] != '\0') {
        printf("  Target Path      : %s\n", delta->target_path);
        if (delta->before.exists) {
            printf("  Before Execution : Inode: #%-8lu | Size: %-6ld bytes | Status: Allocated\n",
                   (unsigned long)delta->before.inode, (long)delta->before.size);
        } else {
            printf("  Before Execution : Inode: [None]    | Size: 0 bytes      | Status: Unallocated\n");
        }

        if (delta->after.exists) {
            printf("  After Execution  : Inode: #%-8lu | Size: %-6ld bytes | Status: Committed to Disk\n",
                   (unsigned long)delta->after.inode, (long)delta->after.size);
        } else {
            printf("  After Execution  : Inode: [None]    | Size: 0 bytes      | Status: Unlinked / Removed\n");
        }
        printf("  Plain-English    : " COLOR_YELLOW "%s" COLOR_RESET "\n", delta->summary);
    } else {
        printf("  I/O Inspection   : Standard interactive terminal streams (No persistent file modified).\n");
    }
}

void visualizer_step6_ipc_exit(int exit_code, int term_sig, bool stopped, const char *ipc_details) {
    if (g_visualizer_mode != MODE_ADVANCED) return;

    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n");
    printf("|  [Step 6: IPC & Exit Code Accounting (CO-2, CO-3)]                                      |\n");
    printf("+-----------------------------------------------------------------------------------------+" COLOR_RESET "\n");
    printf("  IPC Plumbing     : %s\n", ipc_details ? ipc_details : "Standard POSIX file descriptors (0, 1, 2)");

    if (stopped) {
        printf("  Reaping Status   : " COLOR_YELLOW "WIFSTOPPED(status) == true -> Job Suspended via Signal %d" COLOR_RESET "\n", term_sig);
    } else if (term_sig > 0) {
        printf("  Reaping Status   : " COLOR_RED "WIFSIGNALED(status) == true -> Terminated via Signal %d" COLOR_RESET "\n", term_sig);
    } else {
        printf("  Reaping Status   : " COLOR_GREEN "WIFEXITED(status) == true -> Exit Code: %d (%s)" COLOR_RESET "\n",
               exit_code, (exit_code == 0) ? "Success" : "Non-Zero Exit");
    }
    printf(COLOR_BOLD COLOR_CYAN "+-----------------------------------------------------------------------------------------+\n\n" COLOR_RESET);
}

/* --- Built-in Commands Subsystem --- */
bool is_builtin_command(const char *name) {
    if (!name) return false;
    static const char *builtins[] = {
        "help", "exit", "quit", "cd", "clear", "history", "visualize", "visualizer",
        "mode", "summary",
        "jobs", "fg", "bg",
        "sysinspect", "procstate", "zombiedemo", "orphandemo", "scheduler-sim",
        "fifowrite", "fiforead", "socket-daemon", "socket-client",
        "vmmaps", "cowdemo", "arch-alloc", "arch-free", "heapinfo",
        "inodeinfo", "mmapio",
        "threads-demo", "pc-demo", "deadlock-demo",
        NULL
    };
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(name, builtins[i]) == 0) return true;
    }
    return false;
}

static void print_shell_help(void) {
    printf("\n" COLOR_BOLD COLOR_CYAN "==================================================================================\n");
    printf("                  ARCHITECT OS: INTERACTIVE SYSTEMS SHELL & EMULATOR\n");
    printf("==================================================================================" COLOR_RESET "\n");
    printf(COLOR_BOLD "Course Outcomes (CO-1 through CO-6) Interactive Subsystems:\n\n" COLOR_RESET);

    printf(COLOR_YELLOW "CO-1: OS Service Layer & Syscall Introspection\n" COLOR_RESET);
    printf("  sysinspect <cmd> [args]    : Traces syscall sequence, explains Ring 3->Ring 0 trap, /proc/sys\n");
    printf("  mode [advanced|summary|off]: Switch between deep visualizer, simple summary, and silent mode\n");
    printf("  summary [on|off]           : Toggle concise plain-English command summary\n");
    printf("  visualize [on|off|adv|sum] : Configure real-time execution visualizer\n");
    printf("  history                    : List previous interactive command inputs\n\n");

    printf(COLOR_YELLOW "CO-2: Process Control, Lifecycle & Scheduling\n" COLOR_RESET);
    printf("  procstate [pid]            : Read /proc/[pid]/stat & status; inspect Ready/Running/Wait/Zombie\n");
    printf("  zombiedemo                 : Spawn un-reaped child, observe 'Z' state, demonstrate clean reaper\n");
    printf("  orphandemo                 : Demonstrate child reparenting to init/systemd (PID 1)\n");
    printf("  scheduler-sim [RR|EDF|SRTF]: User-level scheduler simulator with Gantt charts & latency metrics\n\n");

    printf(COLOR_YELLOW "CO-3: Inter-Process Communication (IPC), Signals & Job Control\n" COLOR_RESET);
    printf("  cmd1 | cmd2 | ...          : Multi-stage anonymous pipelines with automatic fd closure\n");
    printf("  fifowrite <path> <msg>     : Named Pipe (FIFO) persistent filesystem writer (O_NONBLOCK)\n");
    printf("  fiforead <path>            : Named Pipe (FIFO) persistent filesystem reader\n");
    printf("  socket-daemon              : UNIX domain socket server listening on /tmp/architect_os.sock\n");
    printf("  socket-client [msg]        : UNIX domain socket client sending payload to daemon\n");
    printf("  jobs                       : Display active background tasks and stopped jobs\n");
    printf("  fg [job_id]                : Foreground job control with tcsetpgrp() and terminal switching\n");
    printf("  bg [job_id]                : Background continue stopped job with SIGCONT\n\n");

    printf(COLOR_YELLOW "CO-4: Virtual Memory Management & Memory Introspection\n" COLOR_RESET);
    printf("  vmmaps [pid]               : Virtual memory segment inspector (/proc/[pid]/maps layout)\n");
    printf("  cowdemo                    : Copy-on-Write memory demonstrator using fork & pagemap inspection\n");
    printf("  arch-alloc <bytes>         : Custom heap allocator (mmap-backed segregated free-list)\n");
    printf("  arch-free <hex_ptr>        : Free memory block and demonstrate contiguous coalescing\n");
    printf("  heapinfo                   : Detailed heap fragmentation, block table, and memory telemetry\n\n");

    printf(COLOR_YELLOW "CO-5: Linux Virtual File System (VFS), Inodes & Redirection\n" COLOR_RESET);
    printf("  <, >, >>, 2>, 2>&1         : Stream redirection operators with dup2() descriptor wiring\n");
    printf("  inodeinfo <path>           : Comprehensive VFS inode metadata inspector (stat/lstat)\n");
    printf("  mmapio <file> [find] [rep] : Memory-mapped file I/O with zero-copy microsecond benchmarking\n\n");

    printf(COLOR_YELLOW "CO-6: Concurrency, POSIX Threads & Synchronization Hazards\n" COLOR_RESET);
    printf("  threads-demo <thrd> <iter> [sync|nosync] : Unsynchronized data race vs pthread_mutex_t\n");
    printf("  pc-demo <items> <cap>      : Producer-Consumer bounded buffer queue with semaphores & mutex\n");
    printf("  deadlock-demo [hazard|prevent]          : Circular wait hazard vs strict lock ordering resolution\n\n");

    printf(COLOR_YELLOW "Standard Shell Utilities:\n" COLOR_RESET);
    printf("  cd <path>, clear, exit/quit\n");
    printf("==================================================================================\n\n");
}

int execute_builtin(SimpleCommand *cmd, InodeDelta *delta) {
    if (!cmd || cmd->argc == 0) return 0;
    char *name = cmd->argv[0];

    if (strcmp(name, "help") == 0) {
        print_shell_help();
        return 0;
    }
    if (strcmp(name, "exit") == 0 || strcmp(name, "quit") == 0) {
        printf(COLOR_CYAN "Shutting down Architect OS. Goodbye!\n" COLOR_RESET);
        g_exit_requested = true;
        return 0;
    }
    if (strcmp(name, "clear") == 0) {
        printf("\033[H\033[J");
        return 0;
    }
    if (strcmp(name, "cd") == 0) {
        const char *dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
        if (!dir) dir = "/";
        if (chdir(dir) < 0) {
            fprintf(stderr, COLOR_RED "cd: %s: %s\n" COLOR_RESET, dir, strerror(errno));
            return 1;
        }
        return 0;
    }
    if (strcmp(name, "mode") == 0) {
        if (cmd->argc > 1) {
            if (strcasecmp(cmd->argv[1], "advanced") == 0 || strcasecmp(cmd->argv[1], "adv") == 0) {
                g_visualizer_mode = MODE_ADVANCED;
                g_visualize_enabled = true;
                printf(COLOR_GREEN "Execution Mode set to: ADVANCED (6-Step Deep Telemetry Dashboard)\n" COLOR_RESET);
            } else if (strcasecmp(cmd->argv[1], "summary") == 0 || strcasecmp(cmd->argv[1], "sum") == 0 || strcasecmp(cmd->argv[1], "simple") == 0) {
                g_visualizer_mode = MODE_SUMMARY;
                g_visualize_enabled = true;
                printf(COLOR_GREEN "Execution Mode set to: SUMMARY (Plain-English Command Explanations)\n" COLOR_RESET);
            } else if (strcasecmp(cmd->argv[1], "off") == 0 || strcasecmp(cmd->argv[1], "none") == 0) {
                g_visualizer_mode = MODE_OFF;
                g_visualize_enabled = false;
                printf(COLOR_YELLOW "Execution Mode set to: OFF (Traditional Silent Shell Execution)\n" COLOR_RESET);
            } else {
                printf(COLOR_RED "Unknown mode '%s'. Options: 'advanced', 'summary', 'off'\n" COLOR_RESET, cmd->argv[1]);
                return 1;
            }
            return 0;
        }

        const char *curr_str = (g_visualizer_mode == MODE_ADVANCED) ? "ADVANCED (Deep 6-Step Visualizer)" :
                               (g_visualizer_mode == MODE_SUMMARY)  ? "SUMMARY (Concise Plain-English Explanations)" :
                                                                      "OFF (Traditional Silent Execution)";
        printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Execution Telemetry Modes ===" COLOR_RESET "\n");
        printf("  Current Active Mode : " COLOR_GREEN "%s" COLOR_RESET "\n\n", curr_str);
        printf("  Toggle Commands:\n");
        printf("    " COLOR_YELLOW "mode summary" COLOR_RESET "   - Concise plain-English summary of what each command does\n");
        printf("    " COLOR_YELLOW "mode advanced" COLOR_RESET "  - Full 6-step AST, Ring 3->0 trap, PCB, VMM, VFS & IPC dashboard\n");
        printf("    " COLOR_YELLOW "mode off" COLOR_RESET "       - Disable all telemetry for standard silent execution\n\n");
        return 0;
    }
    if (strcmp(name, "summary") == 0) {
        if (cmd->argc > 1) {
            if (strcasecmp(cmd->argv[1], "off") == 0) {
                g_visualizer_mode = MODE_OFF;
                g_visualize_enabled = false;
            } else {
                g_visualizer_mode = MODE_SUMMARY;
                g_visualize_enabled = true;
            }
        } else {
            g_visualizer_mode = (g_visualizer_mode == MODE_SUMMARY) ? MODE_ADVANCED : MODE_SUMMARY;
            g_visualize_enabled = true;
        }
        printf(COLOR_GREEN "Command Summary Mode is now %s.\n" COLOR_RESET,
               (g_visualizer_mode == MODE_SUMMARY) ? "ON (Plain-English Summary Mode)" : "OFF (Switched to Advanced/Off)");
        return 0;
    }
    if (strcmp(name, "visualize") == 0 || strcmp(name, "visualizer") == 0) {
        if (cmd->argc > 1) {
            if (strcasecmp(cmd->argv[1], "summary") == 0 || strcasecmp(cmd->argv[1], "sum") == 0) {
                g_visualizer_mode = MODE_SUMMARY;
                g_visualize_enabled = true;
            } else if (strcasecmp(cmd->argv[1], "on") == 0 || strcasecmp(cmd->argv[1], "advanced") == 0) {
                g_visualizer_mode = MODE_ADVANCED;
                g_visualize_enabled = true;
            } else if (strcasecmp(cmd->argv[1], "off") == 0) {
                g_visualizer_mode = MODE_OFF;
                g_visualize_enabled = false;
            }
        } else {
            if (g_visualizer_mode == MODE_ADVANCED) {
                g_visualizer_mode = MODE_SUMMARY;
            } else if (g_visualizer_mode == MODE_SUMMARY) {
                g_visualizer_mode = MODE_OFF;
                g_visualize_enabled = false;
            } else {
                g_visualizer_mode = MODE_ADVANCED;
                g_visualize_enabled = true;
            }
        }
        const char *m_desc = (g_visualizer_mode == MODE_ADVANCED) ? "ADVANCED (6-Step Dashboard)" :
                             (g_visualizer_mode == MODE_SUMMARY)  ? "SUMMARY (Plain-English Explanations)" : "OFF";
        printf(COLOR_GREEN "Command Visualizer Mode is now %s.\n" COLOR_RESET, m_desc);
        return 0;
    }
    if (strcmp(name, "history") == 0) {
        history_print();
        return 0;
    }

    /* Subsystem Builtins */
    if (strcmp(name, "jobs") == 0) return cmd_jobs((int)cmd->argc, cmd->argv);
    if (strcmp(name, "fg") == 0) return cmd_fg((int)cmd->argc, cmd->argv);
    if (strcmp(name, "bg") == 0) return cmd_bg((int)cmd->argc, cmd->argv);
    if (strcmp(name, "sysinspect") == 0) return cmd_sysinspect((int)cmd->argc, cmd->argv);
    if (strcmp(name, "procstate") == 0) return cmd_procstate((int)cmd->argc, cmd->argv);
    if (strcmp(name, "zombiedemo") == 0) return cmd_zombiedemo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "orphandemo") == 0) return cmd_orphandemo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "scheduler-sim") == 0) return cmd_scheduler_sim((int)cmd->argc, cmd->argv);
    if (strcmp(name, "fifowrite") == 0) return cmd_fifowrite((int)cmd->argc, cmd->argv);
    if (strcmp(name, "fiforead") == 0) return cmd_fiforead((int)cmd->argc, cmd->argv);
    if (strcmp(name, "socket-daemon") == 0) return cmd_socket_daemon((int)cmd->argc, cmd->argv);
    if (strcmp(name, "socket-client") == 0) return cmd_socket_client((int)cmd->argc, cmd->argv);
    if (strcmp(name, "vmmaps") == 0) return cmd_vmmaps((int)cmd->argc, cmd->argv);
    if (strcmp(name, "cowdemo") == 0) return cmd_cowdemo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "arch-alloc") == 0) return cmd_arch_alloc((int)cmd->argc, cmd->argv);
    if (strcmp(name, "arch-free") == 0) return cmd_arch_free((int)cmd->argc, cmd->argv);
    if (strcmp(name, "heapinfo") == 0) return cmd_heapinfo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "inodeinfo") == 0) return cmd_inodeinfo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "mmapio") == 0) return cmd_mmapio((int)cmd->argc, cmd->argv);
    if (strcmp(name, "threads-demo") == 0) return cmd_threads_demo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "pc-demo") == 0) return cmd_pc_demo((int)cmd->argc, cmd->argv);
    if (strcmp(name, "deadlock-demo") == 0) return cmd_deadlock_demo((int)cmd->argc, cmd->argv);

    (void)delta;
    return -1;
}

/* --- Stream Redirection Setup Helper --- */
static int apply_redirections(SimpleCommand *cmd) {
    for (size_t i = 0; i < cmd->redir_count; i++) {
        Redirection *r = &cmd->redirs[i];
        int fd = -1;

        switch (r->type) {
            case REDIR_IN:
                fd = open(r->target, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, COLOR_RED "Cannot open %s for reading: %s\n" COLOR_RESET,
                            r->target, strerror(errno));
                    return -1;
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                break;

            case REDIR_OUT:
                fd = open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0664);
                if (fd < 0) {
                    fprintf(stderr, COLOR_RED "Cannot open %s for writing: %s\n" COLOR_RESET,
                            r->target, strerror(errno));
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;

            case REDIR_APPEND:
                fd = open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0664);
                if (fd < 0) {
                    fprintf(stderr, COLOR_RED "Cannot open %s for append: %s\n" COLOR_RESET,
                            r->target, strerror(errno));
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;

            case REDIR_ERR:
                fd = open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0664);
                if (fd < 0) {
                    fprintf(stderr, COLOR_RED "Cannot open %s for stderr: %s\n" COLOR_RESET,
                            r->target, strerror(errno));
                    return -1;
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;

            case REDIR_ERR_OUT:
                dup2(STDOUT_FILENO, STDERR_FILENO);
                break;

            default:
                break;
        }
    }
    return 0;
}

/* --- Main Pipeline Executor --- */
int execute_pipeline(Pipeline *p, InodeDelta *delta) {
    if (!p || p->cmd_count == 0) return 0;

    if (g_visualizer_mode == MODE_SUMMARY) {
        visualizer_render_summary(p);
    } else if (g_visualizer_mode == MODE_ADVANCED) {
        visualizer_step1_ast(p);
    }

    /* Single Built-in executed in parent process if no pipeline and not background */
    if (p->cmd_count == 1 && is_builtin_command(p->commands[0]->argv[0]) && !p->is_background) {
        SimpleCommand *cmd = p->commands[0];

        /* If redirections are present, save original fds */
        int saved_in = -1, saved_out = -1, saved_err = -1;
        if (cmd->redir_count > 0) {
            saved_in = dup(STDIN_FILENO);
            saved_out = dup(STDOUT_FILENO);
            saved_err = dup(STDERR_FILENO);
            if (apply_redirections(cmd) < 0) {
                if (saved_in != -1) { dup2(saved_in, STDIN_FILENO); close(saved_in); }
                if (saved_out != -1) { dup2(saved_out, STDOUT_FILENO); close(saved_out); }
                if (saved_err != -1) { dup2(saved_err, STDERR_FILENO); close(saved_err); }
                return 1;
            }
        }

        visualizer_step2_modeswitch(cmd->argv[0]);
        visualizer_step3_pcb(getpid(), getpid(), cmd->argv[0]);
        visualizer_step4_vmm(getpid());

        int ret = execute_builtin(cmd, delta);

        if (cmd->redir_count > 0) {
            if (saved_in != -1) { dup2(saved_in, STDIN_FILENO); close(saved_in); }
            if (saved_out != -1) { dup2(saved_out, STDOUT_FILENO); close(saved_out); }
            if (saved_err != -1) { dup2(saved_err, STDERR_FILENO); close(saved_err); }
        }

        if (delta && delta->target_path[0] != '\0') {
            *delta = vfs_calculate_delta(&delta->before, delta->target_path);
        }
        visualizer_step5_vfs(delta);
        visualizer_step6_ipc_exit(ret, 0, false, "Parent shell context execution");
        return ret;
    }

    /* Multi-stage Pipeline or External Command Execution */
    size_t n = p->cmd_count;
    int (*pipes)[2] = NULL;
    if (n > 1) {
        pipes = malloc((n - 1) * sizeof(int[2]));
        for (size_t i = 0; i < n - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                free(pipes);
                return 1;
            }
        }
    }

    pid_t *pids = malloc(n * sizeof(pid_t));
    pid_t pgid = 0;

    visualizer_step2_modeswitch(p->commands[0]->argv[0]);

    for (size_t i = 0; i < n; i++) {
        SimpleCommand *cmd = p->commands[i];

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            if (pipes) free(pipes);
            return 1;
        }

        if (pid == 0) {
            /* Child Process */
            pid_t my_pid = getpid();
            if (i == 0) pgid = my_pid;
            setpgid(my_pid, pgid);

            if (!p->is_background && g_job_manager.is_interactive) {
                tcsetpgrp(g_job_manager.shell_terminal, pgid);
            }

            /* Reset signal handlers to default */
            struct sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sa, NULL);
            sigaction(SIGTSTP, &sa, NULL);
            sigaction(SIGTTIN, &sa, NULL);
            sigaction(SIGTTOU, &sa, NULL);

            /* Pipe Input wiring */
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            /* Pipe Output wiring */
            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            /* Close all pipe ends in child */
            if (pipes) {
                for (size_t j = 0; j < n - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            /* Apply file redirections */
            if (apply_redirections(cmd) < 0) {
                _exit(EXIT_FAILURE);
            }

            /* Check if builtin in child */
            if (is_builtin_command(cmd->argv[0])) {
                int b_ret = execute_builtin(cmd, delta);
                _exit(b_ret);
            }

            /* Execute external binary */
            execvp(cmd->argv[0], cmd->argv);

            fprintf(stderr, COLOR_RED "architect: %s: command not found\n" COLOR_RESET, cmd->argv[0]);
            _exit(127);
        } else {
            /* Parent Process */
            pids[i] = pid;
            if (i == 0) pgid = pid;
            setpgid(pid, pgid);
        }
    }

    visualizer_step3_pcb(getpid(), pids[0], p->commands[0]->argv[0]);
    visualizer_step4_vmm(pids[0]);

    /* Parent closes all pipe ends */
    if (pipes) {
        for (size_t j = 0; j < n - 1; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        free(pipes);
    }

    int last_exit_code = 0;
    int last_term_sig = 0;
    bool job_stopped = false;

    if (p->is_background) {
        Job *j = jobs_add(pgid, pids[0], JOB_RUNNING, p->raw_cmdline);
        if (j) {
            printf(COLOR_GREEN "[%d] %d (Running in background)\n" COLOR_RESET, j->job_id, pgid);
        }
        visualizer_step5_vfs(delta);
        visualizer_step6_ipc_exit(0, 0, false, "Background task detached; SIGCHLD asynchronous reaper armed");
    } else {
        if (g_job_manager.is_interactive) {
            tcsetpgrp(g_job_manager.shell_terminal, pgid);
        }

        for (size_t i = 0; i < n; i++) {
            int status = 0;
            pid_t w = waitpid(pids[i], &status, WUNTRACED);
            if (w > 0) {
                if (WIFEXITED(status)) {
                    last_exit_code = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    last_term_sig = WTERMSIG(status);
                } else if (WIFSTOPPED(status)) {
                    job_stopped = true;
                    last_term_sig = WSTOPSIG(status);
                }
            }
        }

        if (g_job_manager.is_interactive) {
            tcsetpgrp(g_job_manager.shell_terminal, g_job_manager.shell_pgid);
            tcsetattr(g_job_manager.shell_terminal, TCSADRAIN, &g_job_manager.shell_tmodes);
        }

        if (job_stopped) {
            Job *j = jobs_add(pgid, pids[0], JOB_STOPPED, p->raw_cmdline);
            if (j) {
                printf(COLOR_YELLOW "\n[%d]+ Stopped (SIGTSTP)\t%s\n" COLOR_RESET, j->job_id, p->raw_cmdline);
            }
        }

        if (delta && delta->target_path[0] != '\0') {
            *delta = vfs_calculate_delta(&delta->before, delta->target_path);
        }
        visualizer_step5_vfs(delta);

        char ipc_buf[256];
        if (n > 1) {
            snprintf(ipc_buf, sizeof(ipc_buf),
                     "Anonymous Pipeline: %zu processes linked via %zu kernel pipes (dup2 redirected)",
                     n, n - 1);
        } else {
            snprintf(ipc_buf, sizeof(ipc_buf), "Standard POSIX streams (0, 1, 2) synchronized");
        }

        visualizer_step6_ipc_exit(last_exit_code, last_term_sig, job_stopped, ipc_buf);
    }

    free(pids);
    return last_exit_code;
}
