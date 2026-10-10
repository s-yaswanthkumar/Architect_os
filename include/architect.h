#ifndef ARCHITECT_H
#define ARCHITECT_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>

/* ANSI Terminal Colors */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_DIM     "\033[2m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

#define COLOR_BG_BLUE "\033[44m"
#define COLOR_BG_DARK "\033[40m"

#define MAX_LINE_LEN 4096
#define MAX_HISTORY  100
#define SOCKET_PATH  "/tmp/architect_os.sock"

/* --- CO-1 & CO-5: Dynamic Vector & Lexer Types --- */
typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Vec;

typedef enum {
    REDIR_NONE = 0,
    REDIR_IN,       /* <  (stdin) */
    REDIR_OUT,      /* >  (stdout truncate) */
    REDIR_APPEND,   /* >> (stdout append) */
    REDIR_ERR,      /* 2> (stderr) */
    REDIR_ERR_OUT   /* 2>&1 (stderr to stdout) */
} RedirType;

typedef struct {
    RedirType type;
    char *target;   /* filename or target fd string */
    int fd_src;     /* source file descriptor (0, 1, 2) */
} Redirection;

typedef struct {
    char **argv;
    size_t argc;
    Redirection *redirs;
    size_t redir_count;
    size_t redir_cap;
} SimpleCommand;

typedef struct {
    SimpleCommand **commands;
    size_t cmd_count;
    bool is_background; /* Trailing '&' */
    char *raw_cmdline;
} Pipeline;

/* --- CO-2 & CO-3: Job Control & Process Lifecycle --- */
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;

typedef struct Job {
    int job_id;
    pid_t pgid;
    pid_t pid;
    JobState state;
    char *cmdline;
    struct termios tmodes;
    struct Job *next;
} Job;

typedef struct {
    Job *head;
    int next_id;
    pid_t shell_pgid;
    struct termios shell_tmodes;
    int shell_terminal;
    int is_interactive;
} JobManager;

/* --- CO-5: VFS & Inode Differential Telemetry --- */
typedef struct {
    ino_t inode;
    off_t size;
    time_t mtime;
    nlink_t nlink;
    mode_t mode;
    char path[512];
    bool exists;
} FileSnapshot;

typedef struct {
    FileSnapshot before;
    FileSnapshot after;
    char target_path[512];
    char summary[512];
} InodeDelta;

/* --- Visualizer Modes & Telemetry --- */
typedef enum {
    MODE_OFF = 0,
    MODE_ADVANCED = 1,
    MODE_SUMMARY = 2
} VisualizerMode;

extern VisualizerMode g_visualizer_mode;
extern bool g_visualize_enabled;
extern bool g_exit_requested;
extern JobManager g_job_manager;

void visualizer_render_summary(const Pipeline *pipeline);
void visualizer_step1_ast(const Pipeline *pipeline);
void visualizer_step2_modeswitch(const char *cmd_name);
void visualizer_step3_pcb(pid_t parent_pid, pid_t child_pid, const char *cmd_name);
void visualizer_step4_vmm(pid_t pid);
void visualizer_step5_vfs(const InodeDelta *delta);
void visualizer_step6_ipc_exit(int exit_code, int term_sig, bool stopped, const char *ipc_details);

/* --- Parser Prototypes --- */
Vec *vec_new(void);
void vec_push(Vec *v, char *item);
void vec_free(Vec *v);
Pipeline *parse_command_line(const char *line);
void free_pipeline(Pipeline *p);

/* --- Executor & Builtins Prototypes --- */
int execute_pipeline(Pipeline *p, InodeDelta *delta);
bool is_builtin_command(const char *name);
int execute_builtin(SimpleCommand *cmd, InodeDelta *delta);

/* --- Job Control Prototypes --- */
void jobs_init(void);
void jobs_cleanup(void);
Job *jobs_add(pid_t pgid, pid_t pid, JobState state, const char *cmdline);
Job *jobs_find_by_id(int job_id);
Job *jobs_find_by_pgid(pid_t pgid);
void jobs_remove(int job_id);
void jobs_print(void);
void jobs_check_completed(void);
int cmd_jobs(int argc, char **argv);
int cmd_fg(int argc, char **argv);
int cmd_bg(int argc, char **argv);

/* --- CO-1: Syscall Introspection & Services --- */
int cmd_sysinspect(int argc, char **argv);

/* --- CO-2: Process Control & Scheduler --- */
int cmd_procstate(int argc, char **argv);
int cmd_zombiedemo(int argc, char **argv);
int cmd_orphandemo(int argc, char **argv);
int cmd_scheduler_sim(int argc, char **argv);

/* --- CO-3: IPC, FIFOs, Sockets --- */
int cmd_fifowrite(int argc, char **argv);
int cmd_fiforead(int argc, char **argv);
int cmd_socket_daemon(int argc, char **argv);
int cmd_socket_client(int argc, char **argv);

/* --- CO-4: Virtual Memory Management --- */
int cmd_vmmaps(int argc, char **argv);
int cmd_cowdemo(int argc, char **argv);
void *arch_malloc(size_t size);
void arch_free(void *ptr);
int cmd_arch_alloc(int argc, char **argv);
int cmd_arch_free(int argc, char **argv);
int cmd_heapinfo(int argc, char **argv);
void arch_allocator_cleanup(void);

/* --- CO-5: VFS, Inodes, Stream & mmap I/O --- */
int cmd_inodeinfo(int argc, char **argv);
int cmd_mmapio(int argc, char **argv);
FileSnapshot vfs_take_snapshot(const char *path);
InodeDelta vfs_calculate_delta(const FileSnapshot *before, const char *path);

/* --- CO-6: Concurrency & Synchronization --- */
int cmd_threads_demo(int argc, char **argv);
int cmd_pc_demo(int argc, char **argv);
int cmd_deadlock_demo(int argc, char **argv);

/* --- Shell Core & History --- */
void history_add(const char *line);
void history_print(void);
void history_cleanup(void);

#endif /* ARCHITECT_H */
