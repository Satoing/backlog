#ifndef COMPRESS_H
#define COMPRESS_H

#include <common.h>

//首先预处理生成候选处理列表
//随后处理候选列表，遍历读取两个最小值比较，当数组中只剩一个节点时结束运行
#define MAX_LIST_LEN 256
#define HASH_TABLE_SIZE 257

#ifndef TREENODE
#define TREENODE

typedef struct huffman_tree_node {
    unsigned char letter; //编码的该字母
    int value;            //节点权重

    struct huffman_tree_node *parent;       //父节点
    struct huffman_tree_node *left, *right; //左右孩子节点

    char code[MAX_LIST_LEN]; //用于记录编码，以字符串形式
    int code_length;         //用于记录编码长度
} TreeNode;

#endif

void compress_tar(const char *file);

#endif