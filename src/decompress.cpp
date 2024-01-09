#include <decompress.h>

TreeNode *hash_list_2[MAX_LIST_LEN]; // 用于字典索引的字母列表
int hash_table_2[HASH_TABLE_SIZE_2]; // Hash表

// 用于初始化所有全局变量
void init_2() {
    for (int i = 0; i < MAX_LIST_LEN; i++)
        hash_list_2[i] = NULL;
    for (int i = 0; i < HASH_TABLE_SIZE_2; i++)
        hash_table_2[i] = -1;
}

unsigned int hash_2(char *key) {
    unsigned int seed = 787;
    unsigned int hash = 0;
    while (*key)
        hash = hash * seed + (*key++);
    return hash % HASH_TABLE_SIZE_2;
}

void insert_hash_table_2(char *key, int position) {
    int addr = hash_2(key);
    while (hash_table_2[addr] != -1)
        addr = (addr + 1) % HASH_TABLE_SIZE_2;
    hash_table_2[addr] = position;
}

int search_hash_table_2(char *key) {
    int addr = hash_2(key);
    while (1) {
        if (hash_table_2[addr] == -1)
            return -1;
        if (!strcmp(key, hash_list_2[hash_table_2[addr]]->code))
            return hash_table_2[addr];
        addr = (addr + 1) % HASH_TABLE_SIZE_2;
    }
    return -1;
}

// 同时填充list和hash_list，初次填充
void add_letter_dict_2(int position, const char *code, int code_length, unsigned char letter) {
    TreeNode *new_node = (TreeNode *)malloc(sizeof(TreeNode));
    new_node->letter = letter;
    for (int i = 0; i < MAX_LIST_LEN; i++)
        new_node->code[i] = '\0';
    strcpy(new_node->code, code);
    new_node->code_length = code_length;

    // 填充hash_list
    hash_list_2[position] = new_node;
}

void decompress(const char *filename, const char *writefilename) {
    // ! 文件结构为 {nodes_count, [{节点编码结构}, ...], 文本编码长度, 文本编码}
    // ! 节点编码结构 {letter, code_length, code[0], code[1],...,code[code_length-1]}
    FILE *fin = fopen(filename, "r");

    // 以下读取哈夫曼树结构，构造hash表，表结构为： map[code] = letter
    int nodes_count = 0;
    long codes_count = 0; // 编码bit长度
    // TODO 改为short
    fread(&nodes_count, sizeof(int), 1, fin);

    int letter_num = 0; // 字符种类数
    for (int i = 0; i < nodes_count; i++) {
        unsigned char letter;
        int code_length = -1;
        char code[MAX_LIST_LEN] = {0};
        char temp[MAX_LIST_LEN / 7] = {0};

        fread(&letter, sizeof(char), 1, fin);
        short code_len = -1;
        fread(&code_len, sizeof(short), 1, fin);
        code_length = (int)code_len;
        // 计算占用的char数
        int target_char_num = code_length / 8;
        if (code_length % 8 > 0)
            target_char_num += 1;
        fread(temp, sizeof(char), target_char_num, fin);
        for (int j = 0; j < code_length; j++) {
            if ((temp[j / 8] & (1 << (7 - (j % 8)))) > 0)
                strcat(code, "1");
            else
                strcat(code, "0");
        }

        // 写入字典
        int dict_addr = search_hash_table_2(code);
        if (dict_addr == -1) { //哈希表中尚不存在该字符
            insert_hash_table_2(code, letter_num);
            add_letter_dict_2(letter_num, code, code_length, letter);
            letter_num++;
        }
        else return; //哈希表中存在该字符，出错
    }
    fread(&codes_count, sizeof(long), 1, fin);

    unsigned char *dest_char = (unsigned char *)malloc(sizeof(char) * codes_count);
    for (int i = 0; i < codes_count; i++)
        dest_char[i] = 0;

    int curr_subchar_index = 0;
    int curr_destchar_index = 0;
    unsigned char curr_char;
    char curr_bits[MAX_LIST_LEN] = {0};
    for (int i = 0; i < codes_count; i++) {
        if (curr_subchar_index % 8 == 0) {
            fread(&curr_char, sizeof(char), 1, fin);
            curr_subchar_index = 0;
        }

        // unsigned char curr_bit_char;
        if ((curr_char & (1 << (7 - curr_subchar_index))) > 0)
            strcat(curr_bits, "1");
        else strcat(curr_bits, "0");

        int addr = search_hash_table_2(curr_bits);
        if (addr != -1) {
            // 匹配到
            dest_char[curr_destchar_index] = hash_list_2[addr]->letter;
            curr_destchar_index += 1;
            for (int k = 0; k < MAX_LIST_LEN; k++) {
                if (curr_bits[k] == 0)
                    break;
                curr_bits[k] = 0;
            }
        }
        curr_subchar_index += 1;
    }
    // printf("%s", dest_char);
    fclose(fin);

    // 将解码后数据写入文件
    FILE *fout = fopen(writefilename, "wb");
    if (fout == NULL) {
        perror("open failed!");
        exit(1);
    }
    fwrite(dest_char, sizeof(char), curr_destchar_index, fout);
    fclose(fout);
    free(dest_char);
    dest_char = NULL;
}

void free_hash_list() {
    for (int i = 0; i < MAX_LIST_LEN; i++)
        if (hash_list_2[i] != NULL) {
            free(hash_list_2[i]);
            hash_list_2[i] = NULL;
        }
        else break;
}


// 传入要解压的文件和解压后文件的名字
void decompress_tar(const char *file, const char *rename) {
    init_2();

    char filename[200] = {0};
    string back_path(BACKUP_PATH);
    back_path += file;
    strcpy(filename, back_path.c_str());

    decompress(filename, rename);
    free_hash_list();
}

// int main(int argc, char *argv[]) {
//     decompress_tar(argv[1], argv[2]);
// }