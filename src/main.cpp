#include <common.h>
#include <backup.h>

int main(int argc, char *argv[]) {
    if(strcmp(argv[1], "-r") == 0) {
        restore(argv[2], argv[3]);
        printf("unpack success\n\n");
    }
    else if(strcmp(argv[1], "-b") == 0) {
        backup_all_files(argv[2], argv[3]);
        printf("backup success\n\n");
    }
}