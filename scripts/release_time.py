import sys
import os
import json
import zipfile
import argparse
import time
from datetime import datetime

# 切换到项目根目录
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def get_board_type():
    with open("build/compile_commands.json") as f:
        data = json.load(f)
        for item in data:
            if not item["file"].endswith("main.cc"):
                continue
            command = item["command"]
            # extract -DBOARD_TYPE=xxx
            board_type = command.split("-DBOARD_TYPE=\\\"")[1].split("\\\"")[0].strip()
            return board_type
    return None

def get_project_version():
    with open("CMakeLists.txt") as f:
        for line in f:
            if line.startswith("set(PROJECT_VER"):
                return line.split("\"")[1].split("\"")[0].strip()
    return None

def get_compile_time(format="%Y%m%d_%H%M%S"):
    """
    获取当前时间作为编译时间
    :param format: 时间格式，默认: 年月日_时分秒
    :return: 格式化后的时间字符串
    """
    return datetime.now().strftime(format)

def merge_bin():
    if os.system("idf.py merge-bin") != 0:
        print("merge bin failed")
        sys.exit(1)

def zip_bin(board_type, project_version, compile_time=None):
    if not os.path.exists("releases"):
        os.makedirs("releases")
    
    # 如果没有提供编译时间，则使用当前时间
    if compile_time is None:
        compile_time = get_compile_time()
    
    # 创建文件名，格式：v{版本}_{板子类型}_{编译时间}.zip
    output_path = f"releases/v{project_version}_{board_type}_{compile_time}.zip"
    
    if os.path.exists(output_path):
        os.remove(output_path)
    
    with zipfile.ZipFile(output_path, 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
        zipf.write("build/merged-binary.bin", arcname="merged-binary.bin")
    
    print(f"zip bin to {output_path} done")
    return output_path

def release_current():
    merge_bin()
    board_type = get_board_type()
    print("board type:", board_type)
    project_version = get_project_version()
    print("project version:", project_version)
    compile_time = get_compile_time()
    print("compile time:", compile_time)
    zip_bin(board_type, project_version, compile_time)

def get_all_board_types():
    board_configs = {}
    with open("main/CMakeLists.txt", encoding='utf-8') as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            # 查找 if(CONFIG_BOARD_TYPE_*) 行
            if "if(CONFIG_BOARD_TYPE_" in line:
                config_name = line.strip().split("if(")[1].split(")")[0]
                # 查找下一行的 set(BOARD_TYPE "xxx") 
                next_line = lines[i + 1].strip()
                if next_line.startswith("set(BOARD_TYPE"):
                    board_type = next_line.split('"')[1]
                    board_configs[config_name] = board_type
    return board_configs

def release(board_type, board_config, config_filename="config.json"):
    config_path = f"main/boards/{board_type}/{config_filename}"
    if not os.path.exists(config_path):
        print(f"跳过 {board_type} 因为 {config_filename} 不存在")
        return

    # 获取项目版本
    project_version = get_project_version()
    print(f"Project Version: {project_version}", config_path)

    # 获取编译时间（所有构建使用相同的时间）
    compile_time = get_compile_time()
    print(f"Compile Time: {compile_time}")

    with open(config_path, "r") as f:
        config = json.load(f)
    target = config["target"]
    builds = config["builds"]
    
    for build in builds:
        name = build["name"]
        if not name.startswith(board_type):
            raise ValueError(f"name {name} 必须以 {board_type} 开头")
        
        # 生成包含编译时间的输出路径
        output_path = f"releases/v{project_version}_{name}_{compile_time}.zip"
        if os.path.exists(output_path):
            print(f"跳过 {board_type} 因为 {output_path} 已存在")
            continue

        sdkconfig_append = [f"{board_config}=y"]
        for append in build.get("sdkconfig_append", []):
            sdkconfig_append.append(append)
        print(f"name: {name}")
        print(f"target: {target}")
        for append in sdkconfig_append:
            print(f"sdkconfig_append: {append}")
        # unset IDF_TARGET
        os.environ.pop("IDF_TARGET", None)
        # Call set-target
        if os.system(f"idf.py set-target {target}") != 0:
            print("set-target failed")
            sys.exit(1)
        # Append sdkconfig
        with open("sdkconfig", "a") as f:
            f.write("\n")
            for append in sdkconfig_append:
                f.write(f"{append}\n")
        # Build with macro BOARD_NAME defined to name
        if os.system(f"idf.py -DBOARD_NAME={name} build") != 0:
            print("build failed")
            sys.exit(1)
        # Call merge-bin
        if os.system("idf.py merge-bin") != 0:
            print("merge-bin failed")
            sys.exit(1)
        # Zip bin
        zip_bin(name, project_version, compile_time)
        print("-" * 80)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("board", nargs="?", default=None, help="板子类型或 all")
    parser.add_argument("-c", "--config", default="config.json", help="指定 config 文件名，默认 config.json")
    parser.add_argument("-t", "--time-format", default="%Y%m%d_%H%M%S", 
                       help="时间格式，默认 %%Y%%m%%d_%%H%%M%%S (年月日_时分秒)")
    parser.add_argument("--list-boards", action="store_true", help="列出所有支持的 board 列表")
    parser.add_argument("--json", action="store_true", help="配合 --list-boards，JSON 格式输出")
    args = parser.parse_args()

    # 更新全局时间格式
    if args.time_format:
        def get_compile_time_custom():
            return datetime.now().strftime(args.time_format)
        # 临时替换get_compile_time函数
        original_get_compile_time = get_compile_time
        get_compile_time = get_compile_time_custom

    if args.list_boards:
        board_configs = get_all_board_types()
        boards = list(board_configs.values())
        if args.json:
            print(json.dumps(boards))
        else:
            for board in boards:
                print(board)
        sys.exit(0)

    if args.board:
        board_configs = get_all_board_types()
        found = False
        for board_config, board_type in board_configs.items():
            if args.board == 'all' or board_type == args.board:
                release(board_type, board_config, config_filename=args.config)
                found = True
        if not found:
            print(f"未找到板子类型: {args.board}")
            print("可用的板子类型:")
            for board_type in board_configs.values():
                print(f"  {board_type}")
    else:
        release_current()