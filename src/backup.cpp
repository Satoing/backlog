#include <backup.h>

Backup::Backup(std::string from, std::string to,
            const std::vector<std::string> &relative_paths) {
    this->src_path = from;
    this->back_path = to;
    for(auto path : relative_paths) {
        this->abs_paths.emplace_back(from + path);
    }
}

std::string Backup::getBackupPath() {
    return this->back_path;
}

int doBackup() {
    
}