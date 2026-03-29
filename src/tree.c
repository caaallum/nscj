#include <Windows.h>
#include <cjson/cJSON.h>
#include <assert.h>

#include "tree.h"

struct tree_t {
	TCHAR name[64];
	cJSON* root;
	struct tree_t* next;
};

//==========================================================
tree_t*
tree_find(_In_ LPCTSTR name) {
	tree_t* cur = g_trees;

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

	tree = GlobalAlloc(GPTR, sizeof(tree_t));
	assert(tree);
	lstrcpy(tree->name, name);
	tree->root = root;

	tree->next = g_trees;
	g_trees = tree;
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
	tree_t* cur = g_trees;
	while (cur) {
		tree_t* next = cur->next;

		if (cur->root)
			cJSON_Delete(cur->root);

		GlobalFree(cur);
		cur = next;
	}
	g_trees = NULL;
}
