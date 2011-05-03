#include <stdio.h>

#define TRUE 1
#define FALSE 0

#define ENV_PATH_DELIMITER ':'
#define STDIN_PATH_DELIMITER '\n'
#define DIR_DELIMITER '/'

int get_next_env(char *env);
int get_next_stdin(char *not_used);
void iterate(char *file_expr, char *env, int record_delim,
             int (*get_next) (void *));

void check_dir(char *directory, char *file_expr);

enum returns { R_OKAY, R_ERR_USAGE, R_NO_HITS, R_ERR_LIMIT,
  R_ERR_BWUH, R_GLOB_ERR
};
int exit_status;
