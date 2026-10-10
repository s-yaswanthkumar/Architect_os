#include "architect.h"

JobManager g_job_manager = {
    .head = NULL,
    .next_id = 1,
    .shell_pgid = 0,
    .shell_terminal = STDIN_FILENO,
    .is_interactive = 0
};

void jobs_init(void) {
    g_job_manager.shell_terminal = STDIN_FILENO;
    g_job_manager.is_interactive = isatty(g_job_manager.shell_terminal);

    if (g_job_manager.is_interactive) {
        /* Loop until we are in the foreground */
        while (tcgetpgrp(g_job_manager.shell_terminal) != (g_job_manager.shell_pgid = getpgrp())) {
            kill(-g_job_manager.shell_pgid, SIGTTIN);
        }

        /* Ignore interactive and job-control signals in the parent shell */
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_IGN;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTSTP, &sa, NULL);
        sigaction(SIGTTIN, &sa, NULL);
        sigaction(SIGTTOU, &sa, NULL);

        /* Put ourselves into our own process group */
        g_job_manager.shell_pgid = getpid();
        if (setpgid(g_job_manager.shell_pgid, g_job_manager.shell_pgid) < 0) {
            /* If already a process group leader, this can return EPERM which is fine */
        }

        /* Grab control of the terminal */
        tcsetpgrp(g_job_manager.shell_terminal, g_job_manager.shell_pgid);

        /* Save default terminal attributes */
        tcgetattr(g_job_manager.shell_terminal, &g_job_manager.shell_tmodes);
    }
}

void jobs_cleanup(void) {
    Job *curr = g_job_manager.head;
    while (curr) {
        Job *next = curr->next;
        if (curr->cmdline) free(curr->cmdline);
        free(curr);
        curr = next;
    }
    g_job_manager.head = NULL;
}

Job *jobs_add(pid_t pgid, pid_t pid, JobState state, const char *cmdline) {
    Job *j = calloc(1, sizeof(Job));
    if (!j) return NULL;

    j->job_id = g_job_manager.next_id++;
    j->pgid = pgid;
    j->pid = pid;
    j->state = state;
    j->cmdline = strdup(cmdline ? cmdline : "unknown");
    j->next = g_job_manager.head;
    g_job_manager.head = j;

    return j;
}

Job *jobs_find_by_id(int job_id) {
    Job *curr = g_job_manager.head;
    while (curr) {
        if (curr->job_id == job_id) return curr;
        curr = curr->next;
    }
    return NULL;
}

Job *jobs_find_by_pgid(pid_t pgid) {
    Job *curr = g_job_manager.head;
    while (curr) {
        if (curr->pgid == pgid) return curr;
        curr = curr->next;
    }
    return NULL;
}

void jobs_remove(int job_id) {
    Job **curr = &g_job_manager.head;
    while (*curr) {
        if ((*curr)->job_id == job_id) {
            Job *to_free = *curr;
            *curr = (*curr)->next;
            if (to_free->cmdline) free(to_free->cmdline);
            free(to_free);
            return;
        }
        curr = &(*curr)->next;
    }
}

void jobs_print(void) {
    Job *curr = g_job_manager.head;
    if (!curr) {
        printf(COLOR_DIM "No active jobs in session.\n" COLOR_RESET);
        return;
    }

    printf(COLOR_BOLD "%-6s %-8s %-8s %-16s %-30s\n" COLOR_RESET,
           "JOB ID", "PGID", "PID", "STATE", "COMMAND");
    printf("----------------------------------------------------------------------\n");

    while (curr) {
        const char *state_str = "RUNNING";
        const char *color = COLOR_GREEN;
        if (curr->state == JOB_STOPPED) {
            state_str = "STOPPED";
            color = COLOR_YELLOW;
        } else if (curr->state == JOB_DONE) {
            state_str = "DONE";
            color = COLOR_BLUE;
        }

        printf("[%d]%c    %-8d %-8d %s%-16s" COLOR_RESET " %-30s\n",
               curr->job_id,
               (curr == g_job_manager.head) ? '+' : '-',
               curr->pgid,
               curr->pid,
               color,
               state_str,
               curr->cmdline);
        curr = curr->next;
    }
}

/* Asynchronous/Periodic background reaper for completed jobs */
void jobs_check_completed(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        Job *j = NULL;
        for (Job *curr = g_job_manager.head; curr; curr = curr->next) {
            if (curr->pid == pid || curr->pgid == pid) {
                j = curr;
                break;
            }
        }

        if (!j) continue;

        if (WIFEXITED(status)) {
            printf("\n" COLOR_GREEN "[%d]+ Done (exit %d)\t%s\n" COLOR_RESET,
                   j->job_id, WEXITSTATUS(status), j->cmdline);
            jobs_remove(j->job_id);
        } else if (WIFSIGNALED(status)) {
            printf("\n" COLOR_RED "[%d]+ Terminated by signal %d\t%s\n" COLOR_RESET,
                   j->job_id, WTERMSIG(status), j->cmdline);
            jobs_remove(j->job_id);
        } else if (WIFSTOPPED(status)) {
            j->state = JOB_STOPPED;
            printf("\n" COLOR_YELLOW "[%d]+ Stopped (signal %d)\t%s\n" COLOR_RESET,
                   j->job_id, WSTOPSIG(status), j->cmdline);
        } else if (WIFCONTINUED(status)) {
            j->state = JOB_RUNNING;
            printf("\n" COLOR_GREEN "[%d]+ Continued\t%s\n" COLOR_RESET,
                   j->job_id, j->cmdline);
        }
    }
}

int cmd_jobs(int argc, char **argv) {
    (void)argc;
    (void)argv;
    jobs_check_completed();
    jobs_print();
    return 0;
}

int cmd_fg(int argc, char **argv) {
    Job *j = NULL;
    if (argc > 1) {
        int jid = atoi(argv[1]);
        j = jobs_find_by_id(jid);
    } else {
        j = g_job_manager.head;
    }

    if (!j) {
        fprintf(stderr, COLOR_RED "fg: job not found\n" COLOR_RESET);
        return 1;
    }

    printf(COLOR_CYAN "%s\n" COLOR_RESET, j->cmdline);

    if (g_job_manager.is_interactive) {
        /* Hand over terminal control to the job's process group */
        tcsetpgrp(g_job_manager.shell_terminal, j->pgid);
    }

    if (j->state == JOB_STOPPED) {
        if (kill(-j->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
        j->state = JOB_RUNNING;
    }

    int status = 0;
    pid_t waited_pid = waitpid(-j->pgid, &status, WUNTRACED);

    if (g_job_manager.is_interactive) {
        /* Reclaim terminal control */
        tcsetpgrp(g_job_manager.shell_terminal, g_job_manager.shell_pgid);
        tcsetattr(g_job_manager.shell_terminal, TCSADRAIN, &g_job_manager.shell_tmodes);
    }

    if (waited_pid > 0) {
        if (WIFSTOPPED(status)) {
            j->state = JOB_STOPPED;
            printf(COLOR_YELLOW "\n[%d]+ Stopped (SIGTSTP)\t%s\n" COLOR_RESET,
                   j->job_id, j->cmdline);
        } else {
            jobs_remove(j->job_id);
        }
    }

    return 0;
}

int cmd_bg(int argc, char **argv) {
    Job *j = NULL;
    if (argc > 1) {
        int jid = atoi(argv[1]);
        j = jobs_find_by_id(jid);
    } else {
        j = g_job_manager.head;
    }

    if (!j) {
        fprintf(stderr, COLOR_RED "bg: job not found\n" COLOR_RESET);
        return 1;
    }

    if (j->state == JOB_RUNNING) {
        printf(COLOR_YELLOW "bg: job %d already in background\n" COLOR_RESET, j->job_id);
        return 0;
    }

    if (kill(-j->pgid, SIGCONT) < 0) {
        perror("kill (SIGCONT)");
        return 1;
    }

    j->state = JOB_RUNNING;
    printf(COLOR_GREEN "[%d]+ %s &\n" COLOR_RESET, j->job_id, j->cmdline);
    return 0;
}
