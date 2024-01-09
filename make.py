import os

os.system("g++ -o main src/main.cpp src/backup.cpp src/compress.cpp\
           src/mytar.cpp src/pack.cpp src/decompress.cpp -Iinclude -Llib")

os.system("mv main bin/main")

os.system("g++ -o backup.so src/backup.cpp src/compress.cpp src/mytar.cpp src/pack.cpp \
            src/decompress.cpp -Iinclude -Llib -shared -fPIC")

os.system("mv backup.so server/backup.so")

os.system("mkdir /usr/shared")
