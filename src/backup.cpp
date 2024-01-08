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