import ctypes
from flask import Flask, jsonify, request
import os

app = Flask(__name__)

cpp = ctypes.CDLL("./backup.so")

# 获取某个路径下的文件，将文件列表以json格式返回前端
@app.route("/getfiles", methods=['GET'])
def get_files():
    # 读取url中的查询参数path
    path = request.args.get("path")
    if path == "":  # 如果是空路径直接返回-1
        return "-1", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}
    
    # C实现的遍历文件夹的函数，将文件名写入log_files.txt中
    cpp.get_dir_files(path.encode(), "".encode())
    f = open("log_files.txt")
    
    files = []
    # 从txt中逐行读取文件记录
    line = f.readline()
    while line:   
        files.append(line[:-1])
        line = f.readline() 
    f.close()
    os.remove("log_files.txt")

    # 向网页返回json
    return jsonify(files), 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}

# 对用户选择的文件进行备份，还是将文件名记录在log_files.txt中
@app.route("/backupfiles", methods=['POST'])
def user_register():
    # 前端因为跨域问题，返回字符串而不是json
    filestring = request.get_data()
    print(filestring)
    files = filestring.split(",")
    print(files)
    return "0", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}

# cpp.backup_all_files(path.encode(), name.encode())
# cpp.get_dir_files(path.encode(), "".encode())

if __name__ == '__main__':
    app.run(port=5000)