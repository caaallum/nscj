#include <Windows.h>
#include <cjson/cJSON.h>
#include <assert.h>

#include "tree.h"

struct tree_t {
    PTSTR name;
    cJSON* root;
    struct tree_t* next;
};

#define SHARED_MEM_NAME TEXT("Local\\NSCJ_Tree")

//==========================================================
static tree_t**
get_shared_root(VOID) {
    HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);

    if (!hMap) {
        hMap = CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, sizeof(tree_t*),
            SHARED_MEM_NAME
        );
    }

    if (!hMap) return NULL;

    // Intentionally leak hMap — keeps mapping alive across DLL unloads
    return (tree_t**)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(tree_t*));
}

//==========================================================
tree_t*
tree_node_init(_In_ LPCTSTR name, _In_ cJSON* root) {
    tree_t* node = (tree_t*)malloc(sizeof(tree_t));
    assert(node);

    // +1 for null terminator
    node->name = (PTSTR)malloc(sizeof(TCHAR) * (lstrlen(name) + 1));
    assert(node->name);
    lstrcpy(node->name, name);

    node->root = root;
    node->next = NULL;

    return node;
}

//==========================================================
tree_t*
tree_find(_In_ LPCTSTR name) {
    tree_t** ppRoot = get_shared_root();
    if (!ppRoot) return NULL;

    tree_t* cur = *ppRoot;
    UnmapViewOfFile(ppRoot);

    while (cur) {
        if (lstrcmpi(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

//==========================================================
VOID
tree_set(_In_ LPCTSTR name, _In_ cJSON* root) {
    tree_t* tree = tree_find(name);

    if (tree) {
        if (tree->root) {
            cJSON_Delete(tree->root);
        }
        tree->root = root;
        return;
    }

    tree = tree_node_init(name, root);

    tree_t** ppRoot = get_shared_root();
    if (!ppRoot) return;

    tree->next = *ppRoot;
    *ppRoot = tree;

    UnmapViewOfFile(ppRoot);
}

//==========================================================
cJSON*
tree_get_root(_In_ LPCTSTR name) {
    tree_t* tree = tree_find(name);
    return tree ? tree->root : NULL;
}

//==========================================================
VOID
tree_clear(VOID) {
    tree_t** ppRoot = get_shared_root();
    if (!ppRoot) return;

    tree_t* cur = *ppRoot;
    *ppRoot = NULL;
    UnmapViewOfFile(ppRoot);

    while (cur) {
        tree_t* next = cur->next;

        if (cur->root) {
            cJSON_Delete(cur->root);
        }

        free(cur->name);
        free(cur);
        cur = next;
    }
}