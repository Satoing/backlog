#include <common.h>
#include <pack.h>

int get_file_info(const char *filename, mtar_header_t &header, const char *path) {
    struct stat statbuf;

    string path_s(path);
    path_s += "/";
    path_s += filename;

	if(stat(path_s.c_str(), &statbuf)==-1) {
		printf("Get stat on %s Error: %s\n", filename, strerror(errno));
		return(-1);
	}

    // 统一的操作
    unsigned mask = 0700;
    header.mtime = statbuf.st_mtim.tv_sec;
    header.mode = statbuf.st_mode & mask;
    header.owner = statbuf.st_uid;
    strcpy(header.name, filename);

    // 目录
	if(S_ISDIR(statbuf.st_mode)) {
        printf("%s S_ISDIR\n", filename);
        header.type = MTAR_TDIR;
    }
    // 普通文件
	if(S_ISREG(statbuf.st_mode) && statbuf.st_nlink == 1) {
        printf("%s S_ISREG\n", filename);
        header.type = MTAR_TREG;
        header.size = statbuf.st_size;
    }
    // 硬链接普通文件，也要存储文件数据
    if(S_ISREG(statbuf.st_mode) && statbuf.st_nlink > 1) {
        printf("%s S_ISHARD\n", filename);
        header.type = MTAR_TLNK;
        header.size = statbuf.st_size;
        // 存储inode号，便于后面判断文件是否存在
        strcpy(header.linkname, std::to_string(statbuf.st_ino).c_str());
        printf("%s\n", header.linkname);
    }
    // 软链接普通文件，怎么处理？
    if(S_ISLNK(statbuf.st_mode)) {
        printf("%s S_ISLINK\n", filename);
        header.type = MTAR_TSYM;
        header.size = statbuf.st_size;
        // 存储链接文件名
        char buf[101] = {0};
        readlink("filename", buf, 100);
        strcpy(header.linkname, buf);
    }
}

// 将一个文件添加到tar包里（包括header和data）
int tar_one_file(const char *filename, mtar_t & tar, const char *path) {
    mtar_header_t header;
    memset(&header, 0, sizeof(header));

    get_file_info(filename, header, path);
    mtar_write_header(&tar, &header);

    string path_s(path);
    path_s += "/";
    path_s += filename;

    // 普通文件和硬链接文件都要读数据
    // 后者的原因是如果原来的inode不存在了就写新的文件
    if(header.type == MTAR_TREG || header.type == MTAR_TLNK) {
        int fd = open(path_s.c_str(), O_RDONLY);
        char buf[1024];

        // 防止一次性没读完
        int ret = read(fd, buf, 1023);
        while(ret > 0) {
            mtar_write_data(&tar, buf, ret);
            memset(buf, 0, 1024);
            ret = read(fd, buf, 1023);
        }
    }
    else if(header.type == MTAR_TDIR) {
        // int fd = open(filename, O_RDONLY);
        // char buf[header.size];
        // read(fd, buf, 1023);
        // mtar_write_data(&tar, buf, header.size);
    }
    // 符号链接还有问题，暂时当作普通文件处理
    else if(header.type == MTAR_TSYM) {
    }
    // mtar_next(&tar);
    // printf("%s pack ok\n", filename);
    return 0;
}

// 传入目录的绝对路径，文件列表引用，最后一个参数是为了递归使用，初始传入""即可
int list_dir(const char *path, vector<string> &files, const char *relative) {
	struct dirent * entry;
	size_t len;
	char *sub_path;
    char *full_path;
	
    len = strlen(relative) + strlen(path) + 3;
    full_path = (char *)calloc(len, 1);
    strcpy(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, relative);

	DIR *p_dir = opendir(full_path);
	if (p_dir == NULL) {
		printf("Can not open %s/n", path);
		return -1;
	}
	while((entry = readdir(p_dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            continue;
        
        len = strlen(relative) + strlen(entry->d_name) + 3;
		sub_path = (char *)calloc(len, 1);
        strcpy(sub_path, relative);
		if(strcmp(relative, "") != 0) {
            strcat(sub_path, "/");
        }
		strcat(sub_path, entry->d_name);
        files.emplace_back(sub_path);
        printf("%s\n", sub_path);

		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 
            && strcmp(entry->d_name, "..") != 0) {
            // 递归
			list_dir(path, files, sub_path);
		}
		free(sub_path);
	}
	closedir(p_dir);
	return -1;
}

// 传入的path末尾不能带"/"，tar_name末尾要带.tar
int pack_path(const char *path, const char *tar_name) {
    mtar_t tar;
    // 遍历目录，打包
    vector<string> files;
    list_dir(path, files, "");

    if(mtar_open(&tar, tar_name, "w") != 0) {
        return -1;
    }
    for(string file : files) {
        // 压缩失败
        if(tar_one_file(file.c_str(), tar, path) != 0) {
            mtar_finalize(&tar);
            mtar_close(&tar);
            return -1;
        }
    }
    mtar_finalize(&tar);
    mtar_close(&tar);
    return 0;
}

int pack_file_list(vector<string> &files, const char *tar_name, const char *path) {
    mtar_t tar;

    if(mtar_open(&tar, tar_name, "w") != 0) {
        return -1;
    }
    for(string file : files) {
        // 压缩失败
        if(tar_one_file(file.c_str(), tar, path) != 0) {
            mtar_finalize(&tar);
            mtar_close(&tar);
            return -1;
        }
    }
    mtar_finalize(&tar);
    mtar_close(&tar);
    return 0;
}

// 需要恢复文件元数据
// 输入要解包的tar文件和要解包到的新路径，新路径不能包含"/"
int unpack_file(const char *filepath, const char *newpath) {
    mtar_t tar;
    mtar_header_t header;
    vector<string> fails;
    
    // 应该是不存在则创建
    mkdir(newpath, 775);
    string path = newpath;
    path += "/";

    // 打开打包文件进行读取操作
    mtar_open(&tar, filepath, "r");

    while ( (mtar_read_header(&tar, &header)) != MTAR_ENULLRECORD ) {
        printf("%s (%d bytes)\n", header.name, header.size);
        // 完整的路径名+文件名
        string temp(path);
        temp += header.name;

        // 目录文件
        if(header.type == MTAR_TDIR) {
            printf("MTAR_TDIR\n");
            int ret = mkdir(temp.c_str(), header.mode);
            if(ret < 0) {
                printf("create dir failed\n");
            }
        }
        // 普通文件，读取内容
        if(header.type == MTAR_TREG) {
            printf("MTAR_TREG\n");
            char *p = (char*)calloc(1, header.size + 1);
            mtar_read_data(&tar, p, header.size);
            int fd = open(temp.c_str(), O_RDWR|O_CREAT);
            if(fd < 0) {
                fails.emplace_back(header.name);
                mtar_next(&tar);
                continue;
            }
            write(fd, p, header.size);
            close(fd);
            free(p);
        }
        mtar_next(&tar);
    }
    // 处理失败的文件
    for(string fail : fails) {
        mtar_find(&tar, fail.c_str(), &header);
        // 一般都是普通文件
        // 完整的路径名+文件名
        string temp(newpath);
        temp += header.name;

        if(header.type == MTAR_TREG) {
            char *p = (char*)calloc(1, header.size + 1);
            mtar_read_data(&tar, p, header.size);
            int fd = open(temp.c_str(), O_RDWR|O_CREAT);
            if(fd < 0) {
                fails.emplace_back(header.name);
                continue;
            }
            write(fd, p, header.size);
            close(fd);
            free(p);
        }
    }
    /* 关闭 */
    mtar_close(&tar);
}

// int main(int argc, char *argv[]) {
//     if(strcmp(argv[1], "-u") == 0) {
//         unpack_file(argv[2], argv[3]);
//         printf("unpack success\n\n");
//     }
//     else if(strcmp(argv[1], "-p") == 0) {
//         pack_path(argv[2], argv[3]);
//         printf("pack success\n\n");
//     }
// }