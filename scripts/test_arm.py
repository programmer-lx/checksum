import subprocess
import sys
import argparse
import os
from pathlib import Path
import shutil
import platform

CURRENT_OS = str.lower(platform.system())

def run_command(command, env=None):
    print(f"\n[RUNNING] {' '.join(command)}")
    # 合并当前系统环境变量
    current_env = os.environ.copy()
    if env:
        current_env.update(env)
    subprocess.run(command, check=True, env=current_env)

def main():
    parser = argparse.ArgumentParser(description="arm NEON & SVE-128 Testing (Ubuntu) with QEMU")
    parser.add_argument("--test_mode", choices=["min", "max"], default="min")
    parser.add_argument("--compiler", choices=["gcc", "clang"], default="gcc")
    args = parser.parse_args()

    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    build_base = project_root / "build" / f"{CURRENT_OS}_arm"

    qemu_bin = shutil.which("qemu-aarch64-static") or shutil.which("qemu-aarch64")
    print(f"qemu bin path: {qemu_bin}")

    # 使用 aarch64 交叉工具链
    # (name, c_compiler, cxx_compiler, sub_dir)
    configs = [
        ("Clang-17", "clang-17", "clang++-17", "clang17"),
        ("GCC", "aarch64-linux-gnu-gcc-13", "aarch64-linux-gnu-g++-13", "gcc13"),
    ]
    if args.test_mode == "min":
        if args.compiler == "gcc":
            configs = [configs[1]]
        elif args.compiler == "clang":
            configs = [configs[0]]
        else:
            print("Error: invalid compiler.")
            sys.exit(1)

    build_options = ["Release"]
    if args.test_mode == "max":
        build_options += ["Debug"]

    # 是否编译成DLL
    dll_options = ["OFF"]

    for name, c_compiler, cxx_compiler, subdir in configs:
        for build_cfg in build_options:
            for dll_opt in dll_options:
                current_build_dir = build_base / f"{subdir}_{build_cfg}_dll_{dll_opt}"
                
                print(f"\n{'='*60}\nTarget: {name} | Config: {build_cfg} | DLL: {dll_opt}\n{'='*60}")

                # 1. 配置
                # name -> c_flags
                c_flags = {
                    "Clang-17": "--target=aarch64-linux-gnu -march=armv8-a",
                    "GCC": "-march=armv8-a"
                }

                cxx_flags = {
                    "Clang-17": "--target=aarch64-linux-gnu -march=armv8-a",
                    "GCC": "-march=armv8-a"
                }

                config_args = [
                    "cmake", "-S", str(project_root), "-B", str(current_build_dir),
                    "-G", "Ninja Multi-Config",
                    f"-DCMAKE_C_COMPILER={c_compiler}",
                    f"-DCMAKE_CXX_COMPILER={cxx_compiler}",
                    f"-DCMAKE_C_FLAGS={c_flags[name]}",
                    f"-DCMAKE_CXX_FLAGS={cxx_flags[name]}",
                    "-DCKS_BUILD_TESTS=ON",
                    f"-DCKS_BUILD_DLL={dll_opt}",
                    # 静态链接使得 QEMU 运行不需要额外的库搜索路径
                    "-DCMAKE_EXE_LINKER_FLAGS=-static"
                ]
                if os.environ.get("GITHUB_ACTIONS") != "true":
                    config_args.append(f"-DCMAKE_CROSSCOMPILING_EMULATOR={qemu_bin}")

                run_command(config_args)

                # 2. 编译
                run_command(["cmake", "--build", str(current_build_dir), "--config", build_cfg])

                # 3. 测试
                test_env = {"QEMU_CPU": f"max,sve-max-vq=1"}
                run_command([
                    "ctest", "--output-on-failure", "--test-dir", str(current_build_dir), "-C", build_cfg
                ], env=test_env)

    print("\n[SUCCESS] All ARM tests passed.")

if __name__ == "__main__":
    main()