#include <common.h>


int get_file_info(const char *filename, mtar_header_t &header) {
    // printf("%s\n", filename);
    struct stat statbuf;

	if(stat(filename,&statbuf)==-1) {
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
	if(S_ISREG(statbuf.st_mode)) {
        printf("%s S_ISREG\n", filename);
        header.size = statbuf.st_size;
        header.type = MTAR_TREG;
    }
    // 软链接文件
    if(S_ISLNK(statbuf.st_mode)) {
        printf("%s S_ISLINK\n", filename);
        header.size = statbuf.st_size;
        header.type = MTAR_TSYM;
    }
}

// 将一个文件添加到tar包里（包括header和data）
int tar_one_file(const char *filename, mtar_t & tar) {
    mtar_header_t header;
    memset(&header, 0, sizeof(header));

    get_file_info(filename, header);
    mtar_write_header(&tar, &header);

    if(header.type == MTAR_TREG) {
        int fd = open(filename, O_RDONLY);
        char buf[header.size];
        read(fd, buf, 1023);
        mtar_write_data(&tar, buf, header.size);
    }
    else if(header.type == MTAR_TDIR) {
        // int fd = open(filename, O_RDONLY);
        // char buf[header.size];
        // read(fd, buf, 1023);
        // mtar_write_data(&tar, buf, header.size);
    }
    // 符号链接还有问题，暂时当作普通文件处理
    else if(header.type == MTAR_TSYM) {
        char buf[100] = {0};
        readlink("filename", buf, 100);
        strcpy(header.linkname, buf);
        mtar_write_data(&tar, buf, strlen(buf));
    }
    // mtar_next(&tar);
    return 0;
}

// 递归遍历目录，传入文件列表
int list_dir(const char *path, vector<string> &files) {
	struct dirent * entry;
	size_t len;
	char *sub_path;
	
	DIR *p_dir = opendir(path);
	if (p_dir == NULL) {
		printf("Can not open %s/n", path);
		return -1;
	}
	while((entry = readdir(p_dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            continue;
        
		len = strlen(path) + strlen(entry->d_name) + 3;
		sub_path = (char *)calloc(len, 1);
		if (sub_path == NULL) {
			printf("out of memory/n");
			closedir(p_dir);
			return -1;
		}
		strcpy(sub_path, path);
		strcat(sub_path, "/");
		strcat(sub_path, entry->d_name);
        files.emplace_back(sub_path);
		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 
            && strcmp(entry->d_name, "..") != 0) {
            // 递归
			list_dir(sub_path, files);
		}
		free(sub_path);
	}
	closedir(p_dir);
	return -1;
}

int pack_path(const char *path) {
    mtar_t tar;
    // 遍历目录，打包
    vector<string> files;
    list_dir(path, files);

    if(mtar_open(&tar, "temp.tar", "w") != 0) {
        return -1;
    }
    for(string file : files) {
        if(tar_one_file(file.c_str(), tar) != 0) {
            mtar_finalize(&tar);
            mtar_close(&tar);
            return -1;
        }
    }
    
    mtar_finalize(&tar);
    mtar_close(&tar);
    return 0;
}

int unpack_file(const char *filepath) {
    mtar_t tar;
    mtar_header_t header;
    vector<string> fails;
    int flag = 0;

    /* 打开存档以进行读取操作 */
    mtar_open(&tar, filepath, "r");

    /* 打印所有文件名和文件大小 */
    while ( (mtar_read_header(&tar, &header)) != MTAR_ENULLRECORD ) {
        printf("%s (%d bytes)\n", header.name, header.size);
        if(flag == 0) {
            regex r("(.+?)/");
            std::smatch m;
            string str = header.name;
            if (regex_search(str, m, r)){
                mkdir(m[1].str().c_str(), 0775);
            }
            flag = 1;
        }

        // 目录文件
        if(header.type == MTAR_TDIR) {
            printf("MTAR_TDIR\n");
            int ret = mkdir(header.name, header.mode);
            if(ret < 0) {
                printf("create dir failed\n");
            }
        }
        // 普通文件，读取内容
        if(header.type == MTAR_TREG) {
            printf("MTAR_TREG\n");
            char *p = (char*)calloc(1, header.size + 1);
            mtar_read_data(&tar, p, header.size);
            int fd = open(header.name, O_RDWR|O_CREAT);
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
        if(header.type == MTAR_TREG) {
            char *p = (char*)calloc(1, header.size + 1);
            mtar_read_data(&tar, p, header.size);
            int fd = open(header.name, O_RDWR|O_CREAT);
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

int main() {
    pack_path("test");
    // printf("pack success\n\n");
    // unpack_file("temp2.tar");
}