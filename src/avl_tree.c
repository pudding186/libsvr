#include "../include/avl_tree.h"
#include "./data_def.h"
#include "../include/memory_pool.h"
#include <string.h>
#include <stdlib.h>

__declspec(thread) HMEMORYUNIT def_avl_tree_unit = 0;
__declspec(thread) HMEMORYUNIT def_avl_node_unit = 0;

inline HMEMORYUNIT _default_avl_tree_unit(void)
{
    return def_avl_tree_unit;
}

inline HMEMORYUNIT _default_avl_node_unit(void)
{
    return def_avl_node_unit;
}

static __inline ptrdiff_t _avl_node_height(HAVLNODE node)
{
    return node ? node->avl_height : 0;
}

static __inline void _avl_link_node(HAVLTREE root, HAVLNODE node, HAVLNODE parent,
    HAVLNODE* avl_link)
{
    node->avl_height = 1;
    node->avl_parent = parent;
    node->avl_child[0] = node->avl_child[1] = 0;

    *avl_link = node;

    if (parent)
    {
        if (&(parent->avl_child[0]) == avl_link)
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

static void _avl_roate(HAVLTREE tree, HAVLNODE node)
{
    const ptrdiff_t side = (node != node->avl_parent->avl_child[0]);
    const ptrdiff_t other_side = !side;

    HAVLNODE child = node->avl_child[other_side];
    HAVLNODE parent = node->avl_parent;

    node->avl_parent = parent->avl_parent;
    node->avl_child[other_side] = parent;

    parent->avl_child[side] = child;
    if (child)
        child->avl_parent = parent;

    if (parent->avl_parent)
    {
        const ptrdiff_t parent_side = (parent != parent->avl_parent->avl_child[0]);
        parent->avl_parent->avl_child[parent_side] = node;
    }
    else
        tree->root = node;

    parent->avl_parent = node;

    parent->avl_height = 1 + max(_avl_node_height(parent->avl_child[0]),
        _avl_node_height(parent->avl_child[1]));
    node->avl_height = 1 + max(_avl_node_height(node->avl_child[0]),
        _avl_node_height(node->avl_child[1]));
}

static void _avl_balance(HAVLTREE tree, HAVLNODE node)
{
    while (node)
    {
        ptrdiff_t balance;

        node->avl_height = 1 + max(_avl_node_height(node->avl_child[0]),
            _avl_node_height(node->avl_child[1]));

        balance = _avl_node_height(node->avl_child[0]) - _avl_node_height(node->avl_child[1]);

        if (balance == 2 || balance == -2)
        {
            ptrdiff_t child_balance;

            HAVLNODE tall_child = node->avl_child[balance == -2];

            child_balance = _avl_node_height(tall_child->avl_child[0]) - _avl_node_height(tall_child->avl_child[1]);

            if (child_balance == 0 || (child_balance < 0) == (balance < 0))
            {
                _avl_roate(tree, tall_child);

                node = tall_child->avl_parent;
            }
            else
            {
                HAVLNODE tall_grand_child = tall_child->avl_child[child_balance == -1];

                _avl_roate(tree, tall_grand_child);
                _avl_roate(tree, tall_grand_child);

                node = tall_grand_child->avl_parent;
            }
        }
        else
        {
            node = node->avl_parent;
        }
    }
}

void _avl_splice_out(HAVLTREE tree, HAVLNODE node)
{
    //assert(!node->avl_child[0] || !node->avl_child[1]);

    HAVLNODE parent = node->avl_parent;

    const size_t child_index = (node->avl_child[1] != 0);

    HAVLNODE child = node->avl_child[child_index];

    //assert(node->avl_child[!child_index] == 0);

    if (child)
        child->avl_parent = parent;

    if (parent)
    {
        parent->avl_child[node == parent->avl_child[1]] = child;
    }
    else
        tree->root = child;
}

void avl_tree_erase(HAVLTREE tree, HAVLNODE node)
{
    if (node)
    {
        HAVLNODE parent = node->avl_parent;

        if (!node->avl_child[0] || !node->avl_child[1])
        {
            _avl_splice_out(tree, node);
            _avl_balance(tree, parent);
        }
        else
        {
            HAVLNODE successor = node->list_next;

            HAVLNODE successor_parent = successor->avl_parent;

            _avl_splice_out(tree, successor);

            successor->avl_parent = parent;
            successor->avl_child[0] = node->avl_child[0];
            successor->avl_child[1] = node->avl_child[1];

            if (successor->avl_child[0])
            {
                successor->avl_child[0]->avl_parent = successor;
            }

            if (successor->avl_child[1])
            {
                successor->avl_child[1]->avl_parent = successor;
            }

            if (parent)
            {
                parent->avl_child[node == parent->avl_child[1]] = successor;
            }
            else
                tree->root = successor;

            _avl_balance(tree, node == successor_parent ? successor : successor_parent);
        }

        if (node->list_next)
        {
            node->list_next->list_prev = node->list_prev;
        }
        else
        {
            tree->tail = node->list_prev;
        }

        if (node->list_prev)
        {
            node->list_prev->list_next = node->list_next;
        }
        else
        {
            tree->head = node->list_next;
        }

        memory_unit_free(tree->node_unit, node);
        --tree->size;
    }
}

HAVLNODE avl_tree_insert_int(HAVLTREE tree, int key, void* value)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (key > (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_int = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    return node;
}

HAVLNODE avl_tree_insert_int64(HAVLTREE tree, long long key, void* value)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (key > (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_int64 = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    return node;
}

HAVLNODE avl_tree_insert_str(HAVLTREE tree, const char* key, void* value)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        int cmp_ret = strcmp(key, (*new_node)->key.key_str);

        parent = *new_node;

        if (cmp_ret < 0)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (cmp_ret > 0)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            (*new_node)->key.key_str = key;
            (*new_node)->value.value_user = value;
            return (*new_node);
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_str = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    return node;
}

HAVLNODE avl_tree_insert_user(HAVLTREE tree, void* key, void* value)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    if (tree->key_cmp)
    {
        while (*new_node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, (*new_node)->key.key_user);

            parent = *new_node;

            if (cmp_ret < 0)
            {
                new_node = &((*new_node)->avl_child[0]);
            }
            else if (cmp_ret > 0)
            {
                new_node = &((*new_node)->avl_child[1]);
            }
            else
            {
                (*new_node)->key.key_user = key;
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
                new_node = &((*new_node)->avl_child[0]);
            }
            else if (key > (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->avl_child[1]);
            }
            else
            {
                (*new_node)->key.key_user = key;
                (*new_node)->value.value_user = value;
                return (*new_node);
            }
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_user = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    return node;
}

bool avl_tree_try_insert_int(HAVLTREE tree, int key, void* value, HAVLNODE* insert_or_exist_node)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (key > (*new_node)->key.key_int)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_int = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    *insert_or_exist_node = node;

    return true;
}

bool avl_tree_try_insert_int64(HAVLTREE tree, long long key, void* value, HAVLNODE* insert_or_exist_node)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        parent = *new_node;

        if (key < (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (key > (*new_node)->key.key_int64)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_int64 = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    *insert_or_exist_node = node;

    return true;
}

bool avl_tree_try_insert_str(HAVLTREE tree, const char* key, void* value, HAVLNODE* insert_or_exist_node)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    while (*new_node)
    {
        int cmp_ret = strcmp(key, (*new_node)->key.key_str);

        parent = *new_node;

        if (cmp_ret < 0)
        {
            new_node = &((*new_node)->avl_child[0]);
        }
        else if (cmp_ret > 0)
        {
            new_node = &((*new_node)->avl_child[1]);
        }
        else
        {
            *insert_or_exist_node = (*new_node);
            return false;
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_str = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    *insert_or_exist_node = node;

    return true;
}

bool avl_tree_try_insert_user(HAVLTREE tree, void* key, void* value, HAVLNODE* insert_or_exist_node)
{
    HAVLNODE node;
    HAVLNODE* new_node = &(tree->root);
    HAVLNODE parent = 0;

    if (tree->key_cmp)
    {
        while (*new_node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, (*new_node)->key.key_user);

            parent = *new_node;

            if (cmp_ret < 0)
            {
                new_node = &((*new_node)->avl_child[0]);
            }
            else if (cmp_ret > 0)
            {
                new_node = &((*new_node)->avl_child[1]);
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
                new_node = &((*new_node)->avl_child[0]);
            }
            else if (key > (*new_node)->key.key_user)
            {
                new_node = &((*new_node)->avl_child[1]);
            }
            else
            {
                *insert_or_exist_node = (*new_node);
                return false;
            }
        }
    }

    node = (HAVLNODE)memory_unit_alloc(tree->node_unit, 1024);

    node->key.key_user = key;
    node->value.value_user = value;

    _avl_link_node(tree, node, parent, new_node);
    _avl_balance(tree, node);

    *insert_or_exist_node = node;

    return true;
}

HAVLNODE avl_tree_find_int(HAVLTREE tree, int key)
{
    HAVLNODE node = tree->root;

    while (node)
    {
        if (key < node->key.key_int)
        {
            node = node->avl_child[0];
        }
        else if (key > node->key.key_int)
        {
            node = node->avl_child[1];
        }
        else
            return node;
    }

    return 0;
}

HAVLNODE avl_tree_find_int64(HAVLTREE tree, long long key)
{
    HAVLNODE node = tree->root;

    while (node)
    {
        if (key < node->key.key_int64)
        {
            node = node->avl_child[0];
        }
        else if (key > node->key.key_int64)
        {
            node = node->avl_child[1];
        }
        else
            return node;
    }

    return 0;
}

HAVLNODE avl_tree_find_str(HAVLTREE tree, const char* key)
{
    HAVLNODE node = tree->root;

    while (node)
    {
        int cmp_ret = strcmp(key, node->key.key_str);

        if (cmp_ret < 0)
        {
            node = node->avl_child[0];
        }
        else if (cmp_ret > 0)
        {
            node = node->avl_child[1];
        }
        else
            return node;
    }

    return 0;
}

HAVLNODE avl_tree_find_user(HAVLTREE tree, void* key)
{
    HAVLNODE node = tree->root;

    if (tree->key_cmp)
    {
        while (node)
        {
            ptrdiff_t cmp_ret = tree->key_cmp(key, node->key.key_user);

            if (cmp_ret < 0)
            {
                node = node->avl_child[0];
            }
            else if (cmp_ret > 0)
            {
                node = node->avl_child[1];
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
                node = node->avl_child[0];
            }
            else if (key > node->key.key_user)
            {
                node = node->avl_child[1];
            }
            else
                return node;
        }
    }

    return 0;
}

HAVLNODE avl_tree_find_int_nearby(HAVLTREE tree, int key)
{
    HAVLNODE node = tree->root;
    HAVLNODE nearby_node = node;

    while (node)
    {
        nearby_node = node;

        if (key < node->key.key_int)
        {
            node = node->avl_child[0];
        }
        else if (key > node->key.key_int)
        {
            node = node->avl_child[1];
        }
        else
            return node;
    }

    return nearby_node;
}

HAVLNODE avl_tree_find_int64_nearby(HAVLTREE tree, long long key)
{
    HAVLNODE node = tree->root;
    HAVLNODE nearby_node = node;

    while (node)
    {
        nearby_node = node;

        if (key < node->key.key_int64)
        {
            node = node->avl_child[0];
        }
        else if (key > node->key.key_int64)
        {
            node = node->avl_child[1];
        }
        else
            return node;
    }

    return nearby_node;
}

int avl_node_key_int(HAVLNODE node)
{
    return node->key.key_int;
}

long long avl_node_key_int64(HAVLNODE node)
{
    return node->key.key_int64;
}

const char* avl_node_key_str(HAVLNODE node)
{
    return node->key.key_str;
}

void* avl_node_key_user(HAVLNODE node)
{
    return node->key.key_user;
}

void* avl_node_value(HAVLNODE node)
{
    return node->value.value_user;
}

void avl_node_set_value(HAVLNODE node, void* new_value)
{
    node->value.value_user = new_value;
}

int avl_node_value_int(HAVLNODE node)
{
    return node->value.value_int;
}

void avl_node_set_value_int(HAVLNODE node, int value_int)
{
    node->value.value_int = value_int;
}

long long avl_node_value_int64(HAVLNODE node)
{
    return node->value.value_int64;
}

void avl_node_set_value_int64(HAVLNODE node, long long value_int64)
{
    node->value.value_int64 = value_int64;
}

HAVLNODE avl_first(CONST_HAVLTREE tree)
{
    return tree->head;
}

HAVLNODE avl_last(CONST_HAVLTREE tree)
{
    return tree->tail;
}

HAVLNODE avl_next(CONST_HAVLNODE node)
{
    return node->list_next;
}

HAVLNODE avl_prev(CONST_HAVLNODE node)
{
    return node->list_prev;
}

HAVLTREE create_avl_tree(key_cmp cmp_func)
{
    HAVLTREE tree = 0;

    tree = (HAVLTREE)memory_unit_alloc(_default_avl_tree_unit(), 256);
    tree->root = 0;
    tree->size = 0;
    tree->head = 0;
    tree->tail = 0;
    tree->key_cmp = cmp_func;
    tree->node_unit = _default_avl_node_unit();

    return tree;
}

void destroy_avl_tree(HAVLTREE tree)
{
    HAVLNODE node = tree->head;

    while (node)
    {
        memory_unit_free(tree->node_unit, node);
        node = node->list_next;
    }

    memory_unit_free(_default_avl_tree_unit(), tree);
}

void avl_tree_clear(HAVLTREE tree)
{
    HAVLNODE node = tree->head;

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

size_t avl_tree_size(HAVLTREE root)
{
    return root->size;
}

key_cmp avl_tree_cmp_func_ptr(HAVLTREE tree)
{
    return tree->key_cmp;
}

