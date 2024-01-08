/*
包含文件遍历和打包、解包三部分（解耦合）
遍历前查看文件的可访问性，如果不可访问就不遍历
用户在客户端选择需要备份的文件，返回给服务端需要打包的文件列表
打包使用接收到的文件列表，判断文件类型后打包
解包就是逆操作，主要是要判断文件类型然后进行不同的操作
1. 普通文件复制即可
2. 目录文件不用操作，因为存储了相对路径
3. 软连接文件保持链接关系
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <vector>

#define min(a,b) (((a) < (b)) ? a : b)

struct posix_tar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6]; //
	char version[2]; //
	char uname[32]; //
	char gname[32]; //
	char devmajor[8]; //
	char devminor[8]; //
	char prefix[155];
};

class Pack {
private:
    std::string file; // 当前打包文件,便于出错提醒
    std::vector<std::string> files; // 传入的文件列表(绝对路径)
    bool permission; // 路径安全性
public:
    Pack(std::vector<std::string> &need_files, std::string from, std::string to);
    ~Pack();
    int packOneFile();
    int packFiles(); //打包操作
};

class Unpack {
private:
    std::string file; // 要解包的文件
public:
    Unpack(std::string &unpack_file);
    ~Unpack();
    int unpackFile();
};

int main(int argc,char *argv[]) {
	printf("[解压文件: %s\n", argv[1]);
	int fd = open(argv[1], O_RDWR);
	
	char buf[1024*4];
	int chunk = sizeof(buf);

	while(1) {
		read(fd,buf,512);//读取tar header
		if (buf[0] == 0)
			break;
		
		struct posix_tar_header *phdr = (struct posix_tar_header*)buf;
		
		//从tar header头得到size
		char *p = phdr->size;
		int f_len = 0;
		while(*p)//8进制->10进制
			f_len = (f_len * 8) + (*p++ - '0');

		int bytes_left = f_len;//此文件大小
		int fdout = open(phdr->name,O_CREAT | O_RDWR);
		if (fdout == -1)
		{
			printf("	failed to extract file: %s\n",phdr->name);
			printf("	fdout open failed\n");
			return 0;
		}		
		printf("	%s (%d bytes)\n",phdr->name,f_len);
		while(bytes_left)
		{
			int iobytes = min(chunk,bytes_left);
			//read(fd,buf,iobytes);
			read(fd,buf,((iobytes - 1) / 512 + 1) * 512);
			write(fdout,buf,iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}
	close(fd);
	return 0;
}