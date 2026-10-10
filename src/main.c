#include "architect.h"

/* --- Command History Subsystem --- */
static char *g_history[MAX_HISTORY];
static size_t g_history_count = 0;

void history_add(const char *line) {
    if (!line || !*line) return;
    if (g_history_count < MAX_HISTORY) {
        g_history[g_history_count++] = strdup(line);
    } else {
        free(g_history[0]);
        for (size_t i = 1; i < MAX_HISTORY; i++) {
            g_history[i - 1] = g_history[i];
        }
        g_history[MAX_HISTORY - 1] = strdup(line);
    }
}

void history_print(void) {
    printf("\n" COLOR_BOLD COLOR_CYAN "=== Architect OS Command History ===" COLOR_RESET "\n");
    for (size_t i = 0; i < g_history_count; i++) {
        printf("  %3zu  %s\n", i + 1, g_history[i]);
    }
    printf("\n");
}

void history_cleanup(void) {
    for (size_t i = 0; i < g_history_count; i++) {
        free(g_history[i]);
        g_history[i] = NULL;
    }
    g_history_count = 0;
}

/* --- Signal Handling Architecture --- */
static volatile sig_atomic_t g_sigchld_received = 0;

static void sigchld_handler(int sig) {
    (void)sig;
    g_sigchld_received = 1;
}

void init_signals(void) {
    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    /* The interactive parent shell ignores job control signals */
    struct sigaction sa_ign;
    memset(&sa_ign, 0, sizeof(sa_ign));
    sa_ign.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sa_ign, NULL);
    sigaction(SIGTSTP, &sa_ign, NULL);
    sigaction(SIGQUIT, &sa_ign, NULL);
}

/* --- Interactive Line Reader with Backspace Handling --- */
static char *read_line_interactive(void) {
    char *buf = malloc(MAX_LINE_LEN);
    if (!buf) return NULL;

    size_t pos = 0;
    int c;

    /* Use standard getline if stdin is not a TTY (piped or automated test) */
    if (!isatty(STDIN_FILENO)) {
        size_t n = 0;
        char *line = NULL;
        ssize_t read_bytes = getline(&line, &n, stdin);
        if (read_bytes < 0) {
            free(buf);
            if (line) free(line);
            return NULL;
        }
        line[strcspn(line, "\r\n")] = '\0';
        strncpy(buf, line, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';
        free(line);
        return buf;
    }

    while (1) {
        c = getchar();
        if (c == EOF) {
            if (pos == 0) {
                free(buf);
                return NULL;
            }
            break;
        }

        if (c == '\n') {
            break;
        } else if (c == '\b' || c == 127) {
            /* Handle backspace */
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (pos + 1 < MAX_LINE_LEN) {
            buf[pos++] = (char)c;
        }
    }

    buf[pos] = '\0';
    return buf;
}

static void print_prompt(void) {
    char cwd[512];
    if (!getcwd(cwd, sizeof(cwd))) {
        strncpy(cwd, "unknown", sizeof(cwd));
    }

    /* Replace /home/username with ~ for clean appearance */
    const char *home = getenv("HOME");
    char display_cwd[512];
    if (home && strstr(cwd, home) == cwd) {
        snprintf(display_cwd, sizeof(display_cwd), "~%s", cwd + strlen(home));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd) - 1);
        display_cwd[sizeof(display_cwd) - 1] = '\0';
    }

    printf(COLOR_BOLD COLOR_CYAN "architect-os" COLOR_RESET ":"
           COLOR_BOLD COLOR_BLUE "%s" COLOR_RESET "$ ", display_cwd);
    fflush(stdout);
}

/* Helper to detect file target from pipeline for VFS delta tracking */
static const char *detect_target_file(const Pipeline *p) {
    if (!p) return NULL;
    for (size_t i = 0; i < p->cmd_count; i++) {
        SimpleCommand *cmd = p->commands[i];
        for (size_t r = 0; r < cmd->redir_count; r++) {
            if (cmd->redirs[r].type == REDIR_OUT || cmd->redirs[r].type == REDIR_APPEND) {
                return cmd->redirs[r].target;
            }
        }
        /* Check if command creates or touches a file */
        if (cmd->argc > 1 && (strcmp(cmd->argv[0], "touch") == 0 ||
                              strcmp(cmd->argv[0], "nano") == 0 ||
                              strcmp(cmd->argv[0], "vim") == 0 ||
                              strcmp(cmd->argv[0], "mmapio") == 0)) {
            return cmd->argv[1];
        }
    }
    return NULL;
}

static int execute_compound_command(const char *cmdline) {
    if (!cmdline || !*cmdline) return 0;

    char *copy = strdup(cmdline);
    if (!copy) return 0;

    size_t len = strlen(copy);
    bool in_single = false, in_double = false;
    char *start = copy;
    int last_ret = 0;

    for (size_t i = 0; i <= len; i++) {
        char c = copy[i];
        if (c == '\'' && !in_double) in_single = !in_single;
        else if (c == '"' && !in_single) in_double = !in_double;
        else if ((c == ';' || c == '\0') && !in_single && !in_double) {
            copy[i] = '\0';
            char *seg = start;
            while (*seg && isspace((unsigned char)*seg)) seg++;
            if (*seg != '\0') {
                Pipeline *p = parse_command_line(seg);
                if (p) {
                    const char *tgt = detect_target_file(p);
                    FileSnapshot before = vfs_take_snapshot(tgt);
                    InodeDelta delta;
                    memset(&delta, 0, sizeof(delta));
                    if (tgt) {
                        strncpy(delta.target_path, tgt, sizeof(delta.target_path) - 1);
                        delta.before = before;
                    }
                    last_ret = execute_pipeline(p, &delta);
                    free_pipeline(p);
                    if (g_exit_requested) {
                        break;
                    }
                }
            }
            start = copy + i + 1;
        }
    }

    free(copy);
    return last_ret;
}

int main(int argc, char **argv) {
    /* Handle standalone daemon and client modes */
    if (argc > 1) {
        if (strcmp(argv[1], "--daemon") == 0) {
            return cmd_socket_daemon(argc, argv);
        } else if (strcmp(argv[1], "--client") == 0) {
            char *msg = (argc > 2) ? argv[2] : "Architect OS IPC Daemon Test Payload";
            char *client_args[] = { "socket-client", msg, NULL };
            return cmd_socket_client(2, client_args);
        } else if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            /* Non-interactive command mode */
            jobs_init();
            init_signals();
            int ret = execute_compound_command(argv[2]);
            jobs_cleanup();
            arch_allocator_cleanup();
            return ret;
        } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Architect OS - Systems Programming Shell & Runtime Emulator\n");
            printf("Usage:\n");
            printf("  architect-os               Launch interactive REPL\n");
            printf("  architect-os -c <command>  Execute command string and exit\n");
            printf("  architect-os --daemon      Run UNIX domain socket IPC daemon\n");
            printf("  architect-os --client [msg] Run UNIX domain socket client\n");
            return 0;
        }
    }

    /* Initialize subsystems */
    jobs_init();
    init_signals();

    printf("\n" COLOR_BOLD COLOR_CYAN "==================================================================================\n");
    printf("     ARCHITECT OS: MODULAR C11 SYSTEMS SHELL & OS RUNTIME EMULATOR\n");
    printf("   Course Outcomes (CO-1 through CO-6) Interactive Telemetry Dashboard\n");
    printf("==================================================================================" COLOR_RESET "\n");
    printf("Type " COLOR_YELLOW "'help'" COLOR_RESET " for available course subsystem built-ins.\n");
    printf("Visualizer Mode: " COLOR_GREEN "ADVANCED" COLOR_RESET " (Switch with 'mode summary', 'mode advanced', or 'mode off').\n\n");

    /* REPL Event Loop */
    while (1) {
        /* Check and reap any completed background jobs before prompt */
        if (g_sigchld_received) {
            g_sigchld_received = 0;
            jobs_check_completed();
        }

        print_prompt();

        char *line = read_line_interactive();
        if (!line) {
            /* EOF / Ctrl-D */
            printf("\n" COLOR_CYAN "Session terminated via EOF. Exiting Architect OS...\n" COLOR_RESET);
            break;
        }

        /* Skip empty or whitespace-only lines */
        char *trimmed = line;
        while (*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

        if (*trimmed == '\0') {
            free(line);
            continue;
        }

        history_add(trimmed);
        execute_compound_command(trimmed);
        free(line);
        if (g_exit_requested) {
            break;
        }
    }

    /* Clean subsystem shutdowns */
    jobs_cleanup();
    arch_allocator_cleanup();
    history_cleanup();

    return 0;
}
