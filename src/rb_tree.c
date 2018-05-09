#include "../include/rb_tree.h"
#include "./data_def.h"
#include "../include/memory_pool.h"
#include <string.h>
#include <stdlib.h>

#define	RB_RED		    0
#define	RB_BLACK	    1
#define rb_color(r)   ((r)->rb_color)
#define rb_is_red(r)    (!rb_color(r))
#define rb_is_black(r)  rb_color(r)

__declspec(thread) HMEMORYUNIT def_rb_tree_unit = 0;
__declspec(thread) HMEMORYUNIT def_rb_node_unit = 0;

inline HMEMORYUNIT _default_rb_tree_unit(void)
{
    return def_rb_tree_unit;
}

inline HMEMORYUNIT _default_rb_node_unit(void)
{
    return def_rb_node_unit;
}

static __inline void _rb_link_node(HRBTREE root, HRBNODE node, HRBNODE parent, HRBNODE* rb_link)
{
    node->rb_color = RB_RED;
    node->rb_parent = parent;
    node->rb_left = node->rb_right = 0;

    *rb_link = node;

    if (parent)
    {
        if (&(parent->rb_left) == rb_link)
        {
			if (root->head == parent)
			{
				node->list_next = parent;
				node->list_prev = 0;

				parent->list_prev = node;
				root->head = node;
			}
			else
			{
				node->list_next = parent;
				node->list_prev = parent->list_prev;

				parent->list_prev->list_next = node;
				parent->list_prev = node;
			}
        }
        else
        {
			if (root->tail == parent)
			{
				node->list_next = 0;
				node->list_prev = parent;

				parent->list_next = node;
				root->tail = node;
			}
			else
			{
				node->list_next = parent->list_next;
				node->list_prev = parent;

				parent->list_next->list_prev = node;
				parent->list_next = node;
			}
        }
    }
    else
    {
        root->head = node;
        root->tail = node;
        node->list_next = 0;
        node->list_prev = 0;
    }

    ++root->size;
}

static __inline void _rb_erase_list(HRBTREE root, HRBNODE node)
{
    if (node->list_prev)
    {
        node->list_prev->list_next = node->list_next;
    }
    else
    {
        root->head = node->list_next;
    }

    if (node->list_next)
    {
        node->list_next->list_prev = node->list_prev;
    }
    else
    {
        root->tail = node->list_prev;
    }

    --root->size;
}

#pragma warning( push )
#pragma warning( disable : 4706 )

static void _rb_rotate_left(HRBNODE node, HRBTREE root)
{
    rb_node *right = node->rb_right;
    rb_node *parent = node->rb_parent;

    if ((node->rb_right = right->rb_left))
        right->rb_left->rb_parent = node;

    right->rb_left = node;

    right->rb_parent = parent;

    if (parent)
    {
        if (node == parent->rb_left)
            parent->rb_left = right;
        else
            parent->rb_right = right;
    }
    else
        root->root = right;

    node->rb_parent = right;
}

static void _rb_rotate_right(rb_node *node, rb_tree *root)
{
    rb_node *left = node->rb_left;
    rb_node *parent = node->rb_parent;

    if ((node->rb_left = left->rb_right))
        left->rb_right->rb_parent = node;

    left->rb_right = node;

    left->rb_parent = parent;

    if (parent)
    {
        if (node == parent->rb_right)
            parent->rb_right = left;
        else
            parent->rb_left = left;
    }
    else
        root->root = left;
    node->rb_parent = left;
}

void _rb_insert_balance(rb_node *node, rb_tree *root)
{
    rb_node *parent, *gparent;

    while ((parent = node->rb_parent) && rb_is_red(parent))
    {
        gparent = parent->rb_parent;

        if (parent == gparent->rb_left)
        {
            {
                register rb_node *uncle = gparent->rb_right;
                if (uncle && rb_is_red(uncle))
                {
                    uncle->rb_color = RB_BLACK;
                    parent->rb_color = RB_BLACK;
                    gparent->rb_color = RB_RED;

                    node = gparent;
                    continue;
                }
            }

            if (parent->rb_right == node)
            {
                register rb_node *tmp;
                _rb_rotate_left(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            parent->rb_color = RB_BLACK;
            gparent->rb_color = RB_RED;

            _rb_rotate_right(gparent, root);
        }
        else
        {
            {
                register rb_node *uncle = gparent->rb_left;
                if (uncle && rb_is_red(uncle))
                {
                    uncle->rb_color = RB_BLACK;
                    parent->rb_color = RB_BLACK;
                    gparent->rb_color = RB_RED;

                    node = gparent;
                    continue;
                }
            }

            if (parent->rb_left == node)
            {
                register rb_node *tmp;
                _rb_rotate_right(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            parent->rb_color = RB_BLACK;
            gparent->rb_color = RB_RED;

            _rb_rotate_left(gparent, root);
        }
    }

    root->root->rb_color = RB_BLACK;
}

#pragma warning( pop )

static void _rb_erase_balance(rb_node *node, rb_node *parent, rb_tree *root)
{
    rb_node *other;

    while ((!node || rb_is_black(node)) && node != root->root)
    {
        if (parent->rb_left == node)
        {
            other = parent->rb_right;
            if (rb_is_red(other))
            {
                other->rb_color = RB_BLACK;
                parent->rb_color = RB_RED;
                _rb_rotate_left(parent, root);
                other = parent->rb_right;
            }
            if ((!other->rb_left || rb_is_black(other->rb_left)) &&
                (!other->rb_right || rb_is_black(other->rb_right)))
            {
                other->rb_color = RB_RED;
                node = parent;
                parent = node->rb_parent;
            }
            else
            {
                if (!other->rb_right || rb_is_black(other->rb_right))
                {
                    other->rb_left->rb_color = RB_BLACK;
                    other->rb_color = RB_RED;
                    _rb_rotate_right(other, root);
                    other = parent->rb_right;
                }
                other->rb_color = parent->rb_color;
                parent->rb_color = RB_BLACK;
                other->rb_right->rb_color = RB_BLACK;

                _rb_rotate_left(parent, root);
                node = root->root;
                break;
            }
        }
        else
        {
            other = parent->rb_left;
            if (rb_is_red(other))
            {
                other->rb_color = RB_BLACK;
                parent->rb_color = RB_RED;

                _rb_rotate_right(parent, root);
                other = parent->rb_left;
            }
            if ((!other->rb_left || rb_is_black(other->rb_left)) &&
                (!other->rb_right || rb_is_black(other->rb_right)))
            {
                other->rb_color = RB_RED;
                node = parent;
                parent = node->rb_parent;
            }
            else
            {
                if (!other->rb_left || rb_is_black(other->rb_left))
                {
                    other->rb_right->rb_color = RB_BLACK;
                    other->rb_color = RB_RED;
                    _rb_rotate_left(other, root);
                    other = parent->rb_left;
                }
                other->rb_color = parent->rb_color;
                parent->rb_color = RB_BLACK;
                other->rb_left->rb_color = RB_BLACK;
                _rb_rotate_right(parent, root);
                node = root->root;
                break;
            }
        }
    }
    if (node)
        node->rb_color = RB_BLACK;
}

void rb_tree_erase(HRBTREE root, HRBNODE node)
{
    rb_node *child, *parent, *del;
    int color;

    if (node)
    {
        del = node;

        if (!node->rb_left)
            child = node->rb_right;
        else if (!node->rb_right)
            child = node->rb_left;
        else
        {
            rb_node *old = node, *left;

            node = node->rb_right;
            while ((left = node->rb_left) != 0)
                node = left;

            if (old->rb_parent) {
                if (old->rb_parent->rb_left == old)
                    old->rb_parent->rb_left = node;
                else
                    old->rb_parent->rb_right = node;
            } else
                root->root = node;

            child = node->rb_right;
            parent = node->rb_parent;
            color = rb_color(node);

            if (parent == old) {
                parent = node;
            } else {
                if (child)
                    child->rb_parent = parent;

                parent->rb_left = child;

                node->rb_right = old->rb_right;
                old->rb_right->rb_parent = node;
            }

            node->rb_parent = old->rb_parent;
            node->rb_color = old->rb_color;
            node->rb_left = old->rb_left;
            old->rb_left->rb_parent = node;

            goto color;
        }

        parent = node->rb_parent;
        color = rb_color(node);

        if (child)
            child->rb_parent = parent;

        if (parent)
        {
            if (parent->rb_left == node)
                parent->rb_left = child;
            else
                parent->rb_right = child;
        }
        else
            root->root = child;

color:
        if (color == RB_BLACK)
            _rb_erase_balance(child, parent, root);
        _rb_erase_list(root, del);
        memory_unit_free(root->node_unit, del);
    }
}

HRBNODE rb_first(CONST_HRBTREE tree)
{
    return tree->head;
}

HRBNODE rb_last(CONST_HRBTREE tree)
{
    return tree->tail;
}

HRBNODE rb_next(CONST_HRBNODE node)
{
    return node->list_next;
}

HRBNODE rb_prev(CONST_HRBNODE node)
{
    return node->list_prev;
}

size_t rb_tree_size(HRBTREE root)
{
    return root->size;
}

HRBTREE create_rb_tree(key_cmp cmp_func)
{
    HRBTREE tree = (HRBTREE)memory_unit_alloc(_default_rb_tree_unit(), 256);
    tree->root = 0;
    tree->size = 0;
    tree->head = 0;
    tree->tail = 0;
    tree->key_cmp = cmp_func;
    tree->node_unit = _default_rb_node_unit();

    return tree;
}

void destroy_rb_tree(HRBTREE tree)
{
    HRBNODE node = tree->head;

    while (node)
    {
        memory_unit_free(tree->node_unit, node);
        node = node->list_next;
    }

    memory_unit_free(_default_rb_tree_unit(), tree);
}

HRBTREE create_rb_tree_ex(key_cmp cmp_func)
{
    HRBTREE tree = (HRBTREE)malloc(sizeof(rb_tree));
    tree->root = 0;
    tree->size = 0;
    tree->head = 0;
    tree->tail = 0;
    tree->key_cmp = cmp_func;
    tree->node_unit = create_memory_unit(sizeof(rb_node));

    return tree;
}

void destroy_rb_tree_ex(HRBTREE tree)
{
    HRBNODE node = tree->head;

    while (node)
    {
        memory_unit_free(tree->node_unit, node);
        node = node->list_next;
    }

    destroy_memory_unit(tree->node_unit);
    free(tree);
}

void rb_tree_clear(HRBTREE tree)
{
    HRBNODE node = tree->head;

    while (node)
    {
        memory_unit_free(tree->node_unit, node);
        node = node->list_next;
    }

    tree->root = 0;
    tree->size = 0;
    tree->head = 0;
    tree->tail = 0;
}

HRBNODE rb_tree_insert_int(HRBTREE tree, int key, void* value)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (key > (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_int = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    return node;
}

bool rb_tree_try_insert_int(HRBTREE tree, int key, void* value, HRBNODE* insert_or_exist_node)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (key > (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_int = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    *insert_or_exist_node = node;

    return true;
}

HRBNODE rb_tree_insert_int64(HRBTREE tree, long long key, void* value)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (key > (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_int64 = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    return node;
}

bool rb_tree_try_insert_int64(HRBTREE tree, long long key, void* value, HRBNODE* insert_or_exist_node)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (key > (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_int64 = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    *insert_or_exist_node = node;

    return true;
}

HRBNODE rb_tree_insert_str(HRBTREE tree, const char* key, void* value)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        int cmp_ret = strcmp(key, (*new_node)->key.key_str);

        parent = *new_node;

        if (cmp_ret < 0)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (cmp_ret > 0)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            (*new_node)->key.key_str = key;
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_str = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    return node;
}

bool rb_tree_try_insert_str(HRBTREE tree, const char* key, void* value, HRBNODE* insert_or_exist_node)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    while (*new_node)
    {
        int cmp_ret = strcmp(key, (*new_node)->key.key_str);

        parent = *new_node;

        if (cmp_ret < 0)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (cmp_ret > 0)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_str = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    *insert_or_exist_node = node;

    return true;
}

HRBNODE rb_tree_insert_user(HRBTREE tree, void* key, void* value)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    if (tree->key_cmp)
    {
        while (*new_node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, (*new_node)->key.key_user);

            parent = *new_node;

            if (cmp_ret < 0)
            {
                new_node = &((*new_node)->rb_left);
            }
            else if (cmp_ret > 0)
            {
                new_node = &((*new_node)->rb_right);
            }
            else
            {
                (*new_node)->key.key_str = key;
                (*new_node)->value.value_user = value;
                return (*new_node);
            }
        }
    }
    else
    {
        while (*new_node)
        {
            parent = *new_node;

            if (key < (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->rb_left);
            }
            else if (key > (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->rb_right);
            }
            else
            {
                (*new_node)->key.key_str = key;
                (*new_node)->value.value_user = value;
                return (*new_node);
            }
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_user = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    return node;
}

bool rb_tree_try_insert_user(HRBTREE tree, void* key, void* value, HRBNODE* insert_or_exist_node)
{
    HRBNODE node;
    HRBNODE* new_node = &(tree->root);
    HRBNODE parent = 0;

    if (tree->key_cmp)
    {
        while (*new_node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, (*new_node)->key.key_user);

            parent = *new_node;

            if (cmp_ret < 0)
            {
                new_node = &((*new_node)->rb_left);
            }
            else if (cmp_ret > 0)
            {
                new_node = &((*new_node)->rb_right);
            }
            else
            {
                *insert_or_exist_node = (*new_node);
                return false;
            }
        }
    }
    else
    {
        while (*new_node)
        {
            parent = *new_node;

            if (key < (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->rb_left);
            }
            else if (key > (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->rb_right);
            }
            else
            {
                *insert_or_exist_node = (*new_node);
                return false;
            }
        }
    }

    node = (HRBNODE)memory_unit_alloc(tree->node_unit, 4*1024);

    node->key.key_user = key;
    node->value.value_user = value;

    _rb_link_node(tree, node, parent, new_node);
    _rb_insert_balance(node, tree);

    *insert_or_exist_node = node;

    return true;
}

HRBNODE rb_tree_find_int(HRBTREE tree, int key)
{
    HRBNODE node = tree->root;

    while (node)
    {
        if (key < node->key.key_int)
        {
            node = node->rb_left;
        }
        else if (key > node->key.key_int)
        {
            node = node->rb_right;
        }
        else
            return node;
    }

    return 0;
}

HRBNODE rb_tree_find_int64(HRBTREE tree, long long key)
{
    HRBNODE node = tree->root;

    while (node)
    {
        if (key < node->key.key_int64)
        {
            node = node->rb_left;
        }
        else if (key > node->key.key_int64)
        {
            node = node->rb_right;
        }
        else
            return node;
    }

    return 0;
}

HRBNODE rb_tree_find_str(HRBTREE tree, const char* key)
{
    HRBNODE node = tree->root;

    while (node)
    {
        int cmp_ret = strcmp(key, node->key.key_str);

        if (cmp_ret < 0)
        {
            node = node->rb_left;
        }
        else if (cmp_ret > 0)
        {
            node = node->rb_right;
        }
        else
            return node;
    }

    return 0;
}

HRBNODE rb_tree_find_user(HRBTREE tree, void* key)
{
    HRBNODE node = tree->root;

    if (tree->key_cmp)
    {
        while (node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, node->key.key_user);

            if (cmp_ret < 0)
            {
                node = node->rb_left;
            }
            else if (cmp_ret > 0)
            {
                node = node->rb_right;
            }
            else
                return node;
        }
    }
    else
    {
        while (node)
        {
            if (key < node->key.key_user)
            {
                node = node->rb_left;
            }
            else if (key > node->key.key_user)
            {
                node = node->rb_right;
            }
            else
                return node;
        }
    }

    return 0;
}

HRBNODE rb_tree_find_int_nearby(HRBTREE tree, int key)
{
    HRBNODE node = tree->root;
    HRBNODE nearby_node = node;

    while (node)
    {
        nearby_node = node;

        if (key < node->key.key_int)
        {
            node = node->rb_left;
        }
        else if (key > node->key.key_int)
        {
            node = node->rb_right;
        }
        else
            return node;
    }

    return nearby_node;
}

HRBNODE rb_tree_find_int64_nearby(HRBTREE tree, long long key)
{
    HRBNODE node = tree->root;
    HRBNODE nearby_node = node;

    while (node)
    {
        nearby_node = node;

        if (key < node->key.key_int64)
        {
            node = node->rb_left;
        }
        else if (key > node->key.key_int64)
        {
            node = node->rb_right;
        }
        else
            return node;
    }

    return nearby_node;
}

int rb_node_key_int( HRBNODE node )
{
    return node->key.key_int;
}

long long rb_node_key_int64( HRBNODE node )
{
    return node->key.key_int64;
}

const char* rb_node_key_str( HRBNODE node )
{
    return node->key.key_str;
}

void* rb_node_key_user( HRBNODE node )
{
    return node->key.key_user;
}

void* rb_node_value( HRBNODE node )
{
    return node->value.value_user;
}

void rb_node_set_value(HRBNODE node, void* new_value)
{
    node->value.value_user = new_value;
}

int rb_node_value_int(HRBNODE node )
{
    return node->value.value_int;
}

void rb_node_set_value_int(HRBNODE node, int value_int)
{
    node->value.value_int = value_int;
}

long long rb_node_value_int64(HRBNODE node )
{
    return node->value.value_int64;
}

void rb_node_set_value_int64(HRBNODE node, long long value_int64)
{
    node->value.value_int64 = value_int64;
}

key_cmp rb_tree_cmp_func_ptr(HRBTREE tree)
{
    return tree->key_cmp;
}
