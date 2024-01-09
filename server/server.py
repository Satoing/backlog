import ctypes
from flask import Flask, jsonify, request
import os

app = Flask(__name__)
cpp = ctypes.CDLL("./backup.so")

path = ""

# 获取某个路径下的文件，将文件列表以json格式返回前端
@app.route("/getfiles", methods=['GET'])
def get_files():
    global path
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
def backup_files():
    filename = request.args.get("filename")
    global path
    print("path=",path)
    # 前端因为跨域问题，返回字符串而不是json
    filestring = request.get_data().decode()[:-1]
    print(filestring)
    files = filestring.split(",")
    print(files)
    if files[0] == "":
        return "-1", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}
    with open('log_files.txt', 'w') as f:
        for file in files: f.write(file+'\n')
    cpp.backup_from_log(filename.encode(), path.encode())
    return "0", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}

# 还原文件到指定路径
@app.route("/restorefile", methods=['GET'])
def restore_file():
    path = request.args.get("path")
    filename = request.args.get("filename")
    print(filename)
    cpp.restore(filename.encode(), path.encode())
    return "0", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}

@app.route("/delete", methods=['POST'])
def delete_files():
    filestring = request.get_data().decode()[:-1]
    print(filestring)
    files = filestring.split(",")
    print(files)
    for file in files:
        os.remove("/usr/shared/"+file)
    return "0", 200, {"Content-Type":"application/json", 'Access-Control-Allow-Origin': "*"}


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)