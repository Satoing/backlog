// #include <json/json.h>
// #include <string>
// #include <iostream>
// #include <config.h>
// #include <fcntl.h>
#include <common.h>

// void readStrJson() {
// 	int fd = open((config.path+"/config.json").c_str(), O_RDONLY);
//     char buf[1024];
//     read(fd, buf, 1023);
// 	Json::Reader reader;
// 	Json::Value root;
 
// 	//从字符串中读取数据  
// 	if (reader.parse(buf, root)) {
// 		std::string passwd = root["passwd"].asString();
// 		std::string topath = root["topath"].asString();
// 		std::cout << passwd + "," << topath <<  std::endl;
// 	}
// }

int main(int argc, char *argv[]) {
    mtar_t tar;
	mtar_header_t h;

	/* 打开存档以进行读取操作 */
	mtar_open(&tar, argv[1], "r");

	/* 打印所有文件名和文件大小 */
	while ( (mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD ) {
		printf("%s (%d bytes)\n", h.name, h.size);
		mtar_next(&tar);
	}

	/* 关闭 */
	mtar_close(&tar);
}