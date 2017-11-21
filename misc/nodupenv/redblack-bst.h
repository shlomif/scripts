#ifndef _RBBST_TREE_H
#define _RBBST_TREE_H 1

#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define RBBST_RED true
#define RBBST_BLACK false

typedef char *rbbst_key;        /* "PATH" or what extracted from */
typedef char *rbbst_value;      /* where the full original "PATH=..." lives */

struct rbbst_node_t {
    rbbst_key key;
    rbbst_value value;
    struct rbbst_node_t *left;
    struct rbbst_node_t *right;
    bool color;
};

typedef struct rbbst_node_t *rbbst_node;

/***********************************************************************
 * Public Functions */

char **rbbst2envp(void);
void rbbst_add(rbbst_key key, rbbst_value val);

#endif
