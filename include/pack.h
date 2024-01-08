#ifndef PACK_H
#define PACK_H

#include <common.h>

/* 遍历指定目录下的文件，将里面文件的相对路径存在files中。eg：
test
├── 1
├── 2
└── test2
    └── 3
列表中存的文件就是1、2、test2、test2/3
*/
int list_dir(const char *path, vector<string> &files, const char *relative);

// 直接打包整个目录下的文件，适用于用户不需要筛选情况
int pack_path(const char *path, const char *tar_name);

// 根据用户返回的文件列表进行打包，适用于需要筛选的情况
int pack_file_list(vector<string> &files, const char *tar_name, const char *path);

// 解包指定的tar文件，放到newpath下
int unpack_file(const char *filepath, const char *newpath);

#endif