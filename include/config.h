#include <string>
#include <unistd.h>
#include <iostream>

class Config {
public:
    std::string path;
    Config() {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("getcwd error");
        this->path = cwd;
    }
} config;