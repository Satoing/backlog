#include <common.h>

// 传入目录的绝对路径，文件列表引用
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

int pack_path(const char *path) {
    // 遍历目录，打包
    vector<string> files;
    list_dir(path, files, "");
    string path_s = path;
    path_s += "/";

    for(string file : files) {
        printf("%s\n", (path_s+file).c_str());
    }
    return 0;
}

int main(int argc, char *argv[]) {
    pack_path(argv[1]);
}