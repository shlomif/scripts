#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#define ENV_PATH_DELIMITER ':'
#define STDIN_PATH_DELIMITER '\n'
#define DIR_DELIMITER '/'

int get_next_env(char const *env);
int get_next_stdin(char const *not_used);
void iterate(char const *file_expr, char const *env, int const record_delim,
             int (*get_next) (char const *));

void check_dir(char const *directory, char const *file_expr);

/* If possible, instead use sysexits.h values */
enum returns { R_NO_HITS = 1 };
int exit_status;
