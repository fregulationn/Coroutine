#ifndef _RB_TREE_H
#define _RB_TREE_H

#include <stdint.h>

#define RED 0
#define BLACK 1

struct coroutine;
typedef struct coroutine coroutine_t;
typedef int Type;

// rb tree node
typedef struct RBTreeNode {
    uint8_t color;
    Type key;
    coroutine_t *co;
    struct RBTreeNode *left;
    struct RBTreeNode *right;
    struct RBTreeNode *parent;

}Node;

typedef struct rb_root {
    struct RBTreeNode *node;

}RBRoot;

// create rb tree
RBRoot* create_rbtree();
// destroy rb tree
void destroy_rbtree(RBRoot *root);
// left rotate
void rbtree_left_rotate(RBRoot *root, Node *x);
// right rotate
void rbtree_right_rotate(RBRoot *root, Node *y);
// insert
void rbtree_insert(RBRoot *root, Type key, coroutine_t *co);
// delete
void rbtree_delete(RBRoot *root, Type key);
// search
Node* rbtree_search(RBRoot *root, Type key);

#endif