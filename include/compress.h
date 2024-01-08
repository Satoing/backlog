#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//首先预处理生成候选处理列表
//随后处理候选列表，遍历读取两个最小值比较，当数组中只剩一个节点时结束运行
#define MAX_LIST_LEN 256
#define HASH_TABLE_SIZE 257
#define INT32_MAX 2147483647

typedef struct huffman_tree_node {
    unsigned char letter; //编码的该字母
    int value;            //节点权重

    struct huffman_tree_node *parent;       //父节点
    struct huffman_tree_node *left, *right; //左右孩子节点

    char code[MAX_LIST_LEN]; //用于记录编码，以字符串形式
    int code_length;         //用于记录编码长度
} TreeNode;

// TODO 构建候选列表
void build_list(); //构建候选处理列表
TreeNode *build_huffman_tree();

TreeNode *list[MAX_LIST_LEN]; // 候选项列表
TreeNode *hash_list[MAX_LIST_LEN]; // 用于字典索引的字母列表
int hash_table[HASH_TABLE_SIZE]; // Hash表

// 用于初始化所有全局变量
void init() {
    for (int i = 0; i < MAX_LIST_LEN; i++) {
        list[i] = NULL;
        hash_list[i] = NULL;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        hash_table[i] = -1;
}

// 实现哈希表，用于存储键值对
// ==================================================
// hash函数
unsigned int hash(unsigned char key);

// 处理冲突
void insert_hash_table(unsigned char key, int position);

// 定位，获取key在哈希表中的位置
int search_hash_table(unsigned char key);

// 同时填充list和hash_list，初次填充
void add_letter_dict(int position, unsigned char letter, int value);

// 获得list中存有节点的数量
int len_list(TreeNode *list[MAX_LIST_LEN]);

// 构建候选列表
void build_list(const char *filename);

// 获取列表中两个最小值的下标，便于构建哈夫曼树
void smallest_two_value(TreeNode *list[MAX_LIST_LEN], int *first, int *second);


// 哈夫曼编码实现文件压缩
// ===================================
// 基于哈希表（词频表）构建哈夫曼树
TreeNode *build_huffman_tree();

// 根据生成的二叉树生成变长二进制code，左0右1
void gen_code(TreeNode *root, int code_length, const char *code);

// 根据字典&输入文本，将之翻译为变长二进制数据，利用long long，流式输出到文件
void compress(const char *filename);

void free_tree_mem(TreeNode *root);

// 传入要解压的tar文件即可
void compress_tar(const char *file);