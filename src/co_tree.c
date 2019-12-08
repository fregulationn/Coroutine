#include "coroutine.h"
#include "co_tree.h"

#define RB_PARENT(R) ((R)->parent)
#define RB_COLOR(R) ((R)->color)
#define RB_IS_RED(R) ((R)->color==RED)
#define RB_IS_BLACK(R) ((R)->color==BLACK)
#define RB_SET_BLACK(R) do{ (R)->color = BLACK; } while(0)
#define RB_SET_RED(R) do{ (R)->color = RED; } while(0)
#define RB_SET_PARENT(R,P) do{ (R)->parent = (P); } while(0)
#define RB_SET_COLOR(R, C) do{ (R)->color = (C); } while(0)

// create rb tree
RBRoot* create_rbtree() {
    RBRoot *root = (RBRoot *)malloc(sizeof(RBRoot));
    root->node = NULL;

    return root;
}

Node* rbtree_search(RBRoot *root, Type key) {
    Node *x = root->node;
    
    while((x != NULL) && (x->key != key)) {
        if(key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    return x;
}


/* 
* 对红黑树的节点(x)进行左旋转
*
* 左旋示意图(对节点x进行左旋)：
*      px                              px
*     /                               /
*    x                               y                
*   /  \      --(左旋)-->           / \                #
*  lx   y                          x  ry     
*     /   \                       /  \
*    ly   ry                     lx  ly  
*
*
*/
void rbtree_left_rotate(RBRoot *root, Node *x) {
    // set x's right child y
    Node *y = x->right;

    // set y's left child as x's right child
    // if y's left child is not NULL, set x as y's parent
    x->right = y->left;
    if(y->left != NULL) {
        y->left->parent = x;
    }

    // set x's parent as y's parent
    y->parent = x->parent;

    if(x->parent == NULL) {
        // tree = y;
        root->node = y;
    } else {
        if(x->parent->left == x) {
            x->parent->left = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
        } else {
            x->parent->right = y;   // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
        }
    }

    // set x as y's left child
    y->left = x;
    // set x'parent as y
    x->parent = y;
}


/* 
* 对红黑树的节点(y)进行右旋转
*
* 右旋示意图(对节点y进行左旋)：
*            py                               py
*           /                                /
*          y                                x                  
*         /  \      --(右旋)-->            /  \                     #
*        x   ry                           lx   y  
*       / \                                   / \                   #
*      lx  rx                                rx  ry
* 
*/
void rbtree_right_rotate(RBRoot *root, Node *y) {
    // x is left child of y
    Node *x = y->left;

    // set x's right child as y's left child
    // if x's right child not NULL, set y as parent of x's right child
    y->left = x->right;
    if(x->right != NULL) {
        x->right->parent = y;
    }

    // set y's parent as x's parent
    x->parent = y->parent;

    if(y->parent == NULL) {
        // tree = x         
        root->node = x;     // if y's parent is null, set x as root
    } else {
        if(y == y->parent->right) {
            y->parent->right = x;  // if y is the parent of right child, set x as y->parent->right
        } else {
            y->parent->left = x;  // if y is the parent of left child, set x as y->parent->left
        }
    }

    // set y as x's right chld
    y = x->right;

    // set y's parent as x
    y->parent = x;

}


/*
* 红黑树插入修正函数
*
* 在向红黑树中插入节点之后(失去平衡)，再调用该函数；
* 目的是将它重新塑造成一颗红黑树。
*
* 参数说明：
*     root 红黑树的根
*     node 插入的结点        // 对应《算法导论》中的z
*/
static void rbtree_insert_fixup(RBRoot *root, Node *node) {
    Node *parent, *grand_pa;

    // 若“父节点存在，且父节点的颜色是红色”
    while((parent = RB_PARENT(node)) && RB_IS_RED(parent)) {
        grand_pa = RB_PARENT(parent);

        //若“父节点”是“祖父节点的左孩子”
        if(parent == grand_pa->left) {
            // Case 1条件：叔叔节点是红色
            Node *uncle = grand_pa->right;
            if (uncle && RB_IS_RED(uncle))
            {
                RB_SET_BLACK(uncle);
                RB_SET_BLACK(parent);
                RB_SET_RED(grand_pa);
                node = grand_pa;
                continue;
            }

            // Case 2条件：叔叔是黑色，且当前节点是右孩子
            if (parent->right == node)
            {
                Node *tmp;
                rbtree_left_rotate(root, parent);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            // Case 3条件：叔叔是黑色，且当前节点是左孩子。
            RB_SET_BLACK(parent);
            RB_SET_RED(grand_pa);
            rbtree_right_rotate(root, grand_pa);
        } else {
            //若“z的父节点”是“z的祖父节点的右孩子”
            // Case 1条件：叔叔节点是红色
            Node *uncle = grand_pa->left;
            if (uncle && RB_IS_RED(uncle))
            {
                RB_SET_BLACK(uncle);
                RB_SET_BLACK(parent);
                RB_SET_RED(grand_pa);
                node = grand_pa;
                continue;
            }

            // Case 2条件：叔叔是黑色，且当前节点是左孩子
             if (parent->left == node)
             {
                 Node *tmp;
                 rbtree_right_rotate(root, parent);
                 tmp = parent;
                 parent = node;
                 node = tmp;
             }
 
             // Case 3条件：叔叔是黑色，且当前节点是右孩子。
             RB_SET_BLACK(parent);
             RB_SET_RED(grand_pa);
             rbtree_left_rotate(root, grand_pa);
        }
    }

    // 将根节点设为黑色
    RB_SET_BLACK(root->node);
}

/*
* 添加节点：将节点(node)插入到红黑树中
*
* 参数说明：
*     root 红黑树的根
*     node 插入的结点        // 对应《算法导论》中的z
*/
void rbtree_insert(RBRoot *root, Node *node)
{
    Node *y = NULL;
    Node *x = root->node;

    // 1. 将红黑树当作一颗二叉查找树，将节点添加到二叉查找树中。
    while (x != NULL)
    {
        y = x;
        if (node->key < x->key)
            x = x->left;
        else
            x = x->right;
    }
    RB_PARENT(node) = y;

    if (y != NULL)
    {
        if (node->key < y->key)
            y->left = node;                // 情况2：若“node所包含的值” < “y所包含的值”，则将node设为“y的左孩子”
        else
            y->right = node;            // 情况3：(“node所包含的值” >= “y所包含的值”)将node设为“y的右孩子” 
    }
    else
    {
        root->node = node;                // 情况1：若y是空节点，则将node设为根
    }

    // 2. 设置节点的颜色为红色
    node->color = RED;

    // 3. 将它重新修正为一颗二叉查找树
    rbtree_insert_fixup(root, node);
}


/*
* 红黑树删除修正函数
*
* 在从红黑树中删除插入节点之后(红黑树失去平衡)，再调用该函数；
* 目的是将它重新塑造成一颗红黑树。
*
* 参数说明：
*     root 红黑树的根
*     node 待修正的节点
*/
static void rbtree_delete_fixup(RBRoot *root, Node *node, Node *parent)
{
    Node *other;

    while ((!node || rb_is_black(node)) && node != root->node)
    {
        if (parent->left == node)
        {
            other = parent->right;
            if (RB_IS_RED(other))
            {
                // Case 1: x的兄弟w是红色的  
                RB_SET_BLACK(other);
                RB_SET_RED(parent);
                rbtree_left_rotate(root, parent);
                other = parent->right;
            }
            if ((!other->left || RB_IS_BLACK(other->left)) &&
                (!other->right || RB_IS_BLACK(other->right)))
            {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                RB_SET_RED(other);
                node = parent;
                parent = RB_PARENT(node);
            }
            else
            {
                if (!other->right || RB_IS_BLACK(other->right))
                {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  
                    RB_SET_BLACK(other->left);
                    RB_SET_RED(other);
                    rbtree_right_rotate(root, other);
                    other = parent->right;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。
                RB_SET_COLOR(other, RB_COLOR(parent));
                RB_SET_BLACK(parent);
                RB_SET_BLACK(other->right);
                rbtree_left_rotate(root, parent);
                node = root->node;
                break;
            }
        }
        else
        {
            other = parent->left;
            if (RB_IS_RED(other))
            {
                // Case 1: x的兄弟w是红色的  
                RB_SET_BLACK(other);
                RB_SET_RED(parent);
                rbtree_right_rotate(root, parent);
                other = parent->left;
            }
            if ((!other->left || RB_IS_BLACK(other->left)) &&
                (!other->right || RB_IS_BLACK(other->right)))
            {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                RB_SET_RED(other);
                node = parent;
                parent = RB_PARENT(node);
            }
            else
            {
                if (!other->left || RB_IS_BLACK(other->left))
                {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  
                    RB_SET_BLACK(other->right);
                    RB_SET_RED(other);
                    rbtree_left_rotate(root, other);
                    other = parent->left;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。
                RB_SET_COLOR(other, RB_COLOR(parent));
                RB_SET_BLACK(parent);
                RB_SET_BLACK(other->left);
                rbtree_right_rotate(root, parent);
                node = root->node;
                break;
            }
        }
    }
    if (node)
        RB_SET_BLACK(node);
}


/* 
* 删除结点
*
* 参数说明：
*     tree 红黑树的根结点
*     node 删除的结点
*/
void rbtree_delete(RBRoot *root, Type key)
{   

    Node *node = rbtree_search(root->node, key);
    if(node == NULL) return;

    Node *child, *parent;
    int color;

    // 被删除节点的"左右孩子都不为空"的情况。
    if ( (node->left!=NULL) && (node->right!=NULL) ) 
    {
        // 被删节点的后继节点。(称为"取代节点")
        // 用它来取代"被删节点"的位置，然后再将"被删节点"去掉。
        Node *replace = node;

        // 获取后继节点
        replace = replace->right;
        while (replace->left != NULL)
            replace = replace->left;

        // "node节点"不是根节点(只有根节点不存在父节点)
        if (rb_parent(node))
        {
            if (RB_PARENT(node)->left == node)
                RB_PARENT(node)->left = replace;
            else
                RB_PARENT(node)->right = replace;
        } 
        else 
            // "node节点"是根节点，更新根节点。
            root->node = replace;

        // child是"取代节点"的右孩子，也是需要"调整的节点"。
        // "取代节点"肯定不存在左孩子！因为它是一个后继节点。
        child = replace->right;
        parent = RB_PARENT(replace);
        // 保存"取代节点"的颜色
        color = RB_COLOR(replace);

        // "被删除节点"是"它的后继节点的父节点"
        if (parent == node)
        {
            parent = replace;
        } 
        else
        {
            // child不为空
            if (child)
                RB_SET_PARENT(child, parent);
            parent->left = child;

            replace->right = node->right;
            RB_SET_PARENT(node->right, replace);
        }

        replace->parent = node->parent;
        replace->color = node->color;
        replace->left = node->left;
        node->left->parent = replace;

        if (color == BLACK)
            rbtree_delete_fixup(root, child, parent);
        free(node);

        return ;
    }

    if (node->left !=NULL)
        child = node->left;
    else 
        child = node->right;

    parent = node->parent;
    // 保存"取代节点"的颜色
    color = node->color;

    if (child)
        child->parent = parent;

    // "node节点"不是根节点
    if (parent)
    {
        if (parent->left == node)
            parent->left = child;
        else
            parent->right = child;
    }
    else
        root->node = child;

    if (color == BLACK)
        rbtree_delete_fixup(root, child, parent);
    free(node);
}


/*
* 销毁红黑树
*/
static void rbtree_destroy(Node* tree)
{
    if (tree==NULL)
        return ;

    if (tree->left != NULL)
        rbtree_destroy(tree->left);
    if (tree->right != NULL)
        rbtree_destroy(tree->right);

    free(tree);
}

void destroy_rbtree(RBRoot *root)
{
    if (root != NULL)
        rbtree_destroy(root->node);

    free(root);
}