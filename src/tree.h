#ifndef __TREE_H
#define __TREE_H

typedef struct tree_t tree_t;

/**********************************************************
 * \brief Init tree node
 * 
 * \param name Name of node
 * \param root cJSON structure to set
 * 
 * \return tree_t* newly created node
 **********************************************************/
tree_t* tree_node_init(_In_ LPCTSTR name, _In_ cJSON* root);

/**********************************************************
 * \brief Find tree of given name
 *
 * \param name Name of tree
 * 
 * \return tree_t* Pointer to tree struct.
 *				   NULL if not found
 **********************************************************/
tree_t* tree_find(_In_ LPCTSTR name);

/**********************************************************
 * \brief Add json to tree struct
 *
 * \param name Name of tree struct
 * \param root cJSON structure to set
 **********************************************************/
VOID tree_set(_In_ LPCTSTR name, _In_ cJSON* root);

/**********************************************************
 * \brief Get json root from tree
 *
 * \param name Name of tree struct
 * 
 * \return cJSON* Pointer to tree struct.
 *				  NULL if not found
 **********************************************************/
cJSON* tree_get_root(_In_ LPCTSTR name);

/**********************************************************
 * \brief Clear all trees 
 **********************************************************/
VOID tree_clear(VOID);

#endif /* __TREE_H */