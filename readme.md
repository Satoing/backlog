# 项目编译

```
python3.8 make.py
```

# 命令行使用

可执行文件main存放在bin目录里面，运行命令如下：

## 打包
默认打包到`/usr/shared`，如不存在需要创建。

```
mkdir /usr/shared
./main -b <打包目录的路径> <打包后的文件名>

./main -b /root/program/backlog/include test
root@ubuntu1:/usr/shared# ls
test.tar.compress
```

## 解包

```
./main -r <要解包的文件名> <解包后文件存放的路径>

./main -r test.tar.compress /root/newfolder
```

# 网页使用

编译好backup.so后，启动server，且需要安装flask：

```
pip3 install flask
cd server
python3 server.py
```

在网页的操作请助教查看视频附件。