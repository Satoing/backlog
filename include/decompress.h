#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//首先预处理生成候选处理列表
//随后处理候选列表，遍历读取两个最小值比较，当数组中只剩一个节点时结束运行
#define MAX_LIST_LEN 256
#define HASH_TABLE_SIZE 1571

typedef struct huffman_tree_node {
    char code[MAX_LIST_LEN]; //用于记录编码，以字符串形式
    unsigned char letter;    //编码的该字母
    int code_length; //用于记录编码长度
} TreeNode;


// 用于字典索引的字母列表
TreeNode *hash_list[MAX_LIST_LEN];
// Hash表
int hash_table[HASH_TABLE_SIZE];

unsigned int hash(char *key) {
    unsigned int seed = 787;
    unsigned int hash = 0;
    while (*key)
        hash = hash * seed + (*key++);
    return hash % HASH_TABLE_SIZE;
}

// 用于初始化所有全局变量
void init() {
    for (int i = 0; i < MAX_LIST_LEN; i++)
        hash_list[i] = NULL;
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        hash_table[i] = -1;
}

void insert_hash_table(char *key, int position);

int search_hash_table(char *key);

// 同时填充list和hash_list，初次填充
void add_letter_dict(int position, const char *code, int code_length, unsigned char letter);

// 获得list中存有节点的数量
int len_list(TreeNode *list[MAX_LIST_LEN]);

// 包括读取文件最开始部分生成字典，以及按照字典还原文件
void decompress(const char *filename, const char *writefilename);

void free_hash_list();

// 传入要解压的文件和解压后文件的名字
void decompress_tar(const char *file, const char *rename);