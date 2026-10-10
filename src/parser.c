#include "architect.h"

Vec *vec_new(void) {
    Vec *v = malloc(sizeof(Vec));
    if (!v) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    v->count = 0;
    v->capacity = 8;
    v->items = malloc(v->capacity * sizeof(char *));
    if (!v->items) {
        perror("malloc");
        free(v);
        exit(EXIT_FAILURE);
    }
    return v;
}

void vec_push(Vec *v, char *item) {
    if (!v || !item) return;
    if (v->count >= v->capacity) {
        v->capacity *= 2;
        char **new_items = realloc(v->items, v->capacity * sizeof(char *));
        if (!new_items) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        v->items = new_items;
    }
    v->items[v->count++] = item;
}

void vec_free(Vec *v) {
    if (!v) return;
    for (size_t i = 0; i < v->count; i++) {
        free(v->items[i]);
    }
    free(v->items);
    free(v);
}

/* Tokenizer supporting single/double quotes, escapes, and shell metacharacters */
static Vec *tokenize_line(const char *line) {
    Vec *tokens = vec_new();
    size_t len = strlen(line);
    size_t i = 0;

    char buf[MAX_LINE_LEN];
    size_t b_idx = 0;

    while (i < len) {
        /* Skip leading whitespace */
        while (i < len && isspace((unsigned char)line[i])) {
            i++;
        }
        if (i >= len) break;

        b_idx = 0;

        /* Check for multi-character operators first */
        if (line[i] == '2' && i + 3 < len && strncmp(&line[i], "2>&1", 4) == 0) {
            vec_push(tokens, strdup("2>&1"));
            i += 4;
            continue;
        }
        if (line[i] == '2' && i + 1 < len && line[i + 1] == '>') {
            vec_push(tokens, strdup("2>"));
            i += 2;
            continue;
        }
        if (line[i] == '>' && i + 1 < len && line[i + 1] == '>') {
            vec_push(tokens, strdup(">>"));
            i += 2;
            continue;
        }
        if (line[i] == '|' || line[i] == '<' || line[i] == '>' || line[i] == '&') {
            char op_str[2] = { line[i], '\0' };
            vec_push(tokens, strdup(op_str));
            i++;
            continue;
        }

        /* Parse general token */
        bool in_double_quote = false;
        bool in_single_quote = false;

        while (i < len) {
            char c = line[i];

            if (in_single_quote) {
                if (c == '\'') {
                    in_single_quote = false;
                    i++;
                } else {
                    buf[b_idx++] = c;
                    i++;
                }
            } else if (in_double_quote) {
                if (c == '"') {
                    in_double_quote = false;
                    i++;
                } else if (c == '\\' && i + 1 < len) {
                    buf[b_idx++] = line[++i];
                    i++;
                } else {
                    buf[b_idx++] = c;
                    i++;
                }
            } else {
                if (isspace((unsigned char)c)) {
                    break;
                }
                if (c == '\'' ) {
                    in_single_quote = true;
                    i++;
                } else if (c == '"') {
                    in_double_quote = true;
                    i++;
                } else if (c == '\\' && i + 1 < len) {
                    buf[b_idx++] = line[++i];
                    i++;
                } else if (c == '|' || c == '<' || c == '>' || c == '&') {
                    /* Break on delimiter operator */
                    break;
                } else {
                    buf[b_idx++] = c;
                    i++;
                }
            }
        }

        buf[b_idx] = '\0';
        if (b_idx > 0) {
            vec_push(tokens, strdup(buf));
        }
    }

    return tokens;
}

Pipeline *parse_command_line(const char *line) {
    if (!line || !*line) return NULL;

    Vec *tokens = tokenize_line(line);
    if (!tokens || tokens->count == 0) {
        if (tokens) vec_free(tokens);
        return NULL;
    }

    Pipeline *p = calloc(1, sizeof(Pipeline));
    if (!p) {
        vec_free(tokens);
        return NULL;
    }
    p->raw_cmdline = strdup(line);

    /* Check for background execution '&' at the end */
    if (tokens->count > 0 && strcmp(tokens->items[tokens->count - 1], "&") == 0) {
        p->is_background = true;
        free(tokens->items[tokens->count - 1]);
        tokens->count--;
    }

    if (tokens->count == 0) {
        vec_free(tokens);
        free(p->raw_cmdline);
        free(p);
        return NULL;
    }

    /* Allocate initial commands array */
    size_t cmd_cap = 4;
    p->commands = malloc(cmd_cap * sizeof(SimpleCommand *));
    p->cmd_count = 0;

    SimpleCommand *current_cmd = calloc(1, sizeof(SimpleCommand));
    size_t argv_cap = 8;
    current_cmd->argv = malloc(argv_cap * sizeof(char *));
    current_cmd->argc = 0;
    current_cmd->redir_cap = 4;
    current_cmd->redirs = malloc(current_cmd->redir_cap * sizeof(Redirection));
    current_cmd->redir_count = 0;

    for (size_t i = 0; i < tokens->count; i++) {
        char *tok = tokens->items[i];

        if (strcmp(tok, "|") == 0) {
            /* Finalize current command */
            current_cmd->argv[current_cmd->argc] = NULL;
            if (p->cmd_count >= cmd_cap) {
                cmd_cap *= 2;
                p->commands = realloc(p->commands, cmd_cap * sizeof(SimpleCommand *));
            }
            p->commands[p->cmd_count++] = current_cmd;

            /* Prepare next command */
            current_cmd = calloc(1, sizeof(SimpleCommand));
            argv_cap = 8;
            current_cmd->argv = malloc(argv_cap * sizeof(char *));
            current_cmd->argc = 0;
            current_cmd->redir_cap = 4;
            current_cmd->redirs = malloc(current_cmd->redir_cap * sizeof(Redirection));
            current_cmd->redir_count = 0;
            continue;
        }

        /* Check redirection tokens */
        if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 ||
            strcmp(tok, ">>") == 0 || strcmp(tok, "2>") == 0 ||
            strcmp(tok, "2>&1") == 0) {

            if (current_cmd->redir_count >= current_cmd->redir_cap) {
                current_cmd->redir_cap *= 2;
                current_cmd->redirs = realloc(current_cmd->redirs,
                                              current_cmd->redir_cap * sizeof(Redirection));
            }

            Redirection *r = &current_cmd->redirs[current_cmd->redir_count++];
            if (strcmp(tok, "<") == 0) {
                r->type = REDIR_IN;
                r->fd_src = STDIN_FILENO;
                if (i + 1 < tokens->count) {
                    r->target = strdup(tokens->items[++i]);
                } else {
                    r->target = NULL;
                }
            } else if (strcmp(tok, ">") == 0) {
                r->type = REDIR_OUT;
                r->fd_src = STDOUT_FILENO;
                if (i + 1 < tokens->count) {
                    r->target = strdup(tokens->items[++i]);
                } else {
                    r->target = NULL;
                }
            } else if (strcmp(tok, ">>") == 0) {
                r->type = REDIR_APPEND;
                r->fd_src = STDOUT_FILENO;
                if (i + 1 < tokens->count) {
                    r->target = strdup(tokens->items[++i]);
                } else {
                    r->target = NULL;
                }
            } else if (strcmp(tok, "2>") == 0) {
                r->type = REDIR_ERR;
                r->fd_src = STDERR_FILENO;
                if (i + 1 < tokens->count) {
                    r->target = strdup(tokens->items[++i]);
                } else {
                    r->target = NULL;
                }
            } else if (strcmp(tok, "2>&1") == 0) {
                r->type = REDIR_ERR_OUT;
                r->fd_src = STDERR_FILENO;
                r->target = strdup("1");
            }
            continue;
        }

        /* Regular argument */
        if (current_cmd->argc + 1 >= argv_cap) {
            argv_cap *= 2;
            current_cmd->argv = realloc(current_cmd->argv, argv_cap * sizeof(char *));
        }
        current_cmd->argv[current_cmd->argc++] = strdup(tok);
    }

    /* Finalize last command */
    current_cmd->argv[current_cmd->argc] = NULL;
    if (p->cmd_count >= cmd_cap) {
        cmd_cap += 2;
        p->commands = realloc(p->commands, cmd_cap * sizeof(SimpleCommand *));
    }
    p->commands[p->cmd_count++] = current_cmd;

    vec_free(tokens);
    return p;
}

void free_pipeline(Pipeline *p) {
    if (!p) return;
    if (p->commands) {
        for (size_t i = 0; i < p->cmd_count; i++) {
            SimpleCommand *cmd = p->commands[i];
            if (cmd) {
                if (cmd->argv) {
                    for (size_t j = 0; j < cmd->argc; j++) {
                        free(cmd->argv[j]);
                    }
                    free(cmd->argv);
                }
                if (cmd->redirs) {
                    for (size_t j = 0; j < cmd->redir_count; j++) {
                        if (cmd->redirs[j].target) {
                            free(cmd->redirs[j].target);
                        }
                    }
                    free(cmd->redirs);
                }
                free(cmd);
            }
        }
        free(p->commands);
    }
    if (p->raw_cmdline) {
        free(p->raw_cmdline);
    }
    free(p);
}
