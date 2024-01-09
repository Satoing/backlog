#include <backup.h>

int backup_all_files(const char *path, const char *name) {
    string tar_name = name;
    tar_name += ".tar";
    pack_path(path, tar_name.c_str());
    compress_tar(tar_name.c_str());
    remove(tar_name.c_str());
    return 0;
}

int backup_file_list(vector<string> &files, const char *name, const char *path) {
    string tar_name = name;
    tar_name += ".tar";
    pack_file_list(files, tar_name.c_str(), path);
    compress_tar(tar_name.c_str());
    remove(tar_name.c_str());
    return 0;
}

int restore(const char *file, const char *path) {
    string filename(file);
    string tar_name(filename.substr(0, filename.length() - 9));
    decompress_tar(file, tar_name.c_str());
    unpack_file(tar_name.c_str(), path);
    remove(tar_name.c_str());
    return 0;
}

// 传入目录的绝对路径，第二个参数是为了递归使用，初始传入""即可
int get_dir_files(const char *path, const char *relative) {
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
        
        int fd = open("log_files.txt", O_CREAT|O_RDWR|O_APPEND);
        write(fd, sub_path, strlen(sub_path));
        write(fd, "\n", 1);

        printf("%s\n", sub_path);

		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 
            && strcmp(entry->d_name, "..") != 0) {
            // 递归
			get_dir_files(path, sub_path);
		}
		free(sub_path);
	}
	closedir(p_dir);
	return -1;
}