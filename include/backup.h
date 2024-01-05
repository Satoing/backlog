#include <string>
#include <vector>

class Backup {
private:
    std::string src_path;
    std::string back_path;
    std::vector<std::string> abs_paths;
public:
    Backup(std::string from, std::string to, const std::vector<std::string> &relative_paths);
    ~Backup();
    std::string getBackupPath();
    int doBackup();
};

class Restore {
private:
    std::string back_file;
    std::string restore_path;
public:
    Restore(std::string file, std::string to);
    ~Restore();
    std::string getRestoreupPath();
    void doRestore();
};