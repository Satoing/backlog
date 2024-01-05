#include <json/json.h>
#include <string>
#include <iostream>
#include <config.h>
#include <fcntl.h>

void readStrJson() {
	int fd = open((config.path+"/config.json").c_str(), O_RDONLY);
    char buf[1024];
    read(fd, buf, 1023);
	Json::Reader reader;
	Json::Value root;
 
	//从字符串中读取数据  
	if (reader.parse(buf, root)) {
		std::string passwd = root["passwd"].asString();
		std::string topath = root["topath"].asString();
		std::cout << passwd + "," << topath <<  std::endl;
	}
}

int main() {
    readStrJson();
    return 0;
}