import os

os.system("g++ -o main src/main.cpp -I include -Llib -ljsoncpp")
os.system("./main")