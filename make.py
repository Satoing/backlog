import os

# os.system("g++ -o main src/main.cpp src/backup.cpp src/compress.cpp\
#            src/mytar.cpp src/pack.cpp src/decompress.cpp -Iinclude -Llib")

os.system("g++ -o backup.so src/backup.cpp src/compress.cpp src/mytar.cpp src/pack.cpp \
            src/decompress.cpp -Iinclude -Llib -shared -fPIC")
# os.system("./main")