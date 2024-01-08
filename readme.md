# 打包
默认打包到`/usr/shared`，如不存在需要创建，并设置权限777

```
./main -b <打包目录的路径> <打包后的文件名>

./main -b /root/program/backlog/include test
root@ubuntu1:/usr/shared# ls
test.tar.compress
```

# 解包
```
./main -r <要解包的文件名> <解包后文件存放的路径>

./main -r sb.tar.compress /root/newfolder
```