#ifndef BACKUP_H
#define BACKUP_H

#include <common.h>
#include <compress.h>
#include <decompress.h>
#include <pack.h>

extern "C" {

// 传入要打包的path和打包后文件的名字（不含后缀）
int backup_all_files(const char *path, const char *name);

// 传入文件列表，打包后文件的名字和要打包的path
int backup_file_list(vector<string> &files, const char *name, const char *path);

// 传入要解包的文件名（都存放在固定路径）和解包路径
int restore(const char *file, const char *path);

// 获取目录下的文件，写入文件中（因为Python调用C函数不方便传入指针）
int get_dir_files(const char *path, const char *relative);
}

#endif