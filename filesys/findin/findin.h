#ifndef _H_FINDIN_H_
#define _H_FINDIN_H_

#include <err.h>
#include <glob.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define ENV_PATH_DELIMITER ':'
#define STDIN_PATH_DELIMITER '\n'
#define DIR_DELIMITER '/'

void emit_help(void);
int get_next_env(const char *env);
int get_next_stdin(const char *not_used);
void iterate(char *file_expr, const char *env, int record_delim,
             int (*get_next) (const char *));

void check_dir(char *directory, char *file_expr);

#endif
