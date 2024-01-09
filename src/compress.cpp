#include <compress.h>

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

// hash函数
unsigned int hash(unsigned char key) {
    return ((unsigned int)key) % HASH_TABLE_SIZE;
}

// 处理冲突
void insert_hash_table(unsigned char key, int position) {
    int addr = hash(key);
    while (hash_table[addr] != -1) {
        addr = (addr + 1) % HASH_TABLE_SIZE;
    }
    hash_table[addr] = position;
}

// 定位，获取key在哈希表中的位置
int search_hash_table(unsigned char key) {
    int addr = hash(key);
    while (1) {
        if (hash_table[addr] == -1)
            return -1;
        if (key == hash_list[hash_table[addr]]->letter)
            return hash_table[addr];
        addr = (addr + 1) % HASH_TABLE_SIZE;
    }
    return -1;
}

// 同时填充list和hash_list，初次填充
void add_letter_dict(int position, unsigned char letter, int value) {
    TreeNode *new_node = (TreeNode *)malloc(sizeof(TreeNode));
    new_node->letter = letter;
    new_node->value = value;
    new_node->parent = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    
    for (int i = 0; i < MAX_LIST_LEN; i++)
        new_node->code[i] = '\0';
    new_node->code_length = -1;

    // 同时填充list和hash_list
    hash_list[position] = new_node;
    list[position] = new_node;
}

// 获得list中存有节点的数量
int len_list(TreeNode *list[MAX_LIST_LEN]) {
    int len = 0;
    for (int i = 0; i < MAX_LIST_LEN; i++)
        if (list[i] != NULL)
            len++;
    return len;
}

// 构建候选列表
void build_list(const char *filename) {
    int letter_num = 0; //用于标记当前已经读取的字符总数，方便写入hash_list

    FILE *fin = fopen(filename, "rb");
    unsigned char ch;
    fread(&ch, sizeof(char), 1, fin);

    while (!feof(fin)) {
        int dict_addr = search_hash_table(ch);
        if (dict_addr == -1) { //哈希表中尚不存在该字符
            insert_hash_table(ch, letter_num);
            add_letter_dict(letter_num, ch, 1);
            letter_num++;
        }
        else //哈希表中存在该字符，自增1
            hash_list[dict_addr]->value += 1;
        fread(&ch, sizeof(char), 1, fin);
    }
    fclose(fin);
}

// 获取列表中两个最小值的下标，便于构建哈夫曼树
void smallest_two_value(TreeNode *list[MAX_LIST_LEN], int *first, int *second) {
    int one_value = INT32_MAX, two_value = INT32_MAX;
    int one = -1, two = -1;
    int list_len = len_list(list);
    int curr_num = 0;
    int value_temp, index_temp;
    for (int i = 0; i < MAX_LIST_LEN; i++) {
        if (curr_num == list_len) break;
        if (list[i] == NULL) continue;

        curr_num++;
        if (list[i]->value < one_value) {
            value_temp = one_value;
            index_temp = one;
            one_value = list[i]->value;
            one = i;
            two_value = value_temp;
            two = index_temp;
        }
        else if (list[i]->value < two_value) {
            two_value = list[i]->value;
            two = i;
        }
    }
    *first = one;
    *second = two;
}


// 哈夫曼编码实现文件压缩
// ===================================
// 基于哈希表（词频表）构建哈夫曼树
TreeNode * build_huffman_tree() {
    if (len_list(list) == 0) return NULL;
    while (len_list(list) > 1) // 当待处理列表中还有节点存在，即未出现根节点
    {
        // 生成新节点以连接两个最小值
        TreeNode *new_node = (TreeNode *)malloc(sizeof(TreeNode));
        new_node->parent = NULL;
        new_node->letter = '\0';

        // 得到列表中值最小的两个节点
        int first = -1, second = -1;
        smallest_two_value(list, &first, &second);

        // 将新父节点与左右子节点分别绑定
        new_node->value = list[first]->value + list[second]->value;
        list[first]->parent = new_node;
        list[second]->parent = new_node;
        new_node->left = list[first];
        new_node->right = list[second];

        // 占用子节点中的左孩子在list中的位置，并抹除右节点
        list[second] = NULL;
        list[first] = new_node;
    }

    // 得到根节点
    int root_index = -1;
    for (int i = 0; i < MAX_LIST_LEN; i++)
        if (list[i] == NULL)
            continue;
        else {
            root_index = i;
            break;
        }

    return list[root_index];
}

// 根据生成的二叉树生成变长二进制code，左0右1
void gen_code(TreeNode *root, int code_length, const char *code) {
    if (root->left != NULL) {
        char code_left[MAX_LIST_LEN] = {'\0'};
        strcpy(code_left, code);
        strcat(code_left, "0");
        gen_code(root->left, code_length + 1, code_left);
    }
    if (root->right != NULL) {
        char code_right[MAX_LIST_LEN] = {'\0'};
        strcpy(code_right, code);
        strcat(code_right, "1");
        gen_code(root->right, code_length + 1, code_right);
    }
    // 处理叶子结点
    if (root->left == NULL && root->right == NULL) {
        strcpy(root->code, code);
        root->code_length = code_length;
    }
}

// 根据字典&输入文本，将之翻译为变长二进制数据，利用long long，流式输出到文件
void compress(const char *filename) {
    long letter_count = 0; // 用于标记共有多少letter
    FILE *fin = fopen(filename, "rb");

    // 统计文件字符数
    if (!fin) return;
    fseek(fin, 0L, SEEK_END);
    letter_count = ftell(fin);

    rewind(fin); // 文件指针重新回到开始

    // 分配最终压缩后编码的空间
    unsigned char *dest_codes = (unsigned char *)malloc(sizeof(unsigned char) * letter_count * 2);
    for (int i = 0; i < letter_count * 2; i++)
        dest_codes[i] = 0; // 清零初始化

    long codes_count = 0;       // 用于标记共占用多少bit位
    int curr_char_index = 0;    //用于标记当前处理的char(字节块)下标
    int curr_subchar_index = 0; //用于标记当前处理的char内从左数第几个bit位

    // 循环处理每一个字符
    unsigned char ch;
    fread(&ch, sizeof(char), 1, fin);
    while (!feof(fin)) {
        int addr = search_hash_table(ch);
        int letter_code_length = hash_list[addr]->code_length;

        for (int i = 0; i < hash_list[addr]->code_length; i++) {
            if ((hash_list[addr]->code)[i] == '0');
            else if ((hash_list[addr]->code)[i] == '1')
                dest_codes[curr_char_index] |= 1 << (7 - curr_subchar_index);
            codes_count += 1;
            curr_subchar_index += 1;

            // 处理完一个char块后
            if (curr_subchar_index == 8) {
                curr_subchar_index = 0;
                curr_char_index += 1;
            }
        }
        fread(&ch, sizeof(char), 1, fin);
    }
    fclose(fin);

    // 计算占用的char数
    long char_count = 0;
    if (codes_count % 8 != 0)
        char_count = codes_count / 8 + 1;
    else
        char_count = codes_count / 8;

    // 得到哈弗曼树编码结构以写入文件
    int hash_nodes_len = len_list(hash_list); // 现有已编码字符种类数

    // 写入文件.compress
    char writefilename[200] = {0};
    string back_path(BACKUP_PATH);
    back_path += filename;
    strcpy(writefilename, back_path.c_str());
    strcat(writefilename, ".compress");

    FILE *fout = fopen(writefilename, "wb");
    if (fout == NULL) {
        perror("open failed!");
        exit(1);
    }

    // 文件结构为 {nodes_count, [{节点编码结构}, ...], 文本编码长度, 文本编码}
    // ===================================================================
    // 写入节点数目
    // TODO 改为short
    fwrite(&hash_nodes_len, sizeof(int), 1, fout);

    // 节点编码结构 {letter, code_length, code[0], code[1],...,code[code_length-1]}
    // 这是对哈夫曼树的精简，解压的时候只需要这些信息即可
    // 写入节点具体信息
    for (int i = 0; i < hash_nodes_len; i++) {
        fwrite(&hash_list[i]->letter, sizeof(char), 1, fout);
        // TODO 改为short
        short code_len = (short)hash_list[i]->code_length;
        fwrite(&code_len, sizeof(short), 1, fout);
        // TODO 改为按位存储的char
        int target_char_num = hash_list[i]->code_length / 8;
        if (hash_list[i]->code_length % 8 > 0)
            target_char_num += 1;
        for (int j = 0; j < target_char_num; j++) {
            unsigned char temp = 0;
            for (int k = 0; k < hash_list[i]->code_length - 8 * j; k++) {
                if ((hash_list[i]->code)[k + j * 8] == '0');
                else
                    temp |= (1 << (7 - k));
            }
            fwrite(&temp, sizeof(char), 1, fout);
        }
    }

    // 写入文本压缩后编码
    fwrite(&codes_count, sizeof(long), 1, fout);
    fwrite(dest_codes, sizeof(char), char_count, fout);

    fclose(fout);
    free(dest_codes);
    dest_codes = NULL;
}

void free_tree_mem(TreeNode *root) {
    if (root->left != NULL)
        free_tree_mem(root->left);
    if (root->right != NULL)
        free_tree_mem(root->right);
    free(root);
    root = NULL;
}

// 传入要压缩的tar文件即可
void compress_tar(const char *file) {
    init();
    build_list(file);
    TreeNode *root_node = build_huffman_tree();
    gen_code(root_node, 0, "\0");
    compress(file);
    free_tree_mem(root_node);
}

// int main(int argc, char *argv[]) {
//     compress_tar(argv[1]);
// }