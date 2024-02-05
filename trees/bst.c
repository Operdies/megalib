#include <stdio.h>
#include <stdlib.h>

typedef struct tree_t tree_t;
typedef int key_t;
typedef int value_t;

struct tree_t {
	value_t value;
	tree_t *l, *r, *p;
};

int depth(tree_t *n) {
	if (n) {
		int l, r;
		l = n->l ? depth(n->l) + 1 : 0;
		r = n->r ? depth(n->r) + 1 : 0;
		return l > r ? l : r;
	}
	return 0;
}


// print the root before the values in either subtree
void preorder_treewalk(tree_t *root, int level){
	if (root) {
		printf("%d", root->value);
		if (root->l){
			printf(",");
			printf("\n%*s(", level, "");
			preorder_treewalk(root->l, level + 1);
			printf(")");
		}
		if (root->r){
			printf(",");
			printf("\n%*s[", level, "");
			preorder_treewalk(root->r, level + 1);
			printf("]");
		}
	}
}

// print the root after the values in its subtrees
void postorder_treewalk(tree_t *root){
	if (root) {
		postorder_treewalk(root->l);
		postorder_treewalk(root->r);
		printf(" - %d", root->value);
	}
}

// print the key of the root of a subtree between printing the values in its left subtree and right subtree
void inorder_treewalk(tree_t *root){
	if (root) {
		printf(" down ");
		inorder_treewalk(root->l);
		printf("%d", root->value);
		inorder_treewalk(root->r);
		printf(" up ");
	}
}

tree_t *mk_tree(value_t value) {
	tree_t *r = calloc(sizeof(tree_t), 1);
	r->value = value;
	return r;
}

void destroy_tree(tree_t *root){
	if (root) {
		destroy_tree(root->l);
		destroy_tree(root->r);
		free(root);
	}
}

void _insert(tree_t *node, tree_t *v){
	v->p = node;
	if (v->value < node->value) {
		if (node->l) {
			_insert(node->l, v);
		} else {
			node->l = v;
		}
	} else {
		if (node->r) {
			_insert(node->r, v);
		} else {
			node->r = v;
		}
	}
}

tree_t *insert(tree_t *tree, value_t value) {
	tree_t *v = mk_tree(value);
	if (!tree)
		return v;

	while (tree->p)
		tree = tree->p;
	_insert(tree, v);
	return v;
}

#define t(...) &(tree_t) { __VA_ARGS__ }
int main(void){
	tree_t *root = NULL;
	int set[] = { 1, 4, 10, 5, 17, 16, 21};
	for (int i = 0; i < (int)(sizeof(set) / sizeof(set[0])); i++)
		root = insert(root, set[i]);

	while (root->p) root = root->p;

	preorder_treewalk(root, 0);
	
	printf("\n\ndepth: %d\n", depth(root));

	destroy_tree(root);
	return 0;
}

#undef t
