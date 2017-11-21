/* red-black binary search tree implmentation based on code in "Algorithms"
 * (4th Edition) by Robert Sedgewick and Kevin Wayne */

#if defined(linux) || defined(__linux) || defined(__linux__)
#define _BSD_SOURCE
#endif

#include "redblack-bst.h"

void rbbst_flip_colors(rbbst_node n);
rbbst_node rbbst_insert(rbbst_node n, rbbst_key key, rbbst_value val);
bool rbbst_is_red(rbbst_node n);
rbbst_node rbbst_newnode(rbbst_key key, rbbst_value val);
rbbst_node rbbst_rotate_left(rbbst_node n);
rbbst_node rbbst_rotate_right(rbbst_node n);
void rbbst_walk_envp(rbbst_node n, char ***epp);

extern char **environ;

rbbst_node tree = NULL;
size_t tree_count = 1;          /* +1 for trailing NULL on envp */

bool have_dups = false;

/***********************************************************************
 * Public Functions */

char **rbbst2envp(void)
{
    char **newenv, **ep;

    if (!have_dups)
        return environ;

    if ((newenv = (char **) malloc(sizeof(char **) * tree_count)) == NULL)
        err(EX_OSERR, "could not malloc new environ");
    ep = newenv;

    rbbst_walk_envp(tree, &ep);

    *ep = (char *) NULL;
    return newenv;
}

void rbbst_add(rbbst_key key, rbbst_value val)
{
    tree = rbbst_insert(tree, key, val);
    tree->color = RBBST_BLACK;
}

/***********************************************************************
 * Private Functions */

inline void rbbst_flip_colors(rbbst_node n)
{
    n->color = !n->color;
    n->left->color = !n->left->color;
    n->right->color = !n->right->color;
}

rbbst_node rbbst_insert(rbbst_node n, rbbst_key key, rbbst_value val)
{
    int cmp;

    if (n == NULL)
        return rbbst_newnode(key, val);

    cmp = strcmp(key, n->key);
    if (cmp < 0) {
        n->left = rbbst_insert(n->left, key, val);
    } else if (cmp > 0) {
        n->right = rbbst_insert(n->right, key, val);
    } else {
        /* this is a duplicate value; we instead go with the first
         * (following go, Perl, Ruby, ZSH etc. and notably not bash)
         * could instead toss an exception here, or log something */
        //n->value = val;
        have_dups = true;
        return n;
    }

    if (rbbst_is_red(n->right) && !rbbst_is_red(n->left)) {
        n = rbbst_rotate_left(n);
    }
    if (rbbst_is_red(n->left) && rbbst_is_red(n->left->left)) {
        n = rbbst_rotate_right(n);
    }
    if (rbbst_is_red(n->left) && rbbst_is_red(n->right)) {
        rbbst_flip_colors(n);
    }

    return n;
}

bool rbbst_is_red(rbbst_node n)
{
    if (n == NULL)
        return false;
    return n->color == RBBST_RED;
}

rbbst_node rbbst_newnode(rbbst_key key, rbbst_value val)
{
    rbbst_node n;

    if ((n = calloc((size_t) 1, sizeof(struct rbbst_node_t))) == NULL)
        err(EX_OSERR, "could not calloc node");

    if ((n->key = strdup(key)) == NULL)
        err(EX_OSERR, "could not strdup");

    n->value = val;
    n->color = RBBST_RED;

    tree_count++;

    return n;
}

rbbst_node rbbst_rotate_left(rbbst_node n)
{
    rbbst_node x = n->right;
    n->right = x->left;
    x->left = n;
    x->color = x->left->color;
    x->left->color = RBBST_RED;
    return x;
}

rbbst_node rbbst_rotate_right(rbbst_node n)
{
    rbbst_node x = n->left;
    n->left = x->right;
    x->right = n;
    x->color = x->right->color;
    x->right->color = RBBST_RED;
    return x;
}

void rbbst_walk_envp(rbbst_node n, char ***epp)
{
    **epp = n->value;
    (*epp)++;
    if (n->left) {
        rbbst_walk_envp(n->left, epp);
    }
    if (n->right) {
        rbbst_walk_envp(n->right, epp);
    }
}
